#pragma once

#include "core/types.h"
#include <atomic>
#include <array>

namespace engin {

struct EngineParams {
    float rpm        = 800.0f;
    float throttle   = 0.0f;
    int   gear       = 0;       // 0=neutral, 1-6
    float clutch     = 1.0f;    // 0=disengaged, 1=engaged
    bool  turbo_enabled = false;
    int   preset_id  = 0;
    ShiftPhase shift_phase = ShiftPhase::Idle;
};

// Triple-buffer lock-free parameter bridge.
// Control thread writes, audio thread reads. No locks, no allocations.
class ParameterController {
public:
    ParameterController() {
        buffers_[0] = EngineParams{};
        buffers_[1] = EngineParams{};
        buffers_[2] = EngineParams{};
        write_idx_.store(0, std::memory_order_relaxed);
        ready_idx_.store(1, std::memory_order_relaxed);
        read_idx_.store(2, std::memory_order_relaxed);
        new_data_.store(false, std::memory_order_relaxed);
    }

    // Called from control thread only
    void write(const EngineParams& params) {
        int wi = write_idx_.load(std::memory_order_relaxed);
        buffers_[wi] = params;
        // Swap write and ready: publish written buffer
        int ri = ready_idx_.exchange(wi, std::memory_order_release);
        write_idx_.store(ri, std::memory_order_relaxed);
        new_data_.store(true, std::memory_order_release);
    }

    // Called from audio thread only
    const EngineParams& read() {
        if (new_data_.exchange(false, std::memory_order_acquire)) {
            int rdi = read_idx_.load(std::memory_order_relaxed);
            int ryi = ready_idx_.exchange(rdi, std::memory_order_acq_rel);
            read_idx_.store(ryi, std::memory_order_relaxed);
        }
        return buffers_[read_idx_.load(std::memory_order_relaxed)];
    }

private:
    std::array<EngineParams, 3> buffers_;
    std::atomic<int> write_idx_;
    std::atomic<int> ready_idx_;
    std::atomic<int> read_idx_;
    std::atomic<bool> new_data_;
};

} // namespace engin
