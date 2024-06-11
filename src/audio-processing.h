#pragma once
#include "processed-audio.h"
#include <array>
#include <cassert>
#include <cmath>
#include <fftw3.h>
#include <iostream>
#include <vector>
namespace audio {

std::vector<float> BandFrequencies(std::vector<float> fft_out, int sample_rate);
struct FftOptions {
  size_t max_frequency = 20000;
};

template <size_t sample_count, size_t sample_rate,
          size_t out_size = sample_count / 2 + 1>
void computeFFT64(std::array<float, sample_count> &&samples,
                  ProcessedAudioBuffer<out_size> &output,
                  FftOptions options = FftOptions{}) {

  static std::array<double, sample_count> hanning_multipliers = []() {
    std::array<double, sample_count> window;

    for (std::size_t i = 0; i < sample_count; ++i) {
      window[i] = 0.5 * (1 - std::cos(2 * M_PI * i / (sample_count - 1)));
    }
    return window;
  }();

  fftw_complex in[sample_count], out[sample_count];

  const int maxFrequency = options.max_frequency;

  double freqResolution =
      float(sample_rate) / sample_count; // Frequency resolution
                                         //
  size_t maxIndex = static_cast<size_t>(maxFrequency / freqResolution);

  fftw_plan p;

  // Initialize input array
  for (int i = 0; i < sample_count; ++i) {
    in[i][0] = static_cast<double>(samples.at(i)) *
               hanning_multipliers[i]; // Real Part
    in[i][1] = 0.0;                    // Imaginary part
  }

  // Create plan for FFT
  p = fftw_plan_dft_1d(sample_count, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

  // Execute FFT
  fftw_execute(p);

  float max_amplitude = 0;
  float sum = 0;

  for (int i = 0; i <= maxIndex && i < out_size; i++) {
    // amplitudes[i] =
    //     20 * std::log10((sqrt(out[i][0] * out[i][0] + out[i][1] * out[i][1]))
    //     +
    //                     0.000001);
    output.samples[i].sine_component = out[i][0];
    output.samples[i].cosine_component = out[i][1];

    float amplitude = (sqrt(out[i][0] * out[i][0] + out[i][1] * out[i][1]));
    max_amplitude = std::max(max_amplitude, amplitude);
    sum += amplitude;
    output.samples[i].normalized_amplitude = amplitude;

    // amplitudes[i] = 20.0f * std::log10(amplitudes[i] + 0.00000001);
    // reduce dynamic range
    // amplitudes[i] = std::pow(amplitudes[i], 0.3);
  }

  output.max_amplitude = max_amplitude;
  output.avg_amplitude = sum / std::min(maxIndex, out_size);
  std::cout << " >" << out[5][0] << ", " << out[5][1];

  for (int i = 0; i <= maxIndex && i < out_size; i++) {
    output.samples[i].normalized_amplitude /= max_amplitude;
  }

  // Cleanup
  fftw_destroy_plan(p);
  fftw_cleanup();
}

std::vector<int> binsToRenderMel(int fftSize, double sampleRate,
                                 int numMelBands);
std::vector<float> FrequencyBands(std::vector<float> frequencies,
                                  int band_count);
std::vector<float> FrequencyBands16(std::vector<float> frequencies);
} // namespace audio
