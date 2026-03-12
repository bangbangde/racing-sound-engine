#include "audio/audio_engine.h"
#include "core/engine_model.h"
#include "presets/engine_preset.h"

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

namespace {

#ifndef _WIN32
// Non-blocking keyboard input for Linux/macOS
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
    // Handle extended keys (arrows, function keys)
    if (ch == 0 || ch == 0xE0) {
        int ext = _getch();
        switch (ext) {
            case 72: return 'W';  // Up arrow → same as W
            case 80: return 'S';  // Down arrow → same as S
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

int main() {
    using namespace engin;

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

    // Initialize audio engine
    AudioEngine audio;
    AudioConfig config;
    config.sample_rate   = DEFAULT_SAMPLE_RATE;
    config.buffer_frames = DEFAULT_BUFFER_FRAMES;

    if (!audio.init(config)) {
        std::fprintf(stderr, "Failed to initialize audio device!\n");
        return 1;
    }

    // Load default preset (V8 Cross-Plane)
    const EnginePreset* presets[3] = {
        &get_preset_i4(),
        &get_preset_v8_cross(),
        &get_preset_v8_flat(),
    };
    int current_preset = 1; // V8 Cross-Plane
    audio.load_preset(*presets[current_preset]);

    // Engine model
    EngineModel engine;
    engine.set_idle_rpm(presets[current_preset]->idle_rpm);
    engine.set_redline_rpm(presets[current_preset]->redline_rpm);
    engine.set_inertia(presets[current_preset]->inertia);

    // Start audio
    audio.start();
    std::printf("Audio started: %u Hz / %u frames\n\n", config.sample_rate, config.buffer_frames);

#ifndef _WIN32
    TerminalRawMode raw_mode;
#endif

    float throttle = 0.0f;
    int   gear     = 0;
    bool  turbo    = false;
    bool  running  = true;

    // Control loop at ~60 Hz
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
                    audio.load_preset(*presets[current_preset]);
                    engine.set_idle_rpm(presets[current_preset]->idle_rpm);
                    engine.set_redline_rpm(presets[current_preset]->redline_rpm);
                    engine.set_inertia(presets[current_preset]->inertia);
                    break;
                case 0x102: // F2
                    current_preset = 1;
                    audio.load_preset(*presets[current_preset]);
                    engine.set_idle_rpm(presets[current_preset]->idle_rpm);
                    engine.set_redline_rpm(presets[current_preset]->redline_rpm);
                    engine.set_inertia(presets[current_preset]->inertia);
                    break;
                case 0x103: // F3
                    current_preset = 2;
                    audio.load_preset(*presets[current_preset]);
                    engine.set_idle_rpm(presets[current_preset]->idle_rpm);
                    engine.set_redline_rpm(presets[current_preset]->redline_rpm);
                    engine.set_inertia(presets[current_preset]->inertia);
                    break;
                case 'q': case 'Q': case 27: // Q or ESC
                    running = false;
                    break;
            }
        }

        // Natural throttle decay when not pressing W
        // (simulates releasing the gas pedal)
        // This is subtle; the user explicitly lowers throttle with S

        // Update engine physics
        engine.set_throttle(throttle);
        engine.shift_gear(gear);
        const float dt = 1.0f / 60.0f;
        engine.update(dt);

        // Push params to audio thread
        EngineParams params;
        params.rpm           = engine.get_rpm();
        params.throttle      = engine.get_throttle();
        params.gear          = engine.get_gear();
        params.clutch        = engine.get_clutch();
        params.turbo_enabled = turbo;
        params.preset_id     = current_preset;
        params.shift_phase   = engine.get_shift_phase();
        audio.set_params(params);

        // Display status
        clear_line();
        const char* shift_indicator = "";
        switch (engine.get_shift_phase()) {
            case engin::ShiftPhase::ClutchDisengage: shift_indicator = " [CLUTCH]"; break;
            case engin::ShiftPhase::GearChange:      shift_indicator = " [SHIFT]";  break;
            case engin::ShiftPhase::ClutchEngage:    shift_indicator = " [ENGAGE]"; break;
            default: break;
        }
        std::printf("\r RPM: %5.0f | Gear: %s%s | Throttle: %3.0f%% | Load: %.2f | Turbo: %s | Preset: %s  ",
                    engine.get_rpm(),
                    gear == 0 ? "N" : std::to_string(gear).c_str(),
                    shift_indicator,
                    engine.get_throttle() * 100.0f,
                    engine.get_load(),
                    turbo ? "ON" : "OFF",
                    presets[current_preset]->name);
        std::fflush(stdout);

        // Sleep to maintain ~60Hz
        auto elapsed = std::chrono::steady_clock::now() - tick_start;
        auto sleep_time = tick_duration - elapsed;
        if (sleep_time.count() > 0) {
            std::this_thread::sleep_for(sleep_time);
        }
    }

    std::printf("\n\nShutting down...\n");
    audio.shutdown();
    return 0;
}
