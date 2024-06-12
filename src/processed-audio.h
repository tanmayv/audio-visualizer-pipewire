#pragma once
#include <cmath>
#include <cstddef>
#include <vector>

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

class ProcessedAudioBuffer {
public:
  explicit ProcessedAudioBuffer(size_t buffer_size)
      : samples(buffer_size),
        squashed_samples(squashed_sample_size(buffer_size)) {}

  std::vector<ProcessedAudioSample> samples;
  std::vector<FreqAmpPair> squashed_samples;
  float max_amplitude;
  float avg_amplitude;
};

// template <size_t sample_count> struct ProcessedAudioBuffer {};

} // namespace audio
