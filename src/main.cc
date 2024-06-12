#include "audio-processor.h"
#include "audio-stream.h"
#include "processed-audio.h"
#include "raylib.h"
#include "rlImGui.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <imgui.h>
#include <string>
#include <vector>

static constexpr size_t sample_rate = 48000;
static constexpr size_t buffer_size = 1 << 10;
static constexpr size_t frequency_count = buffer_size / 2 + 1;
static constexpr size_t drawable_width = 400;

struct BarOptions {
  bool disabled = false;
  float inbetween_gap = 0.0f;
  float height_mult = 180.0f;
  float min_height = 10.0f;
  float anim_smoothness = 40.0f;
  float hue_start = 180.0f;
  float hue_dynamic_range = 180.f;
  float pos_y_offset = 100.0f;
  bool enable_changing_hue = true;
  float changing_hue_speed = 10.0f;
};

struct CircleOptions {
  bool disabled = false;
  float radius_multiplier = 1.5f;
  float max_outer_radius = 25.0f;
  float anim_smoothness = 20.0f;
  float hue_start = 300.0f;
  float hue_dynamic_range = 1000.f;
  bool enable_changing_hue = true;
  float changing_hue_speed = 10.0f;
  int segments = 100;
  float start_angle_deg = 0.0f;
  float end_angle_deg = 360.0f;
};

struct WaveOptions {
  bool disabled = false;
  float anim_smoothness = 20.0f;
  float hue = 60.0f;
  float pos_y_offset = 100.0f;
  int wave_count = 3;
  float damp_strength = 200.0f;
  float x_movement_speed = 10.f;
  float amplitude_mult = 30.0f;
  float frequency_mult = 2.0f;
  float thickness = 5.0f;
};

void RenderBars(const audio::ProcessedAudioBuffer &audio_buffer,
                BarOptions bar_options) {
  if (bar_options.disabled)
    return;
  static float time;
  time += GetFrameTime();
  float start_x = (GetRenderWidth() - drawable_width) / 2.0f;
  size_t bar_count = audio_buffer.squashed_samples.size();
  float bar_width =
      drawable_width / (2.0f * bar_count - 1) + bar_options.inbetween_gap;
  static std::vector<float> rendered_bar_heights(bar_count);

  for (int i = 0; i < bar_count; i++) {
    float bar_height = bar_options.min_height;
    if (audio_buffer.max_amplitude > 5.0f)
      bar_height =
          std::max(audio_buffer.squashed_samples[i].normalized_amplitude *
                       bar_options.height_mult,
                   bar_height);

    rendered_bar_heights[i] += (bar_height - rendered_bar_heights[i]) *
                               GetFrameTime() * bar_options.anim_smoothness;

    bar_height = rendered_bar_heights[i];

    float hue = bar_options.hue_start +
                float(i) / bar_count * bar_options.hue_dynamic_range;

    if (bar_options.enable_changing_hue)
      hue += time * bar_options.changing_hue_speed;
    float saturation = 1.0f;
    float value = 1.0f;

    Color color = ColorFromHSV(hue, saturation, value);

    DrawRectangleRec(
        {.x = start_x + 2 * i * bar_width,
         .y = static_cast<float>((GetRenderHeight() - bar_height) / 2) -
              bar_options.pos_y_offset,
         .width = static_cast<float>(bar_width),
         .height = static_cast<float>(bar_height)},
        color);
  }
}

void RenderCircle(const audio::ProcessedAudioBuffer &audio_buffer,
                  CircleOptions circle_options) {
  if (circle_options.disabled)
    return;
  static float time;
  time += GetFrameTime();
  Vector2 center = {.x = GetRenderWidth() / 2.0f,
                    .y = GetRenderHeight() / 2.0f};
  float radius = (drawable_width * circle_options.radius_multiplier / 2.0f);
  float outer_radius = radius + std::max(audio_buffer.avg_amplitude *
                                             circle_options.max_outer_radius,
                                         1.0f);
  static float rendered_radius = 0.0f;
  rendered_radius += (outer_radius - rendered_radius) * GetFrameTime() *
                     circle_options.anim_smoothness;
  float hue = circle_options.hue_start +
              circle_options.hue_dynamic_range *
                  (audio_buffer.avg_amplitude / audio_buffer.max_amplitude);

  if (circle_options.enable_changing_hue)
    hue += time * circle_options.changing_hue_speed;

  float saturation = 1.0f;
  float value = 1.0f;
  Color color = ColorFromHSV(hue, saturation, value);
  DrawRing(center, radius, rendered_radius, circle_options.start_angle_deg,
           circle_options.end_angle_deg, circle_options.segments, color);
}

std::vector<audio::ProcessedAudioSample>
maxKElements(const audio::ProcessedAudioBuffer &audio_buffer, int K) {
  std::vector<audio::ProcessedAudioSample> topKElements(K);

  for (auto sample : audio_buffer.samples) {
    auto pos = std::find_if(topKElements.begin(), topKElements.end(),
                            [sample](audio::ProcessedAudioSample x) {
                              return sample.normalized_amplitude >
                                     x.normalized_amplitude;
                            });

    if (pos != topKElements.end()) {
      std::copy_backward(pos, topKElements.end() - 1, topKElements.end());
      *pos = sample;
    }
  }

  return topKElements;
}

void RenderWave(const audio::ProcessedAudioBuffer &audio_buffer,
                WaveOptions wave_options) {
  if (wave_options.disabled)
    return;
  float start_x = (GetRenderWidth() - drawable_width) / 2.0f;
  static float time = 0.0f;
  int max_elements = wave_options.wave_count;
  auto max_components = maxKElements(audio_buffer, max_elements);

  std::array<Vector2, drawable_width> points{0};

  static std::array<Vector2, drawable_width> rendered_points{
      GetRenderHeight() / 2.0f + wave_options.pos_y_offset};

  const float damp_region = wave_options.damp_strength;
  time += GetFrameTime();

  for (int i = 0; i < drawable_width; i++) {
    for (auto &sample : max_components) {
      const float angularFrequency =
          -2.0f * PI * sample.frequency * wave_options.frequency_mult;
      float y = wave_options.amplitude_mult * sample.normalized_amplitude *
                sinf(i * angularFrequency / sample_rate +
                     time * wave_options.x_movement_speed);

      if (i < drawable_width / 2) {
        points[i].y += y * std::min(i / damp_region, 1.0f);
      } else {
        points[i].y += y * std::min((drawable_width - i) / damp_region, 1.0f);
      }

      points[i].x = start_x + i;
    }
  }

  Vector2 last_point = {
      .x = start_x, .y = GetRenderHeight() / 2.0f + wave_options.pos_y_offset};

  for (int i = 0; i < points.size(); i++) {
    rendered_points[i].x = start_x + i;

    rendered_points[i].y += (points[i].y - rendered_points[i].y) *
                            GetFrameTime() * wave_options.anim_smoothness;
    Vector2 current_point = {.x = rendered_points[i].x,
                             .y = GetRenderHeight() / 2.0f +
                                  wave_options.pos_y_offset +
                                  rendered_points[i].y};

    DrawLineEx(last_point, current_point, wave_options.thickness,
               ColorFromHSV(wave_options.hue, 1.0f, 1.0f));
    last_point = current_point;
  }
}

void RenderBarOptionConfigurator(BarOptions &bar_options) {
  ImGui::Begin("Bar options"); // Create a window called "Hello, world!" and
                               // append into it.

  ImGui::Checkbox("disabled", &bar_options.disabled);
  ImGui::SliderFloat("inbetween_gap", &bar_options.inbetween_gap, 0.0f, 10.0f);
  ImGui::SliderFloat("height_mult", &bar_options.height_mult, 50.0f, 400.0f);
  ImGui::SliderFloat("min_height", &bar_options.min_height, 0.0f, 100.f);
  ImGui::SliderFloat("anim_smoothness", &bar_options.anim_smoothness, 1.0f,
                     100.f);
  ImGui::SliderFloat("hue_start", &bar_options.hue_start, 0.0f, 360.f);
  ImGui::SliderFloat("hue_dynamic_range", &bar_options.hue_dynamic_range, 0.0f,
                     360.f);
  ImGui::SliderFloat("pos_y_offset", &bar_options.pos_y_offset, -400.0f,
                     400.0f); // Edit 1 float using a slider from 0.0f to 1.0f
  ImGui::Checkbox("enable_changing_hue", &bar_options.enable_changing_hue);
  if (bar_options.enable_changing_hue) {

    ImGui::SliderFloat("changing_hue_speed", &bar_options.changing_hue_speed,
                       50.0f,
                       200.0f); // Edit 1 float using a slider from 0.0f to 1.0f
  }

  ImGui::End();
}

void RenderCircleOptionConfigurator(CircleOptions &circle_options) {

  ImGui::Begin("Circle options"); // Create a window called "Hello, world!" and
  ImGui::Checkbox("disabled", &circle_options.disabled);
  ImGui::SliderFloat("radius_multiplier", &circle_options.radius_multiplier,
                     1.0f, 8.0f);
  ImGui::SliderFloat("max_outer_radius", &circle_options.max_outer_radius,
                     10.0f, 100.0f);
  ImGui::SliderFloat("anim_smoothness", &circle_options.anim_smoothness, 1.0f,
                     100.f);
  ImGui::SliderFloat("hue_start", &circle_options.hue_start, 0.0f, 360.f);
  ImGui::SliderFloat("hue_dynamic_range", &circle_options.hue_dynamic_range,
                     500.0f, 2000.f);
  ImGui::SliderInt("segments", &circle_options.segments, 4, 150);
  ImGui::SliderFloat("start_angle_deg", &circle_options.start_angle_deg, 0.0f,
                     270.0f);
  ImGui::SliderFloat("end_angle_deg", &circle_options.end_angle_deg, 90.0f,
                     540.0f);
  ImGui::Checkbox("enable_changing_hue", &circle_options.enable_changing_hue);
  if (circle_options.enable_changing_hue) {

    ImGui::SliderFloat("changing_hue_speed", &circle_options.changing_hue_speed,
                       50.0f,
                       200.0f); // Edit 1 float using a slider from 0.0f to 1.0f
  }

  ImGui::End();
}

void RenderWaveOptionConfigurator(WaveOptions &wave_options) {

  ImGui::Begin("Wave options"); // Create a window called "Hello, world!" and
  ImGui::Checkbox("disabled", &wave_options.disabled);

  ImGui::SliderFloat("pos_y_offset", &wave_options.pos_y_offset, -400.0f,
                     400.0f); // Edit 1 float using a slider from 0.0f to 1.0f
  ImGui::SliderFloat("damp_strength", &wave_options.damp_strength, 10.0f,
                     400.0f);
  ImGui::SliderFloat("hue", &wave_options.hue, 0.0f, 360.0f);
  ImGui::SliderFloat("x_movement_speed", &wave_options.x_movement_speed, 1.0f,
                     100.0f);
  ImGui::SliderFloat("anim_smoothness", &wave_options.anim_smoothness, 1.0f,
                     100.f);
  ImGui::SliderFloat("amplitude_mult", &wave_options.amplitude_mult, 1.0f,
                     100.f);
  ImGui::SliderInt("wave_count", &wave_options.wave_count, 1, 10);
  ImGui::SliderFloat("frequency_mult", &wave_options.frequency_mult, 1.0f,
                     10.0f);
  ImGui::SliderFloat("thickness", &wave_options.thickness, 1.0f, 20.0f);

  ImGui::End();
}

int main() {
  audio::AudioProcessor processor1("Test", sample_rate, buffer_size);
  audio::AudioProcessor processor2("Test", sample_rate, buffer_size);
  Visualizer::AudioStream audio_stream(
      "Google Chrome", sample_rate, buffer_size,
      [&](std::vector<float> samples, size_t sample_rate) {
        processor1.OnNewSample(std::move(samples));
      });

  Visualizer::AudioStream audio_stream2(
      "rnnoise_source", sample_rate / 2, buffer_size / 2,
      [&](std::vector<float> samples, size_t sample_rate) {
        processor2.OnNewSample(std::move(samples));
      });

  BarOptions bar_options;
  CircleOptions circle_options;
  WaveOptions wave_options;

  SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
  InitWindow(1000, 1000, "Example");
  audio_stream.Start();
  audio_stream2.Start();
  bool show_imgui = false;

  rlImGuiSetup(true);

  while (!WindowShouldClose()) {
    const auto &buffer = processor1.Buffer();
    const auto &buffer2 = processor2.Buffer();

    if (IsKeyPressed(KEY_C)) {
      show_imgui = !show_imgui;
    }
    BeginDrawing();
    ClearBackground(BLACK);

    if (show_imgui) {
      rlImGuiBegin();
      RenderBarOptionConfigurator(bar_options);
      RenderCircleOptionConfigurator(circle_options);
      RenderWaveOptionConfigurator(wave_options);
      rlImGuiEnd();
    }

    RenderBars(buffer, bar_options);
    RenderCircle(buffer, circle_options);
    RenderWave(buffer2, wave_options);
    EndDrawing();
  }
  audio_stream.Stop();
  audio_stream2.Stop();
  rlImGuiShutdown(); // cleans up ImGui
  CloseWindow();
}
