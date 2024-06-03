#include "audio-stream.h"
#include "raylib.h"
#include <iostream>
#include <vector>

#include <thread>

void printThreadID() {
  std::cout << "Thread ID: " << std::this_thread::get_id() << std::endl;
}

static void OnFrequencyBuffer(std::vector<float> buffer) {
  std::cout << "Frequencies " << buffer.size() << std::endl;
  float peak1 = buffer.at(0);
  float peak2 = buffer.at(1);

  printThreadID();
  BeginDrawing();

  ClearBackground(WHITE);

  // Draw a red rectangle
  DrawRectangle(100, 100, 400 * peak1, 100, RED);
  DrawRectangle(100, 300, 400 * peak2, 100, RED);

  EndDrawing();
}

int main() {
  const int screenWidth = 800;
  const int screenHeight = 450;
  printThreadID();
  SetTargetFPS(60);
  Visualizer::AudioStream audio_stream(
      "Google Chrome",
      [](std::vector<float> buffer) { OnFrequencyBuffer(buffer); });
  InitWindow(screenWidth, screenHeight, "Example");
  audio_stream.Start();
  audio_stream.Stop();
  CloseWindow();
}
