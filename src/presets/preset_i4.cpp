#include "presets/engine_preset.h"

namespace engin {

const EnginePreset& get_preset_i4() {
    static const EnginePreset preset = [] {
        EnginePreset p;
        p.name = "Inline-4";
        p.num_cylinders = 4;
        p.num_strokes   = 4;
        p.idle_rpm      = 850.0f;
        p.redline_rpm   = 7500.0f;
        p.inertia       = 0.12f;

        // Firing order: 1-3-4-2
        // Even firing intervals for I4: 0, PI/2, PI, 3*PI/2
        p.firing_order[0] = 1; p.phase_offsets[0] = 0.0f;
        p.firing_order[1] = 3; p.phase_offsets[1] = PI * 0.5f;
        p.firing_order[2] = 4; p.phase_offsets[2] = PI;
        p.firing_order[3] = 2; p.phase_offsets[3] = PI * 1.5f;

        // Harmonic profile: I4 emphasizes 2nd and 4th harmonics (buzzy character)
        const float harmonics[] = {
            0.30f, 0.50f, 0.20f, 0.40f,  // 1-4: strong 2nd & 4th
            0.15f, 0.10f, 0.08f, 0.12f,  // 5-8
            0.06f, 0.05f, 0.04f, 0.03f,  // 9-12
            0.02f, 0.02f, 0.01f, 0.01f,  // 13-16
        };
        p.num_harmonics = 16;
        for (int i = 0; i < p.num_harmonics; ++i) {
            p.harmonic_amplitudes[i] = harmonics[i];
        }

        // I4 tends to be raspier
        p.exhaust_resonance_freqs[0] = 250.0f;
        p.exhaust_resonance_Qs[0]    = 3.5f;
        p.exhaust_resonance_gains_db[0] = 10.0f;
        p.exhaust_resonance_freqs[1] = 600.0f;
        p.exhaust_resonance_Qs[1]    = 2.5f;
        p.exhaust_resonance_gains_db[1] = 6.0f;
        p.exhaust_resonance_freqs[2] = 1000.0f;
        p.exhaust_resonance_Qs[2]    = 2.0f;
        p.exhaust_resonance_gains_db[2] = 3.0f;
        p.num_exhaust_resonances = 3;
        p.exhaust_lowpass_freq = 2500.0f;   // moderate roll-off, I4 is raspier
        p.exhaust_lowpass_Q    = 0.707f;
        p.intake_resonance_freq  = 500.0f;
        p.intake_resonance_Q     = 2.5f;

        p.distortion_drive = 3.0f;
        p.distortion_mix   = 0.5f;
        p.reverb_room_size = 0.25f;
        p.reverb_damping   = 0.4f;
        p.reverb_mix       = 0.12f;

        p.intake_noise_level       = 0.06f;
        p.intake_noise_center_freq = 3500.0f;
        p.intake_noise_bandwidth   = 2500.0f;
        p.exhaust_noise_level      = 0.10f;
        p.exhaust_noise_center_freq = 1800.0f;
        p.exhaust_noise_bandwidth   = 1500.0f;

        // Combustion waveform: I4 has sharp, buzzy character
        p.impulse_duration          = 0.008f;   // 8ms — standard for I4
        p.impulse_decay             = 300.0f;
        p.combustion_attack_sharpness = 25.0f;
        p.combustion_ring_freq        = 500.0f;
        p.combustion_ring_amount      = 0.35f;

        // Combustion jitter
        p.combustion_jitter = 0.10f;
        p.timing_jitter     = 0.0003f;

        return p;
    }();
    return preset;
}

} // namespace engin
