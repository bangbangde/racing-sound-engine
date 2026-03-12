#pragma once

#include "core/types.h"
#include "core/constants.h"
#include "core/parameter_controller.h"
#include "core/cylinder_bank.h"
#include "core/engine_model.h"
#include "synth/harmonic_synth.h"
#include "synth/noise_generator.h"
#include "synth/sample_player.h"
#include "synth/turbo_whistle.h"
#include "effects/biquad_filter.h"
#include "effects/distortion.h"
#include "effects/reverb.h"
#include "effects/effects_chain.h"
#include "mixer/audio_mixer.h"
#include "presets/engine_preset.h"

#include <atomic>

namespace engin {

struct AudioConfig {
    SampleRate sample_rate   = DEFAULT_SAMPLE_RATE;
    FrameCount buffer_frames = DEFAULT_BUFFER_FRAMES;
};

class AudioEngine {
public:
    AudioEngine();
    ~AudioEngine();

    bool init(const AudioConfig& config = {});
    void start();
    void stop();
    void shutdown();

    void load_preset(const EnginePreset& preset);
    void set_params(const EngineParams& params);

    // Offline rendering (bypasses miniaudio, for testing)
    void render_to_buffer(Sample* output, FrameCount frames);

    ParameterController& get_param_controller() { return params_; }
    SamplePlayer& get_sample_player() { return sample_player_; }

    SampleRate get_sample_rate() const { return sample_rate_; }
    bool is_running() const { return running_.load(std::memory_order_relaxed); }

private:
    // miniaudio device handle (opaque to header)
    struct MaDeviceWrapper;
    MaDeviceWrapper* device_ = nullptr;

    SampleRate sample_rate_   = DEFAULT_SAMPLE_RATE;
    FrameCount buffer_frames_ = DEFAULT_BUFFER_FRAMES;
    std::atomic<bool> running_{false};

    // Sub-modules
    ParameterController   params_;
    CylinderBank          cylinder_bank_;
    HarmonicSynthesizer   harmonic_synth_;
    NoiseGenerator        intake_noise_;
    NoiseGenerator        exhaust_noise_;
    SamplePlayer          sample_player_;
    TurboWhistle          turbo_whistle_;
    BiquadFilter          exhaust_resonances_[MAX_EXHAUST_RESONANCES];
    int                   num_exhaust_resonances_ = 1;
    BiquadFilter          exhaust_lowpass_;       // high-freq roll-off after resonances
    BiquadFilter          intake_filter_;
    Distortion            distortion_;
    SimpleReverb          reverb_;
    AudioMixer            mixer_;

    // Current preset (stored atomically for audio thread)
    std::atomic<const EnginePreset*> current_preset_{nullptr};
    EnginePreset cached_preset_; // local copy in audio thread

    // Scratch buffers
    static constexpr FrameCount MAX_SCRATCH = 4096;
    Sample scratch_cylinders_[MAX_SCRATCH] = {};
    Sample scratch_harmonics_[MAX_SCRATCH] = {};
    Sample scratch_intake_noise_[MAX_SCRATCH] = {};
    Sample scratch_exhaust_noise_[MAX_SCRATCH] = {};
    Sample scratch_samples_[MAX_SCRATCH] = {};
    Sample scratch_turbo_[MAX_SCRATCH] = {};

    // The audio processing callback
    void process_audio(Sample* output, FrameCount frames);

    // Static callback for miniaudio
    static void audio_callback(void* device, void* output,
                                const void* input, unsigned int frame_count);
};

} // namespace engin
