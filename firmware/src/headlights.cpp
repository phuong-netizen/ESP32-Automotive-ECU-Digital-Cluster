#include <Arduino.h>
#include "headlights.h"

namespace {

// SW2: chân giữa nối GND.  
constexpr uint8_t SWITCH_LOW_PIN  = 36;
constexpr uint8_t SWITCH_HIGH_PIN = 33;
// Theo PCB: K1 bật/tắt nguồn đèn, K2 chọn cốt/pha.
constexpr uint8_t RELAY_ENABLE_PIN = 23; // K1, chung LED D5
constexpr uint8_t RELAY_SELECT_PIN = 13; // K2

constexpr uint32_t DEBOUNCE_MS = 30;

// Khoảng chờ ban đầu cho relay nhả/chuyển tiếp điểm.
// Cần kiểm tra lại theo relay thực tế khi thử tải.
constexpr uint32_t RELAY_WAIT_MS = 50;

enum class Phase {
    READY,
    WAIT_K1_RELEASE,
    WAIT_K2_SETTLE
};

HeadlightMode candidate = HeadlightMode::OFF;
HeadlightMode requested = HeadlightMode::OFF;
HeadlightMode applied   = HeadlightMode::OFF;

Phase phase = Phase::READY;
bool selectedHigh = false;

uint32_t candidateSince = 0;
uint32_t phaseSince = 0;

HeadlightMode readSwitch()
{
    const bool low  = digitalRead(SWITCH_LOW_PIN) == LOW;
    const bool high = digitalRead(SWITCH_HIGH_PIN) == LOW;

    // Hai chân cùng ngắt hoặc cùng đóng: tắt đèn.
    if (low == high) return HeadlightMode::OFF;

    return low ? HeadlightMode::LOW_BEAM
               : HeadlightMode::HIGH_BEAM;
}

void selectBeam(bool high)
{
    selectedHigh = high;
    digitalWrite(RELAY_SELECT_PIN, high ? HIGH : LOW);
}

} // namespace

void initHeadlights()
{
    digitalWrite(RELAY_ENABLE_PIN, LOW);
    digitalWrite(RELAY_SELECT_PIN, LOW);
    pinMode(RELAY_ENABLE_PIN, OUTPUT);
    pinMode(RELAY_SELECT_PIN, OUTPUT);

    // PCB đã có điện trở kéo lên R15/R16.
    // GPIO36 không có pull-up nội.
    pinMode(SWITCH_LOW_PIN, INPUT);
    pinMode(SWITCH_HIGH_PIN, INPUT);

    requested = HeadlightMode::OFF;
    applied = HeadlightMode::OFF;
    selectedHigh = false;

    candidate = readSwitch();
    candidateSince = millis();

    // Chờ K1 nhả trước khi cho phép chuyển K2.
    phase = Phase::WAIT_K1_RELEASE;
    phaseSince = candidateSince;
}

void updateHeadlights()
{
    const uint32_t now = millis();
    const HeadlightMode reading = readSwitch();

    if (reading != candidate) {
        candidate = reading;
        candidateSince = now;
    }

    if (now - candidateSince >= DEBOUNCE_MS) {
        requested = candidate;
    }

    switch (phase) {
    case Phase::READY:
        if (requested != applied) {
            // Ngắt nguồn đèn trước khi chuyển cốt/pha.
            digitalWrite(RELAY_ENABLE_PIN, LOW);
            applied = HeadlightMode::OFF;

            phaseSince = now;
            phase = Phase::WAIT_K1_RELEASE;
        }
        break;

    case Phase::WAIT_K1_RELEASE:
        if (now - phaseSince >= RELAY_WAIT_MS) {
            selectBeam(requested == HeadlightMode::HIGH_BEAM);

            phaseSince = now;
            phase = Phase::WAIT_K2_SETTLE;
        }
        break;

    case Phase::WAIT_K2_SETTLE: {
        const bool wantHigh =
            requested == HeadlightMode::HIGH_BEAM;

        // Nếu người dùng đổi hướng trong lúc chờ,
        // cập nhật K2 và chờ lại; K1 vẫn tắt.
        if (wantHigh != selectedHigh) {
            selectBeam(wantHigh);
            phaseSince = now;
        }

        if (now - phaseSince >= RELAY_WAIT_MS) {
            digitalWrite(
                RELAY_ENABLE_PIN,
                requested == HeadlightMode::OFF ? LOW : HIGH
            );

            applied = requested;
            phase = Phase::READY;
        }
        break;
    }
    }
}

HeadlightMode getHeadlightMode()
{
    return applied;
}