#include <vector>
namespace audio {
// Should be equal to sample count
// constexpr int kMaxWindowSize = 1024;

std::vector<float> BandFrequencies(std::vector<float> fft_out, int sample_rate);
std::vector<float> computeFFT64(const std::vector<float> &samples,
                                double sample_rate);
std::vector<int> binsToRenderMel(int fftSize, double sampleRate,
                                 int numMelBands);
std::vector<float> FrequencyBands(std::vector<float> frequencies,
                                  int band_count);
std::vector<float> FrequencyBands16(std::vector<float> frequencies);
} // namespace audio
