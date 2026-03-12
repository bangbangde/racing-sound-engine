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

    // Filter params — exhaust resonant filter bank (Peak filters in series)
    int   num_exhaust_resonances = 1;
    float exhaust_resonance_freqs[MAX_EXHAUST_RESONANCES]    = {200.0f, 0.0f, 0.0f};
    float exhaust_resonance_Qs[MAX_EXHAUST_RESONANCES]       = {1.5f, 0.0f, 0.0f};
    float exhaust_resonance_gains_db[MAX_EXHAUST_RESONANCES] = {12.0f, 0.0f, 0.0f};
    float exhaust_lowpass_freq = 2500.0f;  // high-freq roll-off after resonances (Hz)
    float exhaust_lowpass_Q    = 0.707f;   // Butterworth Q for smooth roll-off
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

    // Combustion waveform shaping
    float impulse_duration          = 0.008f;   // base impulse duration (seconds)
    float impulse_decay             = 300.0f;    // base impulse decay rate
    float combustion_attack_sharpness = 20.0f;   // attack speed factor
    float combustion_ring_freq        = 400.0f;  // decay oscillation Hz
    float combustion_ring_amount      = 0.3f;    // decay oscillation mix

    // Combustion cycle-to-cycle variation
    float combustion_jitter = 0.12f;   // amplitude jitter range (±)
    float timing_jitter     = 0.0004f; // max timing offset (seconds)

    // Turbo whistle params
    float turbo_base_freq     = 2000.0f;
    float turbo_freq_range    = 5000.0f;
    float turbo_noise_mix     = 0.3f;
    float turbo_max_amplitude = 0.08f;
};

// Preset factory functions
const EnginePreset& get_preset_i4();
const EnginePreset& get_preset_v8_cross();
const EnginePreset& get_preset_v8_flat();

} // namespace engin
