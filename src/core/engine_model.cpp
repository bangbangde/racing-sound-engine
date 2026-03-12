#include "core/engine_model.h"
#include <algorithm>
#include <cmath>

namespace engin {

EngineModel::EngineModel() = default;

void EngineModel::set_throttle(float value) {
    throttle_ = std::clamp(value, 0.0f, 1.0f);
}

void EngineModel::shift_gear(int new_gear) {
    new_gear = std::clamp(new_gear, 0, 6);

    // Ignore if already in this gear, or a shift is in progress
    if (new_gear == gear_ && shift_phase_ == ShiftPhase::Idle) return;
    if (shift_phase_ != ShiftPhase::Idle) return;

    // Shifting to/from neutral is instant (no clutch sequence needed)
    if (gear_ == 0 || new_gear == 0) {
        gear_ = new_gear;
        return;
    }

    // Start the shift state machine
    shift_phase_        = ShiftPhase::ClutchDisengage;
    shift_timer_        = 0.0f;
    pending_gear_       = new_gear;
    shift_is_downshift_ = (new_gear < gear_);
    blip_amount_        = 0.0f;
}

void EngineModel::set_clutch(float value) {
    // Only allow manual clutch override when not mid-shift
    if (shift_phase_ == ShiftPhase::Idle) {
        clutch_ = std::clamp(value, 0.0f, 1.0f);
    }
}

void EngineModel::update_shift(float dt) {
    shift_timer_ += dt;

    switch (shift_phase_) {

    case ShiftPhase::ClutchDisengage: {
        // Smoothly release clutch: 1.0 → 0.0 over kClutchDisengageTime
        float progress = std::min(shift_timer_ / kClutchDisengageTime, 1.0f);
        clutch_ = 1.0f - progress;

        if (shift_timer_ >= kClutchDisengageTime) {
            // Clutch fully disengaged — perform the gear change
            clutch_ = 0.0f;

            // Compute target RPM from gear ratio conversion
            shift_start_rpm_ = current_rpm_;
            float ratio = gear_ratios_[pending_gear_] / gear_ratios_[gear_];
            shift_target_rpm_ = std::clamp(current_rpm_ * ratio, idle_rpm_, redline_rpm_);

            // Actually change gear
            gear_ = pending_gear_;

            // For downshift, initiate rev-match blip
            if (shift_is_downshift_) {
                // Blip strength proportional to the RPM gap
                float rpm_gap = shift_target_rpm_ - shift_start_rpm_;
                blip_amount_ = std::clamp(rpm_gap / (redline_rpm_ - idle_rpm_), 0.0f, 0.6f);
            }

            shift_timer_ = 0.0f;
            shift_phase_ = ShiftPhase::GearChange;
        }
        break;
    }

    case ShiftPhase::GearChange: {
        // Interpolate RPM from start to target over kGearChangeTime
        float progress = std::min(shift_timer_ / kGearChangeTime, 1.0f);
        // Use smoothstep for natural feel: 3t^2 - 2t^3
        float smooth = progress * progress * (3.0f - 2.0f * progress);
        current_rpm_ = shift_start_rpm_ + (shift_target_rpm_ - shift_start_rpm_) * smooth;
        current_rpm_ = std::clamp(current_rpm_, idle_rpm_, redline_rpm_);

        // Decay the blip over this phase
        if (shift_is_downshift_ && blip_amount_ > 0.0f) {
            // Blip peaks at start of gear change, decays to zero
            float blip_progress = std::min(shift_timer_ / kBlipDecayTime, 1.0f);
            float blip_envelope = 1.0f - blip_progress * blip_progress; // quadratic decay
            // Blip adds a temporary RPM boost on top of interpolation
            float blip_rpm = blip_amount_ * (redline_rpm_ - idle_rpm_) * 0.3f * blip_envelope;
            current_rpm_ = std::clamp(current_rpm_ + blip_rpm, idle_rpm_, redline_rpm_);
        }

        if (shift_timer_ >= kGearChangeTime) {
            current_rpm_ = shift_target_rpm_;
            blip_amount_ = 0.0f;
            shift_timer_ = 0.0f;
            shift_phase_ = ShiftPhase::ClutchEngage;
        }
        break;
    }

    case ShiftPhase::ClutchEngage: {
        // Smoothly engage clutch: 0.0 → 1.0 over kClutchEngageTime
        float progress = std::min(shift_timer_ / kClutchEngageTime, 1.0f);
        // Slightly slower initial engagement (ease-in) for realism
        float ease = progress * progress * (3.0f - 2.0f * progress);
        clutch_ = ease;

        if (shift_timer_ >= kClutchEngageTime) {
            clutch_ = 1.0f;
            shift_phase_ = ShiftPhase::Idle;
            shift_timer_ = 0.0f;
        }
        break;
    }

    case ShiftPhase::Idle:
        break;
    }
}

void EngineModel::update(float dt) {
    // Advance shift state machine if active
    if (shift_phase_ != ShiftPhase::Idle) {
        update_shift(dt);
    }

    // Compute load from gear ratio — higher gear = more load at same RPM
    float gear_ratio = gear_ratios_[gear_];
    if (gear_ == 0 || clutch_ < 0.1f) {
        // Neutral or clutch disengaged: no drivetrain load
        load_ = 0.05f; // minimal accessories load
    } else {
        // Load proportional to gear ratio and RPM fraction
        float rpm_frac = current_rpm_ / redline_rpm_;
        load_ = 0.1f + 0.6f * gear_ratio / 3.5f * rpm_frac * clutch_;
    }

    // Torque model: throttle drives, friction and load resist
    float effective_throttle = throttle_;
    // During GearChange phase on downshift, add blip to throttle for torque model
    if (shift_phase_ == ShiftPhase::GearChange && shift_is_downshift_ && blip_amount_ > 0.0f) {
        float blip_progress = std::min(shift_timer_ / kBlipDecayTime, 1.0f);
        float blip_envelope = 1.0f - blip_progress * blip_progress;
        effective_throttle = std::min(1.0f, throttle_ + blip_amount_ * blip_envelope);
    }

    float drive_torque = effective_throttle * max_torque_;
    float resist_torque = friction_ + load_ * max_torque_ * 0.5f;

    // RPM derivative — only apply normal torque model when not mid gear-change
    // (during GearChange, RPM is driven by interpolation in update_shift)
    if (shift_phase_ != ShiftPhase::GearChange) {
        float rpm_dot = (drive_torque - resist_torque) / inertia_;
        current_rpm_ += rpm_dot * dt;
    }

    // Clamp RPM
    if (current_rpm_ < idle_rpm_) {
        // Idle governor: pull RPM back up
        current_rpm_ = idle_rpm_ + (current_rpm_ - idle_rpm_) * 0.9f;
        if (current_rpm_ < idle_rpm_) current_rpm_ = idle_rpm_;
    }
    if (current_rpm_ > redline_rpm_) {
        // Rev limiter: hard cut
        current_rpm_ = redline_rpm_;
    }
}

} // namespace engin
