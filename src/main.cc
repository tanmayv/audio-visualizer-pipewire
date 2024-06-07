#include "audio-processing.h"
#include "audio-stream.h"
#include "raylib.h"
#include <algorithm>
#include <atomic>
#include <cassert>
#include <iostream>
#include <mutex>
#include <string>
#include <vector>

std::mutex samplesMtx;
std::vector<float> samples;
std::atomic<int> sample_rate;

std::vector<float> frequencies;
std::vector<float> renderedFrequencies;

// Running on Audio Capture Thread
static void OnFrequencyBuffer(std::vector<float> s, int sr) {
  sample_rate = sr;
  std::lock_guard<std::mutex> lock(samplesMtx);
  samples = std::move(s);
}

static void ProcessAudio() {
  std::lock_guard<std::mutex> lock(samplesMtx);
  if (samples.size() == 0)
    return;
  frequencies = audio::computeFFT64(samples, sample_rate);
  frequencies = audio::FrequencyBands(frequencies, 8);
  if (renderedFrequencies.size() < frequencies.size()) {
    // renderedFrequencies.clear();
    // for (auto f : frequencies) {
    //   renderedFrequencies.push_back(f);
    // }
    renderedFrequencies = frequencies;
  }
  ClearBackground(WHITE);
}

static std::string GetFreqStr(int index, int count) {
  return std::to_string(index * sample_rate / count);
}

static void RenderFrequencies() {
  if (frequencies.size() == 0) {
    return;
  }
  int canvasHeight = GetRenderHeight();
  int canvasWidth = GetRenderWidth();
  int freqCount = frequencies.size();
  int freqCountToShow = std::min(freqCount, 20) - 1;
  int lineHeight = ((canvasHeight - 20) / freqCountToShow);

  int cursor = 0;
  float prevAmplitude = frequencies.at(0);
  float amplitude = frequencies.at(1);
  for (int y = 1; y < freqCount - 1 && cursor < freqCountToShow; ++y) {
    float nextAmplitude = frequencies.at(y + 1);
    if (amplitude > 0.1 && amplitude >= prevAmplitude &&
        amplitude > nextAmplitude) {
      std::string freqValue = GetFreqStr(y, freqCount);
      DrawText(freqValue.data(), 10, 10 + cursor * lineHeight, lineHeight - 2,
               WHITE);
      DrawText(std::to_string(amplitude).data(), 500, 10 + cursor * lineHeight,
               lineHeight - 2, BLUE);
      cursor++;
    }
    prevAmplitude = amplitude;
    amplitude = nextAmplitude;
  }
  std::string total = "Total frequencies: " + std::to_string(freqCount);
  DrawText(total.data(), 10, 10 + cursor * lineHeight, lineHeight - 2, RED);
}

// Running on UI Thread
static void Render() {

  // For some reason this has to be before acquiring lock;
  DrawFPS(10, 10);

  // Draw a red rectangle
  int size = frequencies.size();
  int canvasWidth = GetRenderWidth();
  int canvasHeight = GetRenderHeight();
  if (size == 0)
    return;
  int cellWidth = canvasWidth / size;
  std::string tot = "total freq: " + std::to_string(size);
  DrawText(tot.data(), 100, 100, 24, WHITE);
  assert(renderedFrequencies.size() == frequencies.size());
  for (int i = 0; i < size; ++i) {
    renderedFrequencies[i] +=
        (frequencies[i] - renderedFrequencies[i]) * GetFrameTime() * 20;
    int barHeight = std::min(20 * renderedFrequencies.at(i), 1080.0f);
    DrawRectangle(i * cellWidth, canvasHeight - barHeight - 20, cellWidth,
                  barHeight, RED);
  }
}

// static void RenderLogLike() {
//   int start = 0;
//   int end = double logStart = std::log10(start);
// }

int main() {
  const int scale = 1;
  const int screenWidth = 513;
  const int screenHeight = 1080 * scale;
  SetTargetFPS(60);
  Visualizer::AudioStream audio_stream("Google Chrome", OnFrequencyBuffer);
  InitWindow(screenWidth, screenHeight, "Example");
  audio_stream.Start();
  while (!WindowShouldClose()) {
    ProcessAudio();
    BeginDrawing();
    ClearBackground(BLACK);
    // RenderFrequencies();
    Render();
    EndDrawing();
  }
  audio_stream.Stop();
  CloseWindow();
}
