#pragma once
#include <array>
#include <cstddef>
namespace audio {

struct ProcessedAudioSample {
  float sine_component;
  float cosine_component;
  float normalized_amplitude;
};

template <size_t sample_count> struct ProcessedAudioBuffer {
  std::array<ProcessedAudioSample, sample_count> samples;
  float max_amplitude;
  float avg_amplitude;
};

} // namespace audio
