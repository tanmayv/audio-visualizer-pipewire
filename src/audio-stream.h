#include "pipewire/main-loop.h"
#include "pipewire/stream.h"
#include "spa/param/audio/format.h"
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace Visualizer {
class AudioStream {
public:
  using FreqCallback = std::function<void(std::vector<float>)>;

  struct Context {
    struct pw_main_loop *loop;
    struct pw_stream *stream;
    struct spa_audio_info format;
    unsigned move : 1;
  };

  AudioStream(std::string source_name, FreqCallback freq_callback)
      : source_name_(std::move(source_name)), freq_callback_(freq_callback),
        context_(CreateContext()) {}
  void Start();
  void Stop();
  void OnProcess();
  void OnStreamParamChanged(uint32_t id, const struct spa_pod *param);
  void OnQuit(int signal_number);

private:
  std::unique_ptr<Context> CreateContext();

  std::string source_name_;

  FreqCallback freq_callback_;
  std::unique_ptr<Context> context_;
};
} // namespace Visualizer