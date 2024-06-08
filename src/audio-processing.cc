#include "audio-processing.h"
#include <array>
#include <cassert>
#include <cmath>
#include <fftw3.h>
#include <iostream>
#include <vector>

namespace audio {

constexpr int kBufferSize = (1 << 13) / 2;

auto createHanningWindow() {
  std::array<double, kBufferSize> window{0};
  for (std::size_t i = 0; i < kBufferSize; ++i) {
    window[i] = 0.5 * (1 - std::cos(2 * M_PI * i / (kBufferSize - 1)));
  }
  return window;
}
auto hanning_window = createHanningWindow();
// Move to cc
std::array<int, 22> bands = {0,    100,  200,  300,  400,  510,  630,  770,
                             920,  1080, 1270, 1480, 1720, 2000, 2320, 2700,
                             3150, 3700, 4400, 5300, 6400, 7700};

std::vector<float> BandFrequencies(std::vector<float> fft_out,
                                   int sample_rate) {
  std::vector<float> filtered_bands;
  std::array<std::pair<float, int>, 22> current_band{};
  int current_band_index = 0;

  for (int i = 0; i < fft_out.size(); ++i) {
    float freq = i * (float(sample_rate) / fft_out.size());
    while (current_band_index < 22 && freq >= bands[current_band_index]) {
      current_band_index++;
    }

    if (current_band_index < 22) {
      // if (freq >= bands[current_band_index - 1] &&
      //     freq < bands[current_band_index]) {
      //   std::cout << "Freq banding failed for " << freq
      //             << " >=" << bands[current_band_index - 1] << " and less
      //             than "
      //             << bands[current_band_index] << std::endl;
      // }
      assert(freq >= bands[current_band_index - 1]);
      assert(freq < bands[current_band_index]);
    }
    current_band[current_band_index - 1].first += fft_out[i];
    current_band[current_band_index - 1].second += 1;
  }

  float max = 0;
  for (int i = 0; i < 22; i++) {
    float avg = current_band[i].first / current_band[i].second;
    max = std::max(max, avg);
    filtered_bands.push_back(avg);
  }

  // Normalization
  for (int i = 0; i < 22; i++) {
    filtered_bands[i] /= max;
  }
  return filtered_bands;
}

std::vector<float> computeFFT64(const std::vector<float> &samples,
                                double sample_rate) {
  const int N = samples.size();
  const int maxFrequency = 20000;
  double freqResolution = sample_rate / N; // Frequency resolution
  int maxIndex = static_cast<int>(maxFrequency / freqResolution);
  fftw_complex in[N], out[N];
  fftw_plan p;

  // Initialize input array
  for (int i = 0; i < N; ++i) {
    auto hanning_multiplier = 0.5 * (1 - std::cos(2 * M_PI * i / (N - 1)));
    in[i][0] =
        static_cast<double>(samples.at(i)) * hanning_multiplier; // Real part
    // static_cast<double>(samples.at(i)); // Real part
    in[i][1] = 0.0; // Imaginary part
  }

  // Create plan for FFT
  p = fftw_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

  // Execute FFT
  fftw_execute(p);

  // Compute amplitudes
  std::vector<float> amplitudes(std::min(maxIndex + 1, N / 2 + 1));
  for (int i = 0; i <= maxIndex && i < N / 2 + 1; i++) {
    // amplitudes[i] =
    //     20 * std::log10((sqrt(out[i][0] * out[i][0] + out[i][1] * out[i][1]))
    //     +
    //                     0.000001);
    amplitudes[i] = (sqrt(out[i][0] * out[i][0] + out[i][1] * out[i][1]));
    // amplitudes[i] = 20.0f * std::log10(amplitudes[i] + 0.00000001);
    // reduce dynamic range
    // amplitudes[i] = std::pow(amplitudes[i], 0.3);
  }

  // Cleanup
  fftw_destroy_plan(p);
  fftw_cleanup();

  return amplitudes;
}

// Convert frequency to mel scale
double freqToMel(double freq) {
  return 2595.0 * std::log10(1.0 + freq / 700.0);
}

// Create mel scale indices
std::vector<int> melScaleIndices(int fftSize, double sampleRate) {
  std::vector<int> indices(fftSize / 2 + 1);
  for (int i = 0; i <= fftSize / 2; ++i) {
    double freq = (sampleRate / fftSize) * i;
    indices[i] = static_cast<int>(freqToMel(freq));
  }
  return indices;
}

// Determine FFT bins to render based on mel scale
std::vector<int> binsToRenderMel(int fftSize, double sampleRate,
                                 int numMelBands) {
  std::vector<int> bins(fftSize / 2 + 1);
  std::vector<int> melIndices = melScaleIndices(fftSize, sampleRate);
  int maxMel = melIndices.back();
  for (int i = 0; i <= fftSize / 2; ++i) {
    bins[i] = static_cast<int>(1.0 * melIndices[i] / maxMel * numMelBands);
  }
  return bins;
}

std::vector<float> FrequencyBands16(std::vector<float> frequencies) {
  std::vector<float> bands;
  int freq_index = 1;
  float avg = 0;
  int count = 0;
  int endIndex = 1;
  while (freq_index < frequencies.size()) {
    endIndex = std::min(endIndex, (int)(frequencies.size() - 1));
    float sum = 0;
    float count = 0;
    while (freq_index <= endIndex) {
      sum += frequencies[freq_index++];
      count++;
    }
    bands.push_back(sum / count);
    endIndex = std::ceil(1.06 * endIndex);
  }
  return bands;
}
std::vector<float> FrequencyBands(std::vector<float> frequencies,
                                  int band_count) {
  std::vector<float> bands(band_count);
  int freq_index = 0;
  float avg = 0;
  int count = 0;
  for (int i = 0; i < band_count - 1; i++) {
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
  bands[band_count - 1] = avg / count;
  return bands;
}

} // namespace audio
// namespace audio
//
