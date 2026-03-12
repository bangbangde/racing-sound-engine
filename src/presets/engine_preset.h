#pragma once

#include "core/types.h"
#include "core/constants.h"

namespace engin {

struct EnginePreset {
    const char* name = "Default";

    // Basic params
    int   num_cylinders  = 4;
    int   num_strokes    = 4;
    float idle_rpm       = 800.0f;
    float redline_rpm    = 8000.0f;
    float inertia        = 0.15f;

    // Firing pattern: phase offsets in radians for each cylinder
    // These determine the timing of each cylinder's power stroke within one crank revolution
    float phase_offsets[MAX_CYLINDERS] = {};
    int   firing_order[MAX_CYLINDERS]  = {};

    // Harmonic profile: amplitudes for each harmonic (1st, 2nd, 3rd, ...)
    float harmonic_amplitudes[MAX_HARMONICS] = {};
    int   num_harmonics = 0;

    // Filter params
    float exhaust_resonance_freq = 200.0f;
    float exhaust_resonance_Q    = 1.5f;
    float intake_resonance_freq  = 400.0f;
    float intake_resonance_Q     = 2.0f;

    // Effect params
    float distortion_drive = 2.0f;
    float distortion_mix   = 0.4f;
    float reverb_room_size = 0.3f;
    float reverb_damping   = 0.5f;
    float reverb_mix       = 0.15f;

    // Noise params
    float intake_noise_level      = 0.05f;
    float intake_noise_center_freq = 3000.0f;
    float intake_noise_bandwidth   = 2000.0f;
    float exhaust_noise_level      = 0.08f;
    float exhaust_noise_center_freq = 1500.0f;
    float exhaust_noise_bandwidth   = 1500.0f;
};

// Preset factory functions
const EnginePreset& get_preset_i4();
const EnginePreset& get_preset_v8_cross();
const EnginePreset& get_preset_v8_flat();

} // namespace engin
