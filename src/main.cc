#include "audio-processor.h"
#include "audio-stream.h"
#include "processed-audio.h"
#include "raylib.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <string>
#include <vector>

static constexpr size_t sample_rate = 48000;
static constexpr size_t buffer_size = 1 << 10;
static constexpr size_t frequency_count = buffer_size / 2 + 1;
static constexpr size_t drawable_width = 500;

void RenderBars(
    const audio::ProcessedAudioBuffer<frequency_count> &audio_buffer) {
  float start_x = (GetRenderWidth() - drawable_width) / 2;
  size_t bar_count = audio_buffer.squashed_samples.size();
  float bar_width = drawable_width / (2 * bar_count - 1);
  static std::vector<float> rendered_bar_heights(bar_count);

  for (int i = 0; i < bar_count; i++) {
    float bar_height = std::max(
        audio_buffer.squashed_samples[i].normalized_amplitude * 180, 10.0f);
    rendered_bar_heights[i] +=
        (bar_height - rendered_bar_heights[i]) * GetFrameTime() * 40;

    bar_height = rendered_bar_heights[i];

    DrawRectangleRec(
        {.x = start_x + 2 * i * bar_width,
         .y = static_cast<float>((GetRenderHeight() - bar_height) / 2) - 100.0f,
         .width = static_cast<float>(bar_width),
         .height = static_cast<float>(bar_height)},
        // 5, 5,
        RED);
  }
}

void RenderCircle(
    const audio::ProcessedAudioBuffer<frequency_count> &audio_buffer) {
  Vector2 center = {.x = GetRenderWidth() / 2, .y = GetRenderHeight() / 2};
  float radius = (drawable_width * 1.5 / 2.0f);
  // DrawCircleLinesV(center, radius, WHITE);
  DrawRing(center, radius,
           radius + std::max(audio_buffer.avg_amplitude * 15, 1.0f), 0, 360,
           100,
           WHITE); // Draw ring outline
}

template <size_t K, size_t frequency_count>
std::array<audio::ProcessedAudioSample, K>
maxKElements(const audio::ProcessedAudioBuffer<frequency_count> &audio_buffer) {
  // if (K <= 0 || arr.size() < K) {
  //   return {};
  // }

  // Initialize an array of size K with the smallest possible integer values
  std::array<audio::ProcessedAudioSample, K> topKElements{0};

  for (auto sample : audio_buffer.samples) {
    // Find the position where the current element should be inserted
    auto pos = std::find_if(topKElements.begin(), topKElements.end(),
                            [sample](audio::ProcessedAudioSample x) {
                              // if (sample.frequency < 1000 || sample.frequency
                              // > 15000)
                              //   return false;
                              return sample.normalized_amplitude >
                                     x.normalized_amplitude;
                            });

    if (pos != topKElements.end()) {
      // Shift elements to the left to make space for the new element
      std::copy_backward(pos, topKElements.end() - 1, topKElements.end());
      *pos = sample;
    }
  }

  return topKElements;
}

void RenderWave(
    const audio::ProcessedAudioBuffer<buffer_size / 4 + 1> &audio_buffer) {
  float start_x = (GetRenderWidth() - drawable_width) / 2;
  static float time = 0.0f;
  constexpr size_t max_elements = 3;
  auto max_components =
      maxKElements<max_elements, buffer_size / 4 + 1>(audio_buffer);
  std::array<Vector2, drawable_width> points{0};
  static std::array<Vector2, drawable_width> rendered_points{
      GetRenderHeight() / 2 + 100.0f};

  const float damp_region = 200.0f;
  time += GetFrameTime();
  for (int i = 0; i < drawable_width; i++) {
    for (auto &sample : max_components) {
      const float angularFrequency = -2.0f * PI * sample.frequency * 3;
      float y = 30 * sample.normalized_amplitude *
                sinf(i * angularFrequency / sample_rate + time * 10);
      if (i < drawable_width / 2) {
        points[i].y += y * std::min(i / damp_region, 1.0f);
      } else {
        points[i].y += y * std::min((drawable_width - i) / damp_region, 1.0f);
      }
      points[i].x = start_x + i;
    }
  }

  Vector2 last_point = {.x = start_x, .y = GetRenderHeight() / 2 + 100.0f};
  for (int i = 0; i < points.size(); i++) {
    rendered_points[i].x = start_x + i;
    // rendered_points[i].y -= GetRenderHeight() / 2 + 100.0f;

    rendered_points[i].y +=
        (points[i].y - rendered_points[i].y) * GetFrameTime() * 10;
    Vector2 current_point = {.x = rendered_points[i].x,
                             .y = GetRenderHeight() / 2 + 100.0f +
                                  rendered_points[i].y};
    DrawLineEx(last_point, current_point, 5, YELLOW);
    last_point = current_point;
  }
}

int main() {
  audio::AudioProcessor<sample_rate, buffer_size> processor1("Test");
  audio::AudioProcessor<sample_rate / 2, buffer_size / 2> processor2("Test");
  Visualizer::AudioStream audio_stream(
      "Google Chrome", sample_rate, buffer_size,
      [&](std::vector<float> samples, size_t sample_rate) {
        std::array<float, buffer_size> new_samples;
        std::move(samples.begin(), samples.end(), new_samples.begin());
        processor1.OnNewSample(std::move(new_samples));
      });

  Visualizer::AudioStream audio_stream2(
      "rnnoise_source", sample_rate / 2, buffer_size / 2,
      [&](std::vector<float> samples, size_t sample_rate) {
        std::array<float, buffer_size / 2> new_samples;
        std::move(samples.begin(), samples.end(), new_samples.begin());
        processor2.OnNewSample(std::move(new_samples));
      });

  static float radius = 100;
  InitWindow(1000, 1000, "Example");
  audio_stream.Start();
  audio_stream2.Start();
  while (!WindowShouldClose()) {
    const auto &buffer = processor1.Buffer();
    const auto &buffer2 = processor2.Buffer();
    BeginDrawing();
    ClearBackground(BLACK);
    RenderBars(buffer);
    RenderCircle(buffer);
    RenderWave(buffer2);
    EndDrawing();
  }
  audio_stream.Stop();
  audio_stream2.Stop();
  CloseWindow();
}
