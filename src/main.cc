#include "audio-stream.h"
#include "raylib.h"
#include <iostream>
#include <mutex>
#include <vector>

std::mutex freqBufferMtx;
std::vector<float> freqBuffer;

// Running on Audio Capture Thread
static void OnFrequencyBuffer(std::vector<float> buffer) {
  std::lock_guard<std::mutex> lock(freqBufferMtx);
  freqBuffer = std::move(buffer);
}

// Running on UI Thread
static void Render() {

  // For some reason this has to be before acquiring lock;
  BeginDrawing();
  ClearBackground(WHITE);
  DrawFPS(10, 10);
  std::lock_guard<std::mutex> guard(freqBufferMtx);

  // Draw a red rectangle
  int size = freqBuffer.size();
  int canvasWidth = GetRenderWidth();
  int canvasHeight = GetRenderHeight();
  if (size == 0)
    return;
  int cellWidth = canvasWidth / size;
  for (int i = 0; i < size; ++i) {
    int barHeight = canvasHeight * 0.5 * freqBuffer.at(i);
    DrawRectangle(i * cellWidth, canvasHeight - barHeight, cellWidth, barHeight,
                  RED);
  }
  EndDrawing();
}

int main() {
  const int screenWidth = 1030;
  const int screenHeight = 450;
  SetTargetFPS(60);
  Visualizer::AudioStream audio_stream(
      "Google Chrome",
      [](std::vector<float> buffer) { OnFrequencyBuffer(buffer); });
  InitWindow(screenWidth, screenHeight, "Example");
  audio_stream.Start();
  while (!WindowShouldClose()) {
    Render();
  }
  audio_stream.Stop();
  CloseWindow();
}
