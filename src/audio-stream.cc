#include "audio-stream.h"
#include "pipewire/pipewire.h"
#include "pipewire/stream.h"
#include "pipewire/thread-loop.h"
#include "spa/param/audio/raw-utils.h"
#include "spa/pod/builder.h"
#include "spa/pod/pod.h"
#include <cstdint>
#include <fftw3.h>
#include <memory>

namespace Visualizer {
namespace {

static void on_process(void *as) {
  auto *audio_stream = static_cast<AudioStream *>(as);
  audio_stream->OnProcess();
}

static void on_stream_param_changed(void *as, uint32_t id,
                                    const struct spa_pod *param) {
  auto *audio_stream = static_cast<AudioStream *>(as);
  audio_stream->OnStreamParamChanged(id, param);
}

static const struct pw_stream_events stream_events = {
    PW_VERSION_STREAM_EVENTS,
    .param_changed = on_stream_param_changed,
    .process = on_process,
};

} // namespace

void AudioStream::Start() {

  size_t buffer[buffer_size_];
  struct spa_pod_builder b = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));

  const struct spa_pod *params[1];
  auto placeholder =
      SPA_AUDIO_INFO_RAW_INIT(.format = SPA_AUDIO_FORMAT_F32,
                              .rate = sample_rate_, .channels = 1);
  params[0] =
      spa_format_audio_raw_build(&b, SPA_PARAM_EnumFormat, &placeholder);
  pw_stream_connect(
      context_->stream, PW_DIRECTION_INPUT, PW_ID_ANY,
      static_cast<enum pw_stream_flags>(PW_STREAM_FLAG_AUTOCONNECT |
                                        PW_STREAM_FLAG_MAP_BUFFERS),
      params, 1);

  pw_thread_loop_start(context_->loop);
}
void AudioStream::Stop() {
  pw_thread_loop_stop(context_->loop);
  pw_stream_destroy(context_->stream);
  pw_thread_loop_destroy(context_->loop);
  pw_deinit();
}

void AudioStream::OnProcess() {

  struct pw_buffer *b;
  struct spa_buffer *buf;
  float *samples, max;
  uint32_t c, n, n_channels, n_samples, peak;

  if ((b = pw_stream_dequeue_buffer(context_->stream)) == NULL) {
    pw_log_warn("out of buffers: %m");
    return;
  }

  buf = b->buffer;
  if ((samples = static_cast<float *>(buf->datas[0].data)) == NULL)
    return;

  n_channels = context_->format.info.raw.channels;
  n_samples = buf->datas[0].chunk->size / sizeof(float);
  // FFT
  std::vector<float> samples_to_process;
  samples_to_process.reserve(n_samples / n_channels);
  for (int i = 0; i < n_samples; i += n_channels) {
    samples_to_process.push_back(samples[i]);
  }
  pw_stream_queue_buffer(context_->stream, b);
  freq_callback_(samples_to_process,
                 context_->format.info.raw.rate / n_channels);
}

void AudioStream::OnStreamParamChanged(uint32_t id,
                                       const struct spa_pod *param) {
  if (param == NULL || id != SPA_PARAM_Format)
    return;

  if (spa_format_parse(param, &context_->format.media_type,
                       &context_->format.media_subtype) < 0)
    return;

  if (context_->format.media_type != SPA_MEDIA_TYPE_audio ||
      context_->format.media_subtype != SPA_MEDIA_SUBTYPE_raw)
    return;

  spa_format_audio_raw_parse(param, &context_->format.info.raw);

  fprintf(stdout, "capturing rate:%d channels:%d\n",
          context_->format.info.raw.rate, context_->format.info.raw.channels);
}

std::unique_ptr<AudioStream::Context> AudioStream::CreateContext() {
  auto context = std::make_unique<Context>();
  const struct spa_pod *params[1];
  struct pw_properties *props;

  pw_init(nullptr, nullptr);

  auto thread_name = source_name_ + "_audio";
  context->loop = pw_thread_loop_new(thread_name.data(), NULL);

  props = pw_properties_new(PW_KEY_MEDIA_TYPE, "Audio", PW_KEY_CONFIG_NAME,
                            "client-rt.conf", PW_KEY_MEDIA_CATEGORY, "Capture",
                            PW_KEY_MEDIA_ROLE, "Music", NULL);
  pw_properties_set(props, PW_KEY_TARGET_OBJECT, source_name_.data());

  context->stream =
      pw_stream_new_simple(pw_thread_loop_get_loop(context->loop),
                           "chrome-audio-capture", props, &stream_events, this);
  return context;
}
} // namespace Visualizer
