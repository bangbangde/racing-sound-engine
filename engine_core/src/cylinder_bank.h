#pragma once

#include "engin/types.h"
#include "engin/constants.h"
#include "engin/engine_preset.h"

namespace engin {

class CylinderBank {
public:
    CylinderBank();

    void configure(const EnginePreset& preset);

    // Generate raw cylinder impulse signal into output buffer (additive).
    // firing_freq: the overall firing rate in Hz
    // throttle: 0-1, affects impulse sharpness
    void process(Sample* output, FrameCount frames, float firing_freq,
                 float throttle, SampleRate sample_rate);

    void reset();

private:
    int   num_cylinders_ = 4;
    float phase_offsets_[MAX_CYLINDERS] = {};
    float crank_phase_ = 0.0f;

    // Per-cylinder impulse state
    struct CylinderState {
        float phase            = 0.0f;
        float last_phase       = 0.0f;
        float impulse_time     = 1.0f; // time since last fire (normalized, >1 = inactive)
        float amplitude_jitter = 1.0f; // random amplitude factor per firing
    };
    CylinderState cylinders_[MAX_CYLINDERS] = {};

    float impulse_duration_ = 0.008f; // seconds
    float impulse_decay_    = 300.0f;

    // Asymmetric combustion waveform parameters
    float attack_sharpness_ = 20.0f;   // controls how fast the attack phase rises
    float ring_frequency_   = 400.0f;  // decay oscillation frequency (Hz)
    float ring_amount_      = 0.3f;    // decay oscillation mix amount

    // Combustion jitter (cycle-to-cycle variation)
    float jitter_amount_    = 0.12f;   // amplitude jitter range (+/-)
    float timing_jitter_    = 0.0004f; // max timing offset in seconds

    // xorshift128 RNG state
    uint32_t rng_state_[4] = {123456789, 362436069, 521288629, 88675123};
    float xorshift128();
};

} // namespace engin
