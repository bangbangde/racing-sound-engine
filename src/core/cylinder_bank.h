#pragma once

#include "core/types.h"
#include "core/constants.h"
#include "presets/engine_preset.h"

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
        float phase       = 0.0f;
        float last_phase   = 0.0f;
        float impulse_time = 1.0f; // time since last fire (normalized, >1 = inactive)
    };
    CylinderState cylinders_[MAX_CYLINDERS] = {};

    float impulse_duration_ = 0.008f; // seconds
    float impulse_decay_    = 300.0f;
};

} // namespace engin
