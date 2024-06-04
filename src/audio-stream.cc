#include "audio-stream.h"
#include "pipewire/pipewire.h"
#include "pipewire/thread-loop.h"
#include "spa/param/audio/raw-utils.h"
#include "spa/pod/builder.h"
#include "spa/pod/pod.h"
#include <cmath>
#include <cstdint>
#include <fftw3.h>
#include <memory>

namespace Visualizer {
namespace {

constexpr int kBufferSize = 512;
std::vector<float> computeFFT64(const float *inputArray, int length,
                                int n_channels) {
  int single_channel_size = length / n_channels;
  const int N = single_channel_size;
  fftw_complex in[N], out[N];
  fftw_plan p;

  // Initialize input array
  for (int i = 0; i < N; ++i) {
    if (i < single_channel_size) {
      in[i][0] = static_cast<double>(inputArray[2 * i]); // Real part
      in[i][1] = 0.0;                                    // Imaginary part
    } else {
      in[i][0] = 0.0; // Padding with zeros if input length < N
      in[i][1] = 0.0;
    }
  }

  // Create plan for FFT
  p = fftw_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

  // Execute FFT
  fftw_execute(p);

  // Compute amplitudes
  std::vector<float> amplitudes(N / 2 + 1);
  for (int i = 0; i < N / 2 + 1; i++) {
    amplitudes[i] = sqrt(out[i][0] * out[i][0] + out[i][1] * out[i][1]);
  }

  // Cleanup
  fftw_destroy_plan(p);
  fftw_cleanup();

  return amplitudes;
}

static void on_process(void *as) {
  auto *audio_stream = static_cast<AudioStream *>(as);
  audio_stream->OnProcess();
}

/* Be notified when the stream param changes. We're only looking at the
 * format changes.
 */

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

static void do_quit(void *as, int signal_number) {
  auto *audio_stream = static_cast<AudioStream *>(as);
  audio_stream->OnQuit(signal_number);
}

} // namespace

void AudioStream::Start() {

  uint8_t buffer[kBufferSize];
  struct spa_pod_builder b = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));

  /* Make one parameter with the supported formats. The SPA_PARAM_EnumFormat
   * id means that this is a format enumeration (of 1 value).
   * We leave the channels and rate empty to accept the native graph
   * rate and channels. */
  const struct spa_pod *params[1];
  auto placeholder = SPA_AUDIO_INFO_RAW_INIT(.format = SPA_AUDIO_FORMAT_F32);
  params[0] =
      spa_format_audio_raw_build(&b, SPA_PARAM_EnumFormat, &placeholder);

  /* Now connect this stream. We ask that our process function is
   * called in a realtime thread. */
  pw_stream_connect(
      context_->stream, PW_DIRECTION_INPUT, PW_ID_ANY,
      static_cast<enum pw_stream_flags>(PW_STREAM_FLAG_AUTOCONNECT |
                                        PW_STREAM_FLAG_MAP_BUFFERS),
      params, 1);

  /* and wait while we let things run */
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
  freq_callback_(computeFFT64(samples, n_samples, n_channels));
  pw_stream_queue_buffer(context_->stream, b);
}
void AudioStream::OnStreamParamChanged(uint32_t id,
                                       const struct spa_pod *param) {
  /* NULL means to clear the format */
  if (param == NULL || id != SPA_PARAM_Format)
    return;

  if (spa_format_parse(param, &context_->format.media_type,
                       &context_->format.media_subtype) < 0)
    return;

  /* only accept raw audio */
  if (context_->format.media_type != SPA_MEDIA_TYPE_audio ||
      context_->format.media_subtype != SPA_MEDIA_SUBTYPE_raw)
    return;

  /* call a helper function to parse the format for us. */
  spa_format_audio_raw_parse(param, &context_->format.info.raw);

  fprintf(stdout, "capturing rate:%d channels:%d\n",
          context_->format.info.raw.rate, context_->format.info.raw.channels);
}

void AudioStream::OnQuit(int signal_number) {
  // pw_thread_loop_quit(context_->loop);
}

std::unique_ptr<AudioStream::Context> AudioStream::CreateContext() {
  auto context = std::make_unique<Context>();
  const struct spa_pod *params[1];
  struct pw_properties *props;

  pw_init(nullptr, nullptr);

  /* make a main loop. If you already have another main loop, you can add
   * the fd of this pipewire mainloop to it. */
  context->loop = pw_thread_loop_new("audio-capture-thread", NULL);

  // pw_loop_add_signal(pw_thread_loop_get_loop(context->loop), SIGINT, do_quit,
  //                    this);
  // pw_loop_add_signal(pw_thread_loop_get_loop(context->loop), SIGTERM,
  // do_quit,
  //                    this);

  /* Create a simple stream, the simple stream manages the core and remote
   * objects for you if you don't need to deal with them.
   *
   * If you plan to autoconnect your stream, you need to provide at least
   * media, category and role properties.
   *
   * Pass your events and a user_data pointer as the last arguments. This
   * will inform you about the stream state. The most important event
   * you need to listen to is the process event where you need to produce
   * the data.
   */
  props = pw_properties_new(PW_KEY_MEDIA_TYPE, "Audio", PW_KEY_CONFIG_NAME,
                            "client-rt.conf", PW_KEY_MEDIA_CATEGORY, "Capture",
                            PW_KEY_MEDIA_ROLE, "Music", NULL);
  /* Set stream target if given on command line */
  pw_properties_set(props, PW_KEY_TARGET_OBJECT, source_name_.data());

  /* uncomment if you want to capture from the sink monitor ports */
  /* pw_properties_set(props, PW_KEY_STREAM_CAPTURE_SINK, "true"); */

  context->stream =
      pw_stream_new_simple(pw_thread_loop_get_loop(context->loop),
                           "audio-capture", props, &stream_events, this);
  return context;
}
} // namespace Visualizer
