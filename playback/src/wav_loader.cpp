#include "wav_loader.h"
#include "miniaudio.h"

namespace engin {

WavData load_wav(const char* path, SampleRate target_rate) {
    WavData result;

    ma_decoder_config config = ma_decoder_config_init(ma_format_f32, 1, target_rate);
    ma_decoder decoder;

    if (ma_decoder_init_file(path, &config, &decoder) != MA_SUCCESS) {
        return result;
    }

    // Get frame count
    ma_uint64 frame_count;
    ma_decoder_get_length_in_pcm_frames(&decoder, &frame_count);

    result.pcm.resize(static_cast<size_t>(frame_count));
    result.sample_rate = target_rate;

    ma_uint64 frames_read;
    ma_decoder_read_pcm_frames(&decoder, result.pcm.data(), frame_count, &frames_read);
    result.pcm.resize(static_cast<size_t>(frames_read));
    result.num_frames = static_cast<uint32_t>(frames_read);

    ma_decoder_uninit(&decoder);

    return result;
}

} // namespace engin
