#include "audio-stream.h"
#include "raylib.h"
#include <atomic>
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

#include <thread>

void printThreadID() {
  std::cout << "Thread ID: " << std::this_thread::get_id() << std::endl;
}

std::vector<float> freqBuffer;

// Running on Audio Capture Thread
static void OnFrequencyBuffer(std::vector<float> buffer) {
  freqBuffer = std::move(buffer);
}

// Running on UI Thread
static void Render() {
  BeginDrawing();

  ClearBackground(WHITE);

  // Draw a red rectangle
  int size = freqBuffer.size();
  int canvasWidth = GetRenderWidth();
  int canvasHeight = GetRenderHeight();
  if (size == 0)
    return;
  assert(canvasWidth >= size);
  int cellWidth = canvasWidth / size;
  for (int i = 0; i < size; ++i) {
    int barHeight = std::log2(1 + 10 * freqBuffer.at(i)) * canvasHeight * 0.05;
    // int barHeight = 10 * freqBuffer.at(i);
    DrawRectangle(i * cellWidth, canvasHeight - barHeight, cellWidth, barHeight,
                  RED);
  }

  EndDrawing();
}

int main() {
  const int screenWidth = 1030;
  const int screenHeight = 450;
  printThreadID();
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
