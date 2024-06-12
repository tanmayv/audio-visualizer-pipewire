#include "pipewire/stream.h"
#include "pipewire/thread-loop.h"
#include "spa/param/audio/format.h"
#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace Visualizer {
class AudioStream {
public:
  using FreqCallback = std::function<void(std::vector<float>, int)>;

  struct Context {
    struct pw_thread_loop *loop;
    struct pw_stream *stream;
    struct spa_audio_info format;
    unsigned move : 1;
  };

  AudioStream(std::string source_name, size_t sample_rate, size_t buffer_size,
              FreqCallback freq_callback)
      : source_name_(std::move(source_name)), sample_rate_(sample_rate),
        buffer_size_(buffer_size), freq_callback_(freq_callback),
        context_(CreateContext()) {}
  void Start();
  void Stop();
  void OnProcess();
  void OnStreamParamChanged(uint32_t id, const struct spa_pod *param);

private:
  std::unique_ptr<Context> CreateContext();

  std::string source_name_;
  size_t sample_rate_;
  size_t buffer_size_;

  FreqCallback freq_callback_;
  std::unique_ptr<Context> context_;
};
} // namespace Visualizer
