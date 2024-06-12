#include "audio-processing.h"
#include <cstddef>
#include <fftw3.h>
#include <mutex>
#include <vector>
namespace audio {
static std::mutex fftw_mtx;

void computeFFT64(std::vector<float> &&samples, size_t sample_rate,
                  ProcessedAudioBuffer &output, FftOptions options) {

  size_t sample_count = samples.size();
  size_t out_size = sample_count / 2 + 1;
  static std::vector<double> hanning_multipliers = [&]() {
    std::vector<double> window(sample_count);

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
  {
    std::lock_guard<std::mutex> fftw_guard(fftw_mtx);
    p = fftw_plan_dft_1d(sample_count, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
  }

  // Execute FFT
  fftw_execute(p);

  float max_amplitude = 0;
  float sum = 0;

  for (int i = 0; i <= maxIndex && i < out_size; i++) {
    output.samples[i].sine_component = out[i][0];
    output.samples[i].cosine_component = out[i][1];
    output.samples[i].frequency =
        static_cast<float>(i) * sample_rate / sample_count;
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

  {
    std::lock_guard<std::mutex> fftw_guard(fftw_mtx);
    fftw_destroy_plan(p);
    fftw_cleanup();
  }
}

} // namespace audio
