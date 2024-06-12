#pragma once
#include "processed-audio.h"
#include <cstddef>
#include <vector>

namespace audio {

struct FftOptions {
  size_t max_frequency = 30000;
};

void computeFFT64(std::vector<float> &&samples, size_t sample_rate,
                  ProcessedAudioBuffer &output,
                  FftOptions options = FftOptions{});
} // namespace audio
