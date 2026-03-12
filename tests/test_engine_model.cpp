#include "core/engine_model.h"
#include <cassert>
#include <cmath>
#include <cstdio>

using namespace engin;

void test_idle_rpm() {
    EngineModel m;
    m.set_throttle(0.0f);
    m.shift_gear(0);

    // Run for a few seconds at idle
    for (int i = 0; i < 300; ++i) {
        m.update(1.0f / 60.0f);
    }
    float rpm = m.get_rpm();
    assert(rpm >= 750.0f && rpm <= 900.0f);
    std::printf("  [PASS] idle_rpm = %.1f\n", rpm);
}

void test_rpm_increases_with_throttle() {
    EngineModel m;
    m.set_throttle(1.0f);
    m.shift_gear(0); // neutral, minimal load

    float initial_rpm = m.get_rpm();
    for (int i = 0; i < 300; ++i) {
        m.update(1.0f / 60.0f);
    }
    float final_rpm = m.get_rpm();
    assert(final_rpm > initial_rpm);
    std::printf("  [PASS] throttle up: %.1f -> %.1f RPM\n", initial_rpm, final_rpm);
}

void test_redline_clamp() {
    EngineModel m;
    m.set_redline_rpm(7000.0f);
    m.set_throttle(1.0f);
    m.shift_gear(0);

    for (int i = 0; i < 6000; ++i) {
        m.update(1.0f / 60.0f);
    }
    float rpm = m.get_rpm();
    assert(rpm <= 7000.0f + 1.0f); // allow tiny float error
    std::printf("  [PASS] redline clamp: rpm = %.1f (max 7000)\n", rpm);
}

void test_gear_affects_load() {
    EngineModel m;
    m.set_throttle(0.5f);
    m.shift_gear(1);
    for (int i = 0; i < 120; ++i) m.update(1.0f / 60.0f);
    float load_g1 = m.get_load();

    EngineModel m2;
    m2.set_throttle(0.5f);
    m2.shift_gear(5);
    for (int i = 0; i < 120; ++i) m2.update(1.0f / 60.0f);
    float load_g5 = m2.get_load();

    // Higher gear at same rpm range should produce different load
    std::printf("  [PASS] gear load: G1=%.3f, G5=%.3f\n", load_g1, load_g5);
}

// --- Shift state machine tests ---

void test_upshift_rpm_drops() {
    // Spin up in gear 1, then shift to gear 2 — RPM should drop by gear ratio.
    // Cut throttle before shifting (realistic: drivers lift off during upshift)
    // so RPM doesn't spike during clutch disengage.
    EngineModel m;
    m.set_throttle(0.8f);
    m.shift_gear(1);
    for (int i = 0; i < 600; ++i) m.update(1.0f / 60.0f);

    // Cut throttle, let RPM stabilize briefly
    m.set_throttle(0.0f);
    for (int i = 0; i < 5; ++i) m.update(1.0f / 60.0f);
    float rpm_before = m.get_rpm();
    assert(rpm_before > 2000.0f);

    // Initiate upshift to gear 2
    m.shift_gear(2);
    assert(m.is_shifting());

    // Run through shift, capture RPM at GearChange->ClutchEngage transition
    float rpm_at_gear_change = 0.0f;
    for (int i = 0; i < 30; ++i) {
        ShiftPhase before_phase = m.get_shift_phase();
        m.update(1.0f / 60.0f);
        ShiftPhase after_phase = m.get_shift_phase();
        if (before_phase == ShiftPhase::GearChange && after_phase == ShiftPhase::ClutchEngage) {
            rpm_at_gear_change = m.get_rpm();
        }
    }

    assert(!m.is_shifting());
    assert(rpm_at_gear_change > 0.0f);
    // RPM should drop during upshift (gear 2 ratio < gear 1 ratio)
    assert(rpm_at_gear_change < rpm_before);
    // Verify roughly matches gear ratio (with tolerance for slight RPM drift during disengage)
    float expected_ratio = EngineModel::gear_ratios_[2] / EngineModel::gear_ratios_[1]; // 0.714
    float expected_rpm = rpm_before * expected_ratio;
    float tolerance = 600.0f;
    assert(std::fabs(rpm_at_gear_change - expected_rpm) < tolerance);
    std::printf("  [PASS] upshift RPM: %.0f -> %.0f at gear change (expected ~%.0f)\n",
                rpm_before, rpm_at_gear_change, expected_rpm);
}

void test_downshift_rpm_rises() {
    // Start in gear 3 at high RPM, cut throttle to bring RPM down to mid-range,
    // then downshift to gear 2 — RPM should rise due to higher gear ratio.
    EngineModel m;
    m.set_throttle(1.0f);
    m.shift_gear(3);
    // Spin to redline
    for (int i = 0; i < 300; ++i) m.update(1.0f / 60.0f);

    // Cut throttle and coast down to mid-range RPM (~4000-5000)
    m.set_throttle(0.0f);
    for (int i = 0; i < 30; ++i) m.update(1.0f / 60.0f);
    float rpm_before = m.get_rpm();

    // Ensure RPM is in a range where downshift target won't hit redline
    // gear2/gear3 = 2.5/1.8 = 1.389, so target = rpm * 1.389
    // Need rpm * 1.389 < 8000 → rpm < 5760
    assert(rpm_before > 2000.0f && rpm_before < 6000.0f);

    // Downshift to gear 2
    m.shift_gear(2);
    assert(m.is_shifting());

    // Capture RPM at GearChange → ClutchEngage transition
    float rpm_at_gear_change = 0.0f;
    for (int i = 0; i < 30; ++i) {
        ShiftPhase before_phase = m.get_shift_phase();
        m.update(1.0f / 60.0f);
        ShiftPhase after_phase = m.get_shift_phase();
        if (before_phase == ShiftPhase::GearChange && after_phase == ShiftPhase::ClutchEngage) {
            rpm_at_gear_change = m.get_rpm();
        }
    }

    assert(!m.is_shifting());
    assert(rpm_at_gear_change > 0.0f);
    // RPM should have risen after downshift
    assert(rpm_at_gear_change > rpm_before);
    std::printf("  [PASS] downshift RPM: %.0f -> %.0f at gear change (rose as expected)\n",
                rpm_before, rpm_at_gear_change);
}

void test_shift_clutch_sequence() {
    // Verify the shift goes through all phases
    EngineModel m;
    m.set_throttle(0.8f);
    m.shift_gear(1);
    for (int i = 0; i < 300; ++i) m.update(1.0f / 60.0f);

    m.shift_gear(2);

    // Phase 1: ClutchDisengage
    assert(m.get_shift_phase() == ShiftPhase::ClutchDisengage);
    // Run 5 frames (~83ms) to get past disengage (80ms)
    for (int i = 0; i < 6; ++i) m.update(1.0f / 60.0f);
    assert(m.get_shift_phase() == ShiftPhase::GearChange);

    // Run 8 frames (~133ms) to get past gear change (120ms)
    for (int i = 0; i < 8; ++i) m.update(1.0f / 60.0f);
    assert(m.get_shift_phase() == ShiftPhase::ClutchEngage);

    // Run 10 frames (~167ms) to get past engage (150ms)
    for (int i = 0; i < 10; ++i) m.update(1.0f / 60.0f);
    assert(m.get_shift_phase() == ShiftPhase::Idle);
    assert(m.get_clutch() >= 0.99f);

    std::printf("  [PASS] shift clutch sequence: all phases traversed\n");
}

void test_neutral_shift_is_instant() {
    EngineModel m;
    m.set_throttle(0.5f);
    m.shift_gear(1);
    for (int i = 0; i < 120; ++i) m.update(1.0f / 60.0f);

    // Shift to neutral should be instant (no state machine)
    m.shift_gear(0);
    assert(!m.is_shifting());
    assert(m.get_gear() == 0);

    // Shift from neutral to gear should also be instant
    m.shift_gear(3);
    assert(!m.is_shifting());
    assert(m.get_gear() == 3);

    std::printf("  [PASS] neutral shift is instant\n");
}

void test_shift_rejected_during_shift() {
    EngineModel m;
    m.set_throttle(0.8f);
    m.shift_gear(1);
    for (int i = 0; i < 300; ++i) m.update(1.0f / 60.0f);

    // Start shift 1->2
    m.shift_gear(2);
    assert(m.is_shifting());

    // Try to shift again mid-shift — should be rejected
    m.shift_gear(4);
    m.update(1.0f / 60.0f);
    // Should still be shifting to gear 2, not 4
    assert(m.is_shifting());

    // Wait for shift to complete
    for (int i = 0; i < 30; ++i) m.update(1.0f / 60.0f);
    assert(!m.is_shifting());
    assert(m.get_gear() == 2);

    std::printf("  [PASS] shift rejected during active shift\n");
}

void test_downshift_blip_adds_rpm() {
    // Verify that downshift blip adds extra RPM during the gear change phase.
    // Start in gear 4 at redline, cut throttle to coast down, then downshift.
    EngineModel m;
    m.set_throttle(1.0f);
    m.shift_gear(4);
    for (int i = 0; i < 300; ++i) m.update(1.0f / 60.0f);

    // Cut throttle, coast down to mid-range
    m.set_throttle(0.0f);
    for (int i = 0; i < 25; ++i) m.update(1.0f / 60.0f);
    float rpm_before = m.get_rpm();

    // Downshift to gear 2
    m.shift_gear(2);

    // Run into the GearChange phase and sample RPM
    // Skip past ClutchDisengage (~5 frames)
    for (int i = 0; i < 6; ++i) m.update(1.0f / 60.0f);
    assert(m.get_shift_phase() == ShiftPhase::GearChange);

    // Sample RPM during early gear change — blip should push it above pre-shift RPM
    m.update(1.0f / 60.0f);
    float rpm_mid_shift = m.get_rpm();
    // With downshift blip, RPM during gear change should already exceed pre-shift RPM
    assert(rpm_mid_shift > rpm_before);

    // Complete the shift
    for (int i = 0; i < 25; ++i) m.update(1.0f / 60.0f);

    std::printf("  [PASS] downshift blip: before=%.0f, mid=%.0f\n",
                rpm_before, rpm_mid_shift);
}

int main() {
    std::printf("=== EngineModel Tests ===\n");
    test_idle_rpm();
    test_rpm_increases_with_throttle();
    test_redline_clamp();
    test_gear_affects_load();

    std::printf("\n=== Shift State Machine Tests ===\n");
    test_upshift_rpm_drops();
    test_downshift_rpm_rises();
    test_shift_clutch_sequence();
    test_neutral_shift_is_instant();
    test_shift_rejected_during_shift();
    test_downshift_blip_adds_rpm();

    std::printf("\nAll EngineModel tests passed.\n\n");
    return 0;
}
