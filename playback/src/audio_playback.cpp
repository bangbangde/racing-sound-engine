#include "engin/audio_playback.h"
#include "synth_engine.h"
#include "miniaudio.h"
#include <cstring>

namespace engin {

struct AudioPlayback::MaDeviceWrapper {
    ma_device device;
};

AudioPlayback::AudioPlayback() = default;

AudioPlayback::~AudioPlayback() {
    shutdown();
}

bool AudioPlayback::init(SynthEngine& synth, SampleRate sample_rate, FrameCount buffer_frames) {
    synth_ = &synth;
    device_ = new MaDeviceWrapper();

    ma_device_config dev_config = ma_device_config_init(ma_device_type_playback);
    dev_config.playback.format   = ma_format_f32;
    dev_config.playback.channels = 1; // mono
    dev_config.sampleRate        = sample_rate;
    dev_config.periodSizeInFrames = buffer_frames;
    dev_config.dataCallback      = [](ma_device* dev, void* out, const void* /*in*/, ma_uint32 fc) {
        auto* pb = static_cast<AudioPlayback*>(dev->pUserData);
        if (pb->synth_) {
            pb->synth_->process(static_cast<Sample*>(out), static_cast<FrameCount>(fc));
        } else {
            std::memset(out, 0, fc * sizeof(float));
        }
    };
    dev_config.pUserData = this;

    if (ma_device_init(nullptr, &dev_config, &device_->device) != MA_SUCCESS) {
        delete device_;
        device_ = nullptr;
        return false;
    }

    return true;
}

void AudioPlayback::start() {
    if (device_ && !running_.load()) {
        ma_device_start(&device_->device);
        running_.store(true, std::memory_order_release);
    }
}

void AudioPlayback::stop() {
    if (device_ && running_.load()) {
        running_.store(false, std::memory_order_release);
        ma_device_stop(&device_->device);
    }
}

void AudioPlayback::shutdown() {
    stop();
    if (device_) {
        ma_device_uninit(&device_->device);
        delete device_;
        device_ = nullptr;
    }
    synth_ = nullptr;
}

} // namespace engin
