#pragma once
#include "processed-audio.h"
#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <fftw3.h>
#include <ostream>
#include <vector>
namespace audio {

std::vector<float> BandFrequencies(std::vector<float> fft_out, int sample_rate);
struct FftOptions {
  size_t max_frequency = 30000;
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
    output.samples[i].frequency = i * sample_rate / sample_count;

    // float amplitude =
    //     (std::logf(out[i][0] * out[i][0] + out[i][1] * out[i][1]));
    float amplitude =
        (std::sqrt(out[i][0] * out[i][0] + out[i][1] * out[i][1]));
    max_amplitude = std::max(max_amplitude, amplitude);
    sum += amplitude;
    output.samples[i].normalized_amplitude = amplitude;
  }

  output.max_amplitude = max_amplitude;
  output.avg_amplitude = sum / std::min(maxIndex, out_size);

  for (int i = 0; i <= maxIndex && i < out_size; i++) {
    output.samples[i].normalized_amplitude /= max_amplitude;
  }

  float step = 1.06;
  float lowf = 1.0f;
  size_t m = 0;
  float max_amp = 1.0f;

  for (float f = lowf; (size_t)f < out_size / 2; f = std::ceilf(f * step)) {
    float f1 = std::ceilf(f * step);
    float a = 0.0f;
    float ff = 0.0f;
    for (size_t q = (size_t)f; q < out_size / 2 && q < (size_t)f1; ++q) {
      float b = output.samples[q].normalized_amplitude;
      if (b > a) {
        a = b;
        ff = output.samples[q].frequency;
      }
    }
    output.squashed_samples[m++] = {.normalized_amplitude = a, .frequency = ff};
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

template <size_t band_size, size_t sample_count>
const std::array<float, band_size> &
FrequencyBands(std::array<float, sample_count> frequencies) {
  static std::array<float, band_size> bands;
  int freq_index = 0;
  float avg = 0;
  int count = 0;
  for (int i = 0; i < band_size - 1; i++) {
    avg = 0;
    count = 0;
    while (freq_index <= std::pow(2, i)) {
      avg += frequencies[freq_index++];
      count++;
    }
    bands[i] = avg / count;
  }
  while (freq_index < frequencies.size()) {
    avg += frequencies[freq_index++];
    count++;
  }
  bands[band_size - 1] = avg / count;
  return bands;
}
} // namespace audio
