#include "synth_engine.h"
#include "engine_model.h"
#include "engin/audio_playback.h"
#include "engin/engine_preset.h"
#include "engin/engine_params.h"
#include "engin/constants.h"

#include <cstdio>
#include <cstdlib>
#include <string>
#include <chrono>
#include <thread>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>
#endif

using namespace engin;

// ---- Inline preset constructors (replaces removed C++ preset factory files) ----

static EnginePreset make_preset_i4() {
    EnginePreset p;
    p.name = "Inline-4";
    p.num_cylinders = 4;
    p.num_strokes   = 4;
    p.idle_rpm      = 850.0f;
    p.redline_rpm   = 7500.0f;
    p.inertia       = 0.12f;

    p.firing_order[0] = 1; p.phase_offsets[0] = 0.0f;
    p.firing_order[1] = 3; p.phase_offsets[1] = PI * 0.5f;
    p.firing_order[2] = 4; p.phase_offsets[2] = PI;
    p.firing_order[3] = 2; p.phase_offsets[3] = PI * 1.5f;

    const float harmonics[] = {
        0.30f, 0.50f, 0.20f, 0.40f,
        0.15f, 0.10f, 0.08f, 0.12f,
        0.06f, 0.05f, 0.04f, 0.03f,
        0.02f, 0.02f, 0.01f, 0.01f,
    };
    p.num_harmonics = 16;
    for (int i = 0; i < p.num_harmonics; ++i) p.harmonic_amplitudes[i] = harmonics[i];

    p.exhaust_resonance_freqs[0] = 250.0f; p.exhaust_resonance_Qs[0] = 3.5f; p.exhaust_resonance_gains_db[0] = 10.0f;
    p.exhaust_resonance_freqs[1] = 600.0f; p.exhaust_resonance_Qs[1] = 2.5f; p.exhaust_resonance_gains_db[1] = 6.0f;
    p.exhaust_resonance_freqs[2] = 1000.0f; p.exhaust_resonance_Qs[2] = 2.0f; p.exhaust_resonance_gains_db[2] = 3.0f;
    p.num_exhaust_resonances = 3;
    p.exhaust_lowpass_freq = 2500.0f; p.exhaust_lowpass_Q = 0.707f;
    p.intake_resonance_freq = 500.0f; p.intake_resonance_Q = 2.5f;

    p.distortion_drive = 3.0f; p.distortion_mix = 0.5f;
    p.reverb_room_size = 0.25f; p.reverb_damping = 0.4f; p.reverb_mix = 0.12f;

    p.intake_noise_level = 0.06f; p.intake_noise_center_freq = 3500.0f; p.intake_noise_bandwidth = 2500.0f;
    p.exhaust_noise_level = 0.10f; p.exhaust_noise_center_freq = 1800.0f; p.exhaust_noise_bandwidth = 1500.0f;

    p.impulse_duration = 0.008f; p.impulse_decay = 300.0f;
    p.combustion_attack_sharpness = 25.0f; p.combustion_ring_freq = 500.0f; p.combustion_ring_amount = 0.35f;
    p.combustion_jitter = 0.10f; p.timing_jitter = 0.0003f;

    return p;
}

static EnginePreset make_preset_v8_cross() {
    EnginePreset p;
    p.name = "V8-CrossPlane";
    p.num_cylinders = 8;
    p.num_strokes   = 4;
    p.idle_rpm      = 700.0f;
    p.redline_rpm   = 6500.0f;
    p.inertia       = 0.20f;

    p.firing_order[0] = 1; p.phase_offsets[0] = 0.0f;
    p.firing_order[1] = 8; p.phase_offsets[1] = PI * 0.25f;
    p.firing_order[2] = 4; p.phase_offsets[2] = PI * 0.75f;
    p.firing_order[3] = 3; p.phase_offsets[3] = PI * 0.50f;
    p.firing_order[4] = 6; p.phase_offsets[4] = PI * 1.00f;
    p.firing_order[5] = 5; p.phase_offsets[5] = PI * 1.25f;
    p.firing_order[6] = 7; p.phase_offsets[6] = PI * 1.75f;
    p.firing_order[7] = 2; p.phase_offsets[7] = PI * 1.50f;

    const float harmonics[] = {
        0.70f, 0.55f, 0.40f, 0.30f,
        0.18f, 0.12f, 0.08f, 0.06f,
        0.05f, 0.04f, 0.03f, 0.02f,
        0.02f, 0.01f, 0.01f, 0.01f,
        0.01f, 0.01f, 0.00f, 0.00f,
    };
    p.num_harmonics = 20;
    for (int i = 0; i < p.num_harmonics; ++i) p.harmonic_amplitudes[i] = harmonics[i];

    p.exhaust_resonance_freqs[0] = 120.0f; p.exhaust_resonance_Qs[0] = 2.0f; p.exhaust_resonance_gains_db[0] = 15.0f;
    p.exhaust_resonance_freqs[1] = 300.0f; p.exhaust_resonance_Qs[1] = 2.0f; p.exhaust_resonance_gains_db[1] = 8.0f;
    p.exhaust_resonance_freqs[2] = 600.0f; p.exhaust_resonance_Qs[2] = 1.5f; p.exhaust_resonance_gains_db[2] = 3.0f;
    p.num_exhaust_resonances = 3;
    p.exhaust_lowpass_freq = 1200.0f; p.exhaust_lowpass_Q = 0.707f;
    p.intake_resonance_freq = 350.0f; p.intake_resonance_Q = 1.8f;

    p.distortion_drive = 1.8f; p.distortion_mix = 0.35f;
    p.reverb_room_size = 0.40f; p.reverb_damping = 0.55f; p.reverb_mix = 0.18f;

    p.intake_noise_level = 0.04f; p.intake_noise_center_freq = 2500.0f; p.intake_noise_bandwidth = 2000.0f;
    p.exhaust_noise_level = 0.07f; p.exhaust_noise_center_freq = 1200.0f; p.exhaust_noise_bandwidth = 1200.0f;

    p.impulse_duration = 0.016f; p.impulse_decay = 180.0f;
    p.combustion_attack_sharpness = 15.0f; p.combustion_ring_freq = 100.0f; p.combustion_ring_amount = 0.25f;
    p.combustion_jitter = 0.14f; p.timing_jitter = 0.0005f;

    return p;
}

static EnginePreset make_preset_v8_flat() {
    EnginePreset p;
    p.name = "V8-FlatPlane";
    p.num_cylinders = 8;
    p.num_strokes   = 4;
    p.idle_rpm      = 900.0f;
    p.redline_rpm   = 9000.0f;
    p.inertia       = 0.10f;

    p.firing_order[0] = 1; p.phase_offsets[0] = 0.0f;
    p.firing_order[1] = 5; p.phase_offsets[1] = PI * 0.25f;
    p.firing_order[2] = 3; p.phase_offsets[2] = PI * 0.50f;
    p.firing_order[3] = 7; p.phase_offsets[3] = PI * 0.75f;
    p.firing_order[4] = 2; p.phase_offsets[4] = PI * 1.00f;
    p.firing_order[5] = 6; p.phase_offsets[5] = PI * 1.25f;
    p.firing_order[6] = 4; p.phase_offsets[6] = PI * 1.50f;
    p.firing_order[7] = 8; p.phase_offsets[7] = PI * 1.75f;

    const float harmonics[] = {
        0.25f, 0.30f, 0.35f, 0.40f,
        0.30f, 0.25f, 0.20f, 0.18f,
        0.15f, 0.12f, 0.10f, 0.08f,
        0.06f, 0.05f, 0.04f, 0.04f,
        0.03f, 0.03f, 0.02f, 0.02f,
        0.02f, 0.01f, 0.01f, 0.01f,
    };
    p.num_harmonics = 24;
    for (int i = 0; i < p.num_harmonics; ++i) p.harmonic_amplitudes[i] = harmonics[i];

    p.exhaust_resonance_freqs[0] = 350.0f; p.exhaust_resonance_Qs[0] = 4.0f; p.exhaust_resonance_gains_db[0] = 10.0f;
    p.exhaust_resonance_freqs[1] = 700.0f; p.exhaust_resonance_Qs[1] = 3.0f; p.exhaust_resonance_gains_db[1] = 8.0f;
    p.exhaust_resonance_freqs[2] = 1200.0f; p.exhaust_resonance_Qs[2] = 2.0f; p.exhaust_resonance_gains_db[2] = 4.0f;
    p.num_exhaust_resonances = 3;
    p.exhaust_lowpass_freq = 3500.0f; p.exhaust_lowpass_Q = 0.707f;
    p.intake_resonance_freq = 600.0f; p.intake_resonance_Q = 3.0f;

    p.distortion_drive = 4.0f; p.distortion_mix = 0.55f;
    p.reverb_room_size = 0.20f; p.reverb_damping = 0.35f; p.reverb_mix = 0.10f;

    p.intake_noise_level = 0.07f; p.intake_noise_center_freq = 4000.0f; p.intake_noise_bandwidth = 3000.0f;
    p.exhaust_noise_level = 0.06f; p.exhaust_noise_center_freq = 2000.0f; p.exhaust_noise_bandwidth = 1800.0f;

    p.impulse_duration = 0.006f; p.impulse_decay = 400.0f;
    p.combustion_attack_sharpness = 30.0f; p.combustion_ring_freq = 600.0f; p.combustion_ring_amount = 0.40f;
    p.combustion_jitter = 0.08f; p.timing_jitter = 0.0002f;

    return p;
}

// ---- Platform keyboard helpers ----

namespace {

#ifndef _WIN32
struct TerminalRawMode {
    termios old_settings{};
    TerminalRawMode() {
        tcgetattr(STDIN_FILENO, &old_settings);
        termios raw = old_settings;
        raw.c_lflag &= ~(ICANON | ECHO);
        raw.c_cc[VMIN]  = 0;
        raw.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSANOW, &raw);
    }
    ~TerminalRawMode() {
        tcsetattr(STDIN_FILENO, TCSANOW, &old_settings);
    }
};

bool kbhit_unix() {
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    timeval tv = {0, 0};
    return select(STDIN_FILENO + 1, &fds, nullptr, nullptr, &tv) > 0;
}

int getch_unix() {
    int ch;
    if (read(STDIN_FILENO, &ch, 1) == 1) return ch;
    return -1;
}
#endif

bool key_available() {
#ifdef _WIN32
    return _kbhit() != 0;
#else
    return kbhit_unix();
#endif
}

int read_key() {
#ifdef _WIN32
    int ch = _getch();
    if (ch == 0 || ch == 0xE0) {
        int ext = _getch();
        switch (ext) {
            case 72: return 'W';  // Up arrow
            case 80: return 'S';  // Down arrow
            case 59: return 0x101; // F1
            case 60: return 0x102; // F2
            case 61: return 0x103; // F3
        }
        return 0;
    }
    return ch;
#else
    return getch_unix();
#endif
}

void clear_line() {
    std::printf("\r\033[K");
}

} // anonymous namespace

// ---- Main ----

int main() {
    std::printf("=== Racing Engine Sound Simulator ===\n\n");
    std::printf("Controls:\n");
    std::printf("  W / Up Arrow    : Throttle up\n");
    std::printf("  S / Down Arrow  : Throttle down\n");
    std::printf("  1-6             : Shift gear\n");
    std::printf("  N               : Neutral\n");
    std::printf("  T               : Toggle turbo\n");
    std::printf("  F1              : Preset: Inline-4\n");
    std::printf("  F2              : Preset: V8 Cross-Plane\n");
    std::printf("  F3              : Preset: V8 Flat-Plane\n");
    std::printf("  Q / ESC         : Quit\n\n");

    // Build presets
    EnginePreset presets[3] = {
        make_preset_i4(),
        make_preset_v8_cross(),
        make_preset_v8_flat(),
    };
    int current_preset = 1; // V8 Cross-Plane default

    // Initialize synth engine
    SampleRate sample_rate   = DEFAULT_SAMPLE_RATE;
    FrameCount buffer_frames = DEFAULT_BUFFER_FRAMES;

    SynthEngine synth;
    synth.init(sample_rate);
    synth.load_preset(presets[current_preset]);

    // Initialize audio playback
    AudioPlayback playback;
    if (!playback.init(synth, sample_rate, buffer_frames)) {
        std::fprintf(stderr, "Failed to initialize audio device!\n");
        return 1;
    }

    // Engine model (physics)
    EngineModel engine;
    engine.set_idle_rpm(presets[current_preset].idle_rpm);
    engine.set_redline_rpm(presets[current_preset].redline_rpm);
    engine.set_inertia(presets[current_preset].inertia);

    // Start audio
    playback.start();
    std::printf("Audio started: %u Hz / %u frames\n\n", sample_rate, buffer_frames);

#ifndef _WIN32
    TerminalRawMode raw_mode;
#endif

    float throttle = 0.0f;
    int   gear     = 0;
    bool  turbo    = false;
    bool  running  = true;

    const auto tick_duration = std::chrono::microseconds(16667); // ~60 Hz

    while (running) {
        auto tick_start = std::chrono::steady_clock::now();

        // Process input
        while (key_available()) {
            int ch = read_key();
            switch (ch) {
                case 'w': case 'W':
                    throttle = std::min(1.0f, throttle + 0.08f);
                    break;
                case 's': case 'S':
                    throttle = std::max(0.0f, throttle - 0.08f);
                    break;
                case '1': case '2': case '3':
                case '4': case '5': case '6':
                    gear = ch - '0';
                    break;
                case 'n': case 'N':
                    gear = 0;
                    break;
                case 't': case 'T':
                    turbo = !turbo;
                    break;
                case 0x101: // F1
                    current_preset = 0;
                    synth.load_preset(presets[current_preset]);
                    engine.set_idle_rpm(presets[current_preset].idle_rpm);
                    engine.set_redline_rpm(presets[current_preset].redline_rpm);
                    engine.set_inertia(presets[current_preset].inertia);
                    break;
                case 0x102: // F2
                    current_preset = 1;
                    synth.load_preset(presets[current_preset]);
                    engine.set_idle_rpm(presets[current_preset].idle_rpm);
                    engine.set_redline_rpm(presets[current_preset].redline_rpm);
                    engine.set_inertia(presets[current_preset].inertia);
                    break;
                case 0x103: // F3
                    current_preset = 2;
                    synth.load_preset(presets[current_preset]);
                    engine.set_idle_rpm(presets[current_preset].idle_rpm);
                    engine.set_redline_rpm(presets[current_preset].redline_rpm);
                    engine.set_inertia(presets[current_preset].inertia);
                    break;
                case 'q': case 'Q': case 27: // Q or ESC
                    running = false;
                    break;
            }
        }

        // Update engine physics
        engine.set_throttle(throttle);
        engine.shift_gear(gear);
        const float dt = 1.0f / 60.0f;
        engine.update(dt);

        // Push params to synth engine
        EngineParams params;
        params.rpm           = engine.get_rpm();
        params.throttle      = engine.get_throttle();
        params.gear          = engine.get_gear();
        params.clutch        = engine.get_clutch();
        params.turbo_enabled = turbo ? 1 : 0;
        params.preset_id     = current_preset;
        params.shift_phase   = engine.get_shift_phase();
        synth.set_params(params);

        // Display status
        clear_line();
        const char* shift_indicator = "";
        switch (engine.get_shift_phase()) {
            case ShiftPhase_ClutchDisengage: shift_indicator = " [CLUTCH]"; break;
            case ShiftPhase_GearChange:      shift_indicator = " [SHIFT]";  break;
            case ShiftPhase_ClutchEngage:    shift_indicator = " [ENGAGE]"; break;
            default: break;
        }
        std::printf("\r RPM: %5.0f | Gear: %s%s | Throttle: %3.0f%% | Load: %.2f | Turbo: %s | Preset: %s  ",
                    engine.get_rpm(),
                    gear == 0 ? "N" : std::to_string(gear).c_str(),
                    shift_indicator,
                    engine.get_throttle() * 100.0f,
                    engine.get_load(),
                    turbo ? "ON" : "OFF",
                    presets[current_preset].name);
        std::fflush(stdout);

        // Sleep to maintain ~60Hz
        auto elapsed = std::chrono::steady_clock::now() - tick_start;
        auto sleep_time = tick_duration - elapsed;
        if (sleep_time.count() > 0) {
            std::this_thread::sleep_for(sleep_time);
        }
    }

    std::printf("\n\nShutting down...\n");
    playback.shutdown();
    return 0;
}
