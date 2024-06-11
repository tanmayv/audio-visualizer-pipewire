#include "audio-processing.h"
#include "audio-processor.h"
#include "audio-stream.h"
#include "raylib.h"
#include <algorithm>
#include <atomic>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <mutex>
#include <string>
#include <vector>

// #define NUM_WAVES 10
// #define NUM_POINTS 40
// #define TWO_PI 6.28318530718f
//
// std::mutex samplesMtx;
// std::vector<float> samples;
// std::atomic<int> sample_rate;
//
// std::vector<float> frequencies;
// std::vector<float> renderedFrequencies;
// std::array<Vector2, 1920> frame_points;
// std::array<Vector2, 1920> rendered_points;
//
// // Running on Audio Capture Thread
// static void OnFrequencyBuffer(std::vector<float> s, int sr) {
//   sample_rate = sr;
//   std::lock_guard<std::mutex> lock(samplesMtx);
//   samples = std::move(s);
// }
//
// static void ProcessAudio() {
//   std::lock_guard<std::mutex> lock(samplesMtx);
//   if (samples.size() == 0)
//     return;
//   frequencies = audio::computeFFT64(samples, sample_rate);
//   frequencies = audio::FrequencyBands(frequencies, 8);
//   // frequencies = audio::FrequencyBands16(frequencies);
//   if (renderedFrequencies.size() < frequencies.size()) {
//     // renderedFrequencies.clear();
//     // for (auto f : frequencies) {
//     //   renderedFrequencies.push_back(f);
//     // }
//     renderedFrequencies = frequencies;
//   }
//   ClearBackground(WHITE);
// }
//
// static std::string GetFreqStr(int index, int count) {
//   return std::to_string(index * sample_rate / count);
// }
//
// static void RenderFrequencies() {
//   if (frequencies.size() == 0) {
//     return;
//   }
//   int canvasHeight = GetRenderHeight();
//   int canvasWidth = GetRenderWidth();
//   int freqCount = frequencies.size();
//   int freqCountToShow = std::min(freqCount, 20) - 1;
//   int lineHeight = ((canvasHeight - 20) / freqCountToShow);
//
//   int cursor = 0;
//   float prevAmplitude = frequencies.at(0);
//   float amplitude = frequencies.at(1);
//   for (int y = 1; y < freqCount - 1 && cursor < freqCountToShow; ++y) {
//     float nextAmplitude = frequencies.at(y + 1);
//     if (amplitude > 0.1 && amplitude >= prevAmplitude &&
//         amplitude > nextAmplitude) {
//       std::string freqValue = GetFreqStr(y, freqCount);
//       DrawText(freqValue.data(), 10, 10 + cursor * lineHeight, lineHeight -
//       2,
//                WHITE);
//       DrawText(std::to_string(amplitude).data(), 500, 10 + cursor *
//       lineHeight,
//                lineHeight - 2, BLUE);
//       cursor++;
//     }
//     prevAmplitude = amplitude;
//     amplitude = nextAmplitude;
//   }
//   std::string total = "Total frequencies: " + std::to_string(freqCount);
//   DrawText(total.data(), 10, 10 + cursor * lineHeight, lineHeight - 2, RED);
// }
//
// // Running on UI Thread
// static void Render() {
//
//   // For some reason this has to be before acquiring lock;
//   DrawFPS(10, 10);
//
//   static std::array<Vector2, NUM_POINTS> points;
//   // Draw a red rectangle
//   int size = frequencies.size();
//   int canvasWidth = GetRenderWidth();
//   int canvasHeight = GetRenderHeight();
//   if (size == 0)
//     return;
//   int cellWidth = canvasWidth / size;
//   std::string tot = "total freq: " + std::to_string(size);
//   DrawText(tot.data(), 100, 100, 24, WHITE);
//   assert(renderedFrequencies.size() == frequencies.size());
//   float dt = TWO_PI / NUM_POINTS;
//   Vector2 lastPoint;
//   for (int i = 0; i < NUM_POINTS; ++i) {
//     // int barHeight = std::min(20 * renderedFrequencies.at(i), 1080.0f);
//     // DrawRectangle(i * cellWidth, canvasHeight - barHeight - 20, cellWidth,
//     //               barHeight, RED);
//     float t = dt * i;
//     // Initialize sums for x and y
//     float sumX = 0.0f;
//     float sumY = 0.0f;
//     for (int j = 0; j < renderedFrequencies.size(); j++) {
//       // renderedFrequencies[i] +=
//       //     (frequencies[i] - renderedFrequencies[i]) * GetFrameTime() * 20;
//
//       // Sum over multiple sine waves and their Hilbert transforms
//       if (renderedFrequencies[j] < 1.5) {
//         continue;
//       }
//       float freq = j * sample_rate / renderedFrequencies.size();
//       float sineWave = 4 * renderedFrequencies[j] * sin(freq * t);
//       float hilbertTransform = 4 * renderedFrequencies[j] * cos(freq * t);
//
//       sumX += sineWave;
//       sumY += hilbertTransform;
//
//       // Create the points for the combined shape
//     }
//     Vector2 currentPoint{GetRenderWidth() / 2 + sumX,
//                          GetRenderHeight() / 2 + sumY};
//     // auto distance = std::sqrt(std::pow(lastPoint.x - currentPoint.x, 2) +
//     //                           std::pow(lastPoint.y - currentPoint.y, 2)) /
//     //                 34;
//     if (i != 0)
//       // DrawCircleV(currentPoint, 5, RED);
//       DrawLineV(currentPoint, lastPoint, RED);
//     lastPoint = currentPoint;
//     // points[i].x = GetRenderWidth() / 2 + sumX;
//     // points[i].y = GetRenderHeight() / 2 + sumY;
//   }
// }
//
// // Function to get max N elements and their original indices
// std::vector<std::pair<float, int>>
// getMaxNElementsAndIndices(const std::vector<float> &vec, int N) {
//   // Create a vector of pairs (value, index)
//   std::vector<std::pair<float, int>> valueIndexPairs;
//   for (int i = 0; i < vec.size(); ++i) {
//     valueIndexPairs.push_back({vec[i], i});
//   }
//
//   // Sort the pairs based on the value in descending order
//   std::sort(valueIndexPairs.begin(), valueIndexPairs.end(),
//             [](const auto &a, const auto &b) { return a.first > b.first; });
//
//   // Extract the top N elements and their indices
//   std::vector<std::pair<float, int>> result;
//   for (int i = 2; i < N && i < valueIndexPairs.size(); ++i) {
//     result.push_back(valueIndexPairs[i]);
//   }
//
//   return result;
// }
//
// // Function to draw a sine wave
// void DrawSineWave(float amplitude, float frequency, int sampleRate,
//                   bool reset) {
//   // Time variable
//   // static float time = 0.0f;
//   // const float timeIncrement = 1.0f / sampleRate;
//   const float angularFrequency = -2.0f * PI * frequency * 0.05;
//
//   // Calculate sine wave points
//   // std::vector<Vector2> points(GetRenderWidth());
//   for (int x = 0; x < GetRenderWidth(); x++) {
//     float y = std::max(0.5f * amplitude - 40.0f, 0.0f) *
//               sinf(x * angularFrequency / sampleRate);
//     // points[x] = {static_cast<float>(x), y};
//     if (reset) {
//       frame_points[x] = {static_cast<float>(x), y};
//     } else {
//       frame_points[x].x = x;
//       frame_points[x].y += y;
//     }
//   }
//
//   // Draw the sine wave as a continuous line
//   // DrawLineStrip(points.data(), points.size(), WHITE);
//
//   // Update the time variable
//   // time += timeIncrement;
// }
//
// float ShapeFn(int x, float dt) {
//   return 0.5;
//   static int wave_width = 120 * PI;
//   static int start_x = (GetRenderWidth() - wave_width) / 2;
//   if (x < start_x || x > start_x + wave_width) {
//     return 0;
//   }
//   return 0.5 * std::sinf(0.005 * (start_x + x));
// }
//
// void RenderLineWave() {
//   if (!renderedFrequencies.size())
//     return;
//   static float time = 0;
//   auto maxFrequencies =
//       getMaxNElementsAndIndices(renderedFrequencies, NUM_WAVES);
//
//   DrawFPS(10, 10);
//   DrawSineWave(0, 1, sample_rate, true);
//
//   for (auto freqPair : maxFrequencies) {
//     DrawSineWave(freqPair.first * 10,
//                  freqPair.second * sample_rate / renderedFrequencies.size(),
//                  sample_rate, false);
//     // DrawSineWave(100, 100, sample_rate, false);
//   }
//
//   std::array<Vector2, 1920> to_draw;
//   time += GetFrameTime();
//   for (int x = 0; x < GetRenderWidth(); x++) {
//     rendered_points[x].x = x;
//     rendered_points[x].y += ShapeFn(x, time) *
//                             (frame_points[x].y - rendered_points[x].y) *
//                             GetFrameTime() * 20;
//     to_draw[x] = {static_cast<float>(x),
//                   rendered_points[x].y + GetRenderHeight() / 2};
//   }
//   for (int i = 1; i < GetRenderWidth(); i++) {
//     DrawLineEx(to_draw[i - 1], to_draw[i], 5, WHITE);
//   }
//   // DrawLineStrip(to_draw.data(), GetRenderWidth(), RED);
//   // float waveWidth = PI * 400;
//   //
//   // int startX = (GetRenderWidth() - waveWidth) / 2;
//   // int endX = startX + waveWidth;
//   // float dt = waveWidth / NUM_POINTS;
//   // for (float i = 0; i < NUM_POINTS; i++) {
//   //   float x = i * dt;
//   //   int amplitude = 100;
//   //   float freq = 0.05f;
//   //
//   //   // float y = GetRenderHeight() / 2 + amplitude * std::sinf(freq * x);
//   //   // DrawPixel(startX + x, y, WHITE);
//   // }
//   //
//   // DrawLine(startX, GetRenderHeight() / 2, endX, GetRenderHeight() / 2,
//   // RED);
//   // DrawSineWave(100, 43, 22000);
// }
// // static void RenderLogLike() {
// //   int start = 0;
// //   int end = double logStart = std::log10(start);
// // }
//
//
//
// int main() {
//   const float scale = 1;
//   const int screenWidth = 1920 * scale;
//   const int screenHeight = 1080 * scale;
//   SetTargetFPS(60);
//   Visualizer::AudioStream audio_stream("Gogle Chrome", OnFrequencyBuffer);
//   InitWindow(screenWidth, screenHeight, "Example");
//   Shader shader = LoadShader("vertex.vs", "fragment.fs");
//   audio_stream.Start();
//   while (!WindowShouldClose()) {
//     ProcessAudio();
//     BeginDrawing();
//     ClearBackground(BLACK);
//     for (int i = 0; i < renderedFrequencies.size(); i++) {
//       renderedFrequencies[i] +=
//           (frequencies[i] - renderedFrequencies[i]) * GetFrameTime() * 5;
//     }
//
//     // BeginShaderMode(shader);
//     // RenderFrequencies();
//     // Render();
//     RenderLineWave();
//     // EndShaderMode();
//     EndDrawing();
//   }
//   audio_stream.Stop();
//   UnloadShader(shader);
//   CloseWindow();
// }
//
// float Wave1(float x, float t) {
//   return GetRenderHeight() / 2 + 100 * std::sinf(x * 0.05f + t);
// }
//
// float Wave1(float x, float t) {
//   return GetRenderHeight() / 2 + 200 * std::sinf(x * 0.01f + t + PI / 3);
// }

// void RendExp() {
//   static float time = 0.0f;
//   time += GetFrameTime();
//   int NUM_POINTS = 100;
//   for (int i = 0; i < NUM_POINTS; i++) {
//     float angle = angleStep * i;
//     // Using sum of sine waves to calculate x and y
//     float x = RADIUS * (cos(angle) + 0.5f * sin(2 * angle));
//     float y = RADIUS * (sin(angle) + 0.5f * cos(2 * angle));
//     points[i] = (Vector2){screenWidth / 2 + x, screenHeight / 2 + y};
//   }
// }

// int main() {
//   const int scale = 1;
//   const int screenWidth = 1920 * scale;
//   const int screenHeight = 1080 * scale;
//   SetTargetFPS(60);
//   // Visualizer::AudioStream audio_stream("Google Chrome",
//   OnFrequencyBuffer);
//   InitWindow(screenWidth, screenHeight, "Example");
//   // audio_stream.Start();
//   while (!WindowShouldClose()) {
//     ProcessAudio();
//     BeginDrawing();
//     ClearBackground(BLACK);
//     RendExp();
//     EndDrawing();
//   }
//   // audio_stream.Stop();
//   CloseWindow();
// }

// #include "raylib.h"
// #include <math.h>

// // Define the sine wave parameters
// #define NUM_WAVES 3
// #define NUM_POINTS 1000
// #define TWO_PI 6.28318530718f
//
// int main(void) {
//   // Initialization
//   const int screenWidth = 800;
//   const int screenHeight = 600;
//
//   InitWindow(screenWidth, screenHeight,
//              "Sum of Multiple Circles from Sine Waves");
//
//   Vector2 points[NUM_POINTS];
//   float dt = TWO_PI / NUM_POINTS;
//
//   // Frequencies and amplitudes for the waves
//   float frequencies[NUM_WAVES] = {1.0f, 2.0f, 3.0f};
//   float amplitudes[NUM_WAVES] = {50.0f, 30.0f, 20.0f};
//
//   while (!WindowShouldClose()) // Detect window close button or ESC key
//   {
//     // Update
//     for (int i = 0; i < NUM_POINTS; i++) {
//       float t = dt * i;
//
//       // Initialize sums for x and y
//       float sumX = 0.0f;
//       float sumY = 0.0f;
//
//       // Sum over multiple sine waves and their Hilbert transforms
//       for (int j = 0; j < NUM_WAVES; j++) {
//         float sineWave = amplitudes[j] * sin(frequencies[j] * t);
//         float hilbertTransform = amplitudes[j] * cos(frequencies[j] * t);
//
//         sumX += sineWave;
//         sumY += hilbertTransform;
//       }
//
//       // Create the points for the combined shape
//       points[i] = (Vector2){screenWidth / 2 + sumX, screenHeight / 2 +
//       sumY};
//     }
//
//     // Draw
//     BeginDrawing();
//     ClearBackground(RAYWHITE);
//
//     for (int i = 0; i < NUM_POINTS; i++) {
//       DrawPixelV(points[i], BLACK);
//     }
//
//     EndDrawing();
//   }
//
//   // De-Initialization
//   CloseWindow(); // Close window and OpenGL context
//
//   return 0;
// }

int main() {
  constexpr size_t sample_rate = 48000;
  constexpr size_t buffer_size = 1 << 10;
  constexpr size_t frequency_count = sample_rate / 2 + 1;
  audio::AudioProcessor<sample_rate, buffer_size> processor_("Test");
  Visualizer::AudioStream audio_stream(
      "Google Chrome", sample_rate, buffer_size,
      [&](std::vector<float> samples, size_t sample_rate) {
        std::array<float, buffer_size> new_samples;
        std::move(samples.begin(), samples.end(), new_samples.begin());
        processor_.OnNewSample(std::move(new_samples));
      });

  static float radius = 100;
  InitWindow(1000, 1000, "Example");
  audio_stream.Start();
  while (!WindowShouldClose()) {
    const auto &buffer = processor_.Buffer();
    BeginDrawing();
    ClearBackground(BLACK);
    std::string avg_amplitude =
        "Avg amplitude: " + std::to_string(buffer.avg_amplitude);
    float want_radius = 100 * buffer.avg_amplitude;
    radius += (want_radius - radius) * GetFrameTime() * 20;
    DrawText(avg_amplitude.data(), 10, 10, 24, WHITE);
    DrawCircle(GetRenderWidth() / 2, GetRenderHeight() / 2, radius, RED);
    int cellWidth = GetRenderWidth() / buffer.squashed_samples.size();
    for (int i = 0; i < buffer.squashed_samples.size(); i++) {
      DrawRectangle(i * cellWidth, 0, cellWidth,
                    buffer.squashed_samples[i] * GetRenderHeight() * 0.95,
                    WHITE);
    }
    EndDrawing();
  }
  audio_stream.Stop();
  CloseWindow();
}
