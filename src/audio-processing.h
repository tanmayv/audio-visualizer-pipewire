#include <algorithm>
#include <array>
#include <cassert>
#include <iostream>
#include <tuple>
#include <utility>
#include <vector>
namespace audio {
// Should be equal to sample count
// constexpr int kMaxWindowSize = 1024;

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

} // namespace audio
