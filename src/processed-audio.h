#pragma once
#include <array>
#include <cmath>
#include <cstddef>

namespace audio {
constexpr auto squashed_sample_size = [](size_t raw_samples_count) {
  size_t count = 0;
  float step = 1.06;
  float lowf = 1.0f;
  for (float f = lowf; (size_t)f < raw_samples_count / 2;
       f = std::ceilf(f * step)) {
    count++;
  }
  return count;
};

struct ProcessedAudioSample {
  float sine_component;
  float cosine_component;
  float normalized_amplitude;
  float frequency;
};

struct FreqAmpPair {
  float normalized_amplitude;
  float frequency;
};

template <size_t sample_count> struct ProcessedAudioBuffer {
  std::array<ProcessedAudioSample, sample_count> samples;
  std::array<FreqAmpPair, squashed_sample_size(sample_count)> squashed_samples;
  float max_amplitude;
  float avg_amplitude;
};

} // namespace audio
