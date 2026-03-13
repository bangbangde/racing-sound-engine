#pragma once

#include "engin/types.h"
#include "engin/constants.h"
#include "engin/engine_preset.h"
#include "engin/engine_params.h"
#include "parameter_controller.h"
#include "cylinder_bank.h"
#include "harmonic_synth.h"
#include "noise_generator.h"
#include "sample_player.h"
#include "turbo_whistle.h"
#include "biquad_filter.h"
#include "distortion.h"
#include "reverb.h"
#include "effects_chain.h"
#include "audio_mixer.h"

#include <atomic>

namespace engin {

class SynthEngine {
public:
    SynthEngine();
    ~SynthEngine();

    void init(SampleRate sample_rate);

    void load_preset(const EnginePreset& preset);
    void set_params(const EngineParams& params);

    // Load a sample from raw PCM float data. Returns sample_id.
    int load_sample(const float* pcm_data, uint32_t num_frames,
                    SampleRate original_rate, bool loop);

    // Render audio into output buffer (call from audio callback or offline)
    void process(Sample* output, FrameCount frames);

    void reset();

    ParameterController& get_param_controller() { return params_; }
    SamplePlayer& get_sample_player() { return sample_player_; }
    SampleRate get_sample_rate() const { return sample_rate_; }

private:
    SampleRate sample_rate_ = DEFAULT_SAMPLE_RATE;

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
    BiquadFilter          exhaust_lowpass_;
    BiquadFilter          intake_filter_;
    Distortion            distortion_;
    SimpleReverb          reverb_;
    AudioMixer            mixer_;

    // Current preset (stored atomically for audio thread)
    std::atomic<const EnginePreset*> current_preset_{nullptr};
    EnginePreset cached_preset_;

    // Scratch buffers
    static constexpr FrameCount MAX_SCRATCH = 4096;
    Sample scratch_cylinders_[MAX_SCRATCH] = {};
    Sample scratch_harmonics_[MAX_SCRATCH] = {};
    Sample scratch_intake_noise_[MAX_SCRATCH] = {};
    Sample scratch_exhaust_noise_[MAX_SCRATCH] = {};
    Sample scratch_samples_[MAX_SCRATCH] = {};
    Sample scratch_turbo_[MAX_SCRATCH] = {};
};

} // namespace engin
