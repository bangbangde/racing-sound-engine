#pragma once

#include "engin/types.h"
#include "engin/constants.h"

struct EnginePreset {
    const char* name;

    /* Basic params */
    int   num_cylinders;
    int   num_strokes;
    float idle_rpm;
    float redline_rpm;
    float inertia;

    /* Firing pattern: phase offsets in radians for each cylinder */
    float phase_offsets[MAX_CYLINDERS];
    int   firing_order[MAX_CYLINDERS];

    /* Harmonic profile: amplitudes for each harmonic (1st, 2nd, 3rd, ...) */
    float harmonic_amplitudes[MAX_HARMONICS];
    int   num_harmonics;

    /* Filter params -- exhaust resonant filter bank (Peak filters in series) */
    int   num_exhaust_resonances;
    float exhaust_resonance_freqs[MAX_EXHAUST_RESONANCES];
    float exhaust_resonance_Qs[MAX_EXHAUST_RESONANCES];
    float exhaust_resonance_gains_db[MAX_EXHAUST_RESONANCES];
    float exhaust_lowpass_freq;
    float exhaust_lowpass_Q;
    float intake_resonance_freq;
    float intake_resonance_Q;

    /* Effect params */
    float distortion_drive;
    float distortion_mix;
    float reverb_room_size;
    float reverb_damping;
    float reverb_mix;

    /* Noise params */
    float intake_noise_level;
    float intake_noise_center_freq;
    float intake_noise_bandwidth;
    float exhaust_noise_level;
    float exhaust_noise_center_freq;
    float exhaust_noise_bandwidth;

    /* Combustion waveform shaping */
    float impulse_duration;
    float impulse_decay;
    float combustion_attack_sharpness;
    float combustion_ring_freq;
    float combustion_ring_amount;

    /* Combustion cycle-to-cycle variation */
    float combustion_jitter;
    float timing_jitter;

    /* Turbo whistle params */
    float turbo_base_freq;
    float turbo_freq_range;
    float turbo_noise_mix;
    float turbo_max_amplitude;

#ifdef __cplusplus
    EnginePreset()
        : name("Default"),
          num_cylinders(4), num_strokes(4),
          idle_rpm(800.0f), redline_rpm(8000.0f), inertia(0.15f),
          phase_offsets{}, firing_order{},
          harmonic_amplitudes{}, num_harmonics(0),
          num_exhaust_resonances(1),
          exhaust_resonance_freqs{200.0f, 0.0f, 0.0f},
          exhaust_resonance_Qs{1.5f, 0.0f, 0.0f},
          exhaust_resonance_gains_db{12.0f, 0.0f, 0.0f},
          exhaust_lowpass_freq(2500.0f), exhaust_lowpass_Q(0.707f),
          intake_resonance_freq(400.0f), intake_resonance_Q(2.0f),
          distortion_drive(2.0f), distortion_mix(0.4f),
          reverb_room_size(0.3f), reverb_damping(0.5f), reverb_mix(0.15f),
          intake_noise_level(0.05f), intake_noise_center_freq(3000.0f),
          intake_noise_bandwidth(2000.0f),
          exhaust_noise_level(0.08f), exhaust_noise_center_freq(1500.0f),
          exhaust_noise_bandwidth(1500.0f),
          impulse_duration(0.008f), impulse_decay(300.0f),
          combustion_attack_sharpness(20.0f),
          combustion_ring_freq(400.0f), combustion_ring_amount(0.3f),
          combustion_jitter(0.12f), timing_jitter(0.0004f),
          turbo_base_freq(2000.0f), turbo_freq_range(5000.0f),
          turbo_noise_mix(0.3f), turbo_max_amplitude(0.08f) {}
#endif
};
