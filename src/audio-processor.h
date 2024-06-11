#pragma once
#include "audio-processing.h"
#include "processed-audio.h"
#include <cstddef>
#include <mutex>
#include <string>
#include <string_view>

namespace audio {

template <size_t sample_rate, size_t sample_count,
          size_t frequencies_count = sample_count / 2 + 1>
class AudioProcessor {
public:
  AudioProcessor(std::string_view source)
      : source_(source), current_buffer_(front_buffer_) {}

  void OnNewSample(std::array<float, sample_count> &&sample) {
    ProcessAudioSamplesIntoBackBuffer(std::move(sample));
    SwitchBuffers();
  }
  const ProcessedAudioBuffer<frequencies_count> &Buffer() {
    std::lock_guard<std::mutex> lock(buffer_lock_);
    return current_buffer_;
  }

private:
  void
  ProcessAudioSamplesIntoBackBuffer(std::array<float, sample_count> &&sample) {
    computeFFT64<sample_count, sample_rate>(std::move(sample),
                                            AlternateBuffer());
  }

  ProcessedAudioBuffer<frequencies_count> &AlternateBuffer() {
    if (is_front_buffer_active_)
      return back_buffer_;
    return front_buffer_;
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

  std::string source_;
  std::mutex buffer_lock_;
  bool is_front_buffer_active_;
  ProcessedAudioBuffer<frequencies_count> front_buffer_;
  ProcessedAudioBuffer<frequencies_count> back_buffer_;
  ProcessedAudioBuffer<frequencies_count> &current_buffer_;
};
} // namespace audio
