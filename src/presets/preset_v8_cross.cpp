#include "presets/engine_preset.h"

namespace engin {

const EnginePreset& get_preset_v8_cross() {
    static const EnginePreset preset = [] {
        EnginePreset p;
        p.name = "V8-CrossPlane";
        p.num_cylinders = 8;
        p.num_strokes   = 4;
        p.idle_rpm      = 700.0f;
        p.redline_rpm   = 6500.0f;
        p.inertia       = 0.20f;

        // Cross-plane V8 firing order: 1-8-4-3-6-5-7-2
        // Uneven firing intervals create the classic V8 burble
        // Cross-plane crankshaft: pins at 0, 90, 270, 180 degrees
        // This creates firing intervals of: 0, 90, 180, 270, 360, 450, 540, 630 (uneven per bank)
        p.firing_order[0] = 1; p.phase_offsets[0] = 0.0f;
        p.firing_order[1] = 8; p.phase_offsets[1] = PI * 0.25f;
        p.firing_order[2] = 4; p.phase_offsets[2] = PI * 0.75f;
        p.firing_order[3] = 3; p.phase_offsets[3] = PI * 0.50f;
        p.firing_order[4] = 6; p.phase_offsets[4] = PI * 1.00f;
        p.firing_order[5] = 5; p.phase_offsets[5] = PI * 1.25f;
        p.firing_order[6] = 7; p.phase_offsets[6] = PI * 1.75f;
        p.firing_order[7] = 2; p.phase_offsets[7] = PI * 1.50f;

        // V8 cross-plane: very strong low harmonics for deep bass
        const float harmonics[] = {
            0.70f, 0.55f, 0.40f, 0.30f,  // 1-4: dominant bass fundamentals
            0.18f, 0.12f, 0.08f, 0.06f,  // 5-8
            0.05f, 0.04f, 0.03f, 0.02f,  // 9-12
            0.02f, 0.01f, 0.01f, 0.01f,  // 13-16
            0.01f, 0.01f, 0.00f, 0.00f,  // 17-20
        };
        p.num_harmonics = 20;
        for (int i = 0; i < p.num_harmonics; ++i) {
            p.harmonic_amplitudes[i] = harmonics[i];
        }

        // Deep, boomy exhaust — resonant filter bank with strong bass peak
        p.exhaust_resonance_freqs[0] = 120.0f;
        p.exhaust_resonance_Qs[0]    = 2.0f;
        p.exhaust_resonance_gains_db[0] = 15.0f;
        p.exhaust_resonance_freqs[1] = 300.0f;
        p.exhaust_resonance_Qs[1]    = 2.0f;
        p.exhaust_resonance_gains_db[1] = 8.0f;
        p.exhaust_resonance_freqs[2] = 600.0f;
        p.exhaust_resonance_Qs[2]    = 1.5f;
        p.exhaust_resonance_gains_db[2] = 3.0f;
        p.num_exhaust_resonances = 3;
        p.exhaust_lowpass_freq = 1200.0f;   // aggressive high-cut for deep V8 character
        p.exhaust_lowpass_Q    = 0.707f;
        p.intake_resonance_freq  = 350.0f;
        p.intake_resonance_Q     = 1.8f;

        p.distortion_drive = 1.8f;
        p.distortion_mix   = 0.35f;
        p.reverb_room_size = 0.40f;
        p.reverb_damping   = 0.55f;
        p.reverb_mix       = 0.18f;

        p.intake_noise_level       = 0.04f;
        p.intake_noise_center_freq = 2500.0f;
        p.intake_noise_bandwidth   = 2000.0f;
        p.exhaust_noise_level      = 0.07f;
        p.exhaust_noise_center_freq = 1200.0f;
        p.exhaust_noise_bandwidth   = 1200.0f;

        // Combustion waveform: V8 cross — longer impulse for bass, low ring for deep rumble
        p.impulse_duration          = 0.016f;   // 16ms — much longer for bass energy
        p.impulse_decay             = 180.0f;   // slower decay preserves low freq content
        p.combustion_attack_sharpness = 15.0f;  // slightly softer attack for warmth
        p.combustion_ring_freq        = 100.0f;  // low ring keeps energy in bass range
        p.combustion_ring_amount      = 0.25f;   // subtle oscillation

        // Combustion jitter — slightly more variation for the classic burble
        p.combustion_jitter = 0.14f;
        p.timing_jitter     = 0.0005f;

        return p;
    }();
    return preset;
}

} // namespace engin
