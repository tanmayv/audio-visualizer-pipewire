#pragma once
#include "audio-processing.h"
#include "processed-audio.h"
#include <cstddef>
#include <mutex>
#include <string>
#include <string_view>
#include <vector>

namespace audio {

class AudioProcessor {
public:
  AudioProcessor(std::string_view source, size_t sample_rate,
                 size_t sample_count)
      : source_(source), sample_rate_(sample_rate),
        front_buffer_(sample_count / 2 + 1), back_buffer_(sample_count / 2 + 1),
        current_buffer_(front_buffer_) {}

private:
  ProcessedAudioBuffer &AlternateBuffer() {
    if (is_front_buffer_active_)
      return back_buffer_;
    return front_buffer_;
  }

  void ProcessAudioSamplesIntoBackBuffer(std::vector<float> &&sample) {
    computeFFT64(std::move(sample), sample_rate_, AlternateBuffer());
  }

  void SwitchBuffers() {
    std::lock_guard<std::mutex> lock(buffer_lock_);
    if (is_front_buffer_active_) {
      current_buffer_ = back_buffer_;
    } else {
      current_buffer_ = front_buffer_;
    }
    is_front_buffer_active_ = !is_front_buffer_active_;
  }

public:
  void OnNewSample(std::vector<float> &&sample) {
    ProcessAudioSamplesIntoBackBuffer(std::move(sample));
    SwitchBuffers();
  }
  const ProcessedAudioBuffer &Buffer() {
    std::lock_guard<std::mutex> lock(buffer_lock_);
    return current_buffer_;
  }

private:
  std::string source_;
  size_t sample_rate_;
  std::mutex buffer_lock_;
  bool is_front_buffer_active_;
  ProcessedAudioBuffer front_buffer_;
  ProcessedAudioBuffer back_buffer_;
  ProcessedAudioBuffer &current_buffer_;
};
} // namespace audio
