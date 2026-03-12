#include "presets/engine_preset.h"

namespace engin {

const EnginePreset& get_preset_v8_flat() {
    static const EnginePreset preset = [] {
        EnginePreset p;
        p.name = "V8-FlatPlane";
        p.num_cylinders = 8;
        p.num_strokes   = 4;
        p.idle_rpm      = 900.0f;
        p.redline_rpm   = 9000.0f;
        p.inertia       = 0.10f;

        // Flat-plane V8: even firing intervals (like two I4s)
        // Produces the screaming high-RPM F1-like sound
        p.firing_order[0] = 1; p.phase_offsets[0] = 0.0f;
        p.firing_order[1] = 5; p.phase_offsets[1] = PI * 0.25f;
        p.firing_order[2] = 3; p.phase_offsets[2] = PI * 0.50f;
        p.firing_order[3] = 7; p.phase_offsets[3] = PI * 0.75f;
        p.firing_order[4] = 2; p.phase_offsets[4] = PI * 1.00f;
        p.firing_order[5] = 6; p.phase_offsets[5] = PI * 1.25f;
        p.firing_order[6] = 4; p.phase_offsets[6] = PI * 1.50f;
        p.firing_order[7] = 8; p.phase_offsets[7] = PI * 1.75f;

        // Flat-plane: emphasis on higher harmonics, screamy character
        const float harmonics[] = {
            0.25f, 0.30f, 0.35f, 0.40f,  // 1-4: higher harmonics stronger
            0.30f, 0.25f, 0.20f, 0.18f,  // 5-8
            0.15f, 0.12f, 0.10f, 0.08f,  // 9-12
            0.06f, 0.05f, 0.04f, 0.04f,  // 13-16
            0.03f, 0.03f, 0.02f, 0.02f,  // 17-20
            0.02f, 0.01f, 0.01f, 0.01f,  // 21-24
        };
        p.num_harmonics = 24;
        for (int i = 0; i < p.num_harmonics; ++i) {
            p.harmonic_amplitudes[i] = harmonics[i];
        }

        // Sharper, higher resonance
        p.exhaust_resonance_freq = 350.0f;
        p.exhaust_resonance_Q    = 2.0f;
        p.intake_resonance_freq  = 600.0f;
        p.intake_resonance_Q     = 3.0f;

        p.distortion_drive = 4.0f;
        p.distortion_mix   = 0.55f;
        p.reverb_room_size = 0.20f;
        p.reverb_damping   = 0.35f;
        p.reverb_mix       = 0.10f;

        p.intake_noise_level       = 0.07f;
        p.intake_noise_center_freq = 4000.0f;
        p.intake_noise_bandwidth   = 3000.0f;
        p.exhaust_noise_level      = 0.06f;
        p.exhaust_noise_center_freq = 2000.0f;
        p.exhaust_noise_bandwidth   = 1800.0f;

        return p;
    }();
    return preset;
}

} // namespace engin
