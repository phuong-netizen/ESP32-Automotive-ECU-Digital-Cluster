#include <Arduino.h>
#include "turn_signals.h"

namespace {

// Theo PCB: SW1 dùng GPIO4/5; D3/D4 dùng GPIO22/21.
// Nếu gạt trái lại sáng phải, đổi hai số 4 và 5 bên dưới.
constexpr uint8_t SWITCH_LEFT_PIN  = 4;
constexpr uint8_t SWITCH_RIGHT_PIN = 5;

constexpr uint8_t LED_LEFT_PIN  = 22; // D3
constexpr uint8_t LED_RIGHT_PIN = 21; // D4

constexpr uint32_t DEBOUNCE_MS = 30;
constexpr uint32_t BLINK_MS    = 500; // 500 ms sáng, 500 ms tắt

enum class Direction : uint8_t {
    OFF,
    LEFT,
    RIGHT
};

Direction candidate = Direction::OFF;
Direction active    = Direction::OFF;

uint32_t candidateSince = 0;
uint32_t lastBlink      = 0;
bool ledOn = false;

Direction readSwitch()
{
    // Công tắc nối xuống GND: LOW là đang chọn.
    const bool left  = digitalRead(SWITCH_LEFT_PIN) == LOW;
    const bool right = digitalRead(SWITCH_RIGHT_PIN) == LOW;

    // Cả hai ngắt = OFF; cả hai đóng bất thường cũng cho OFF.
    if (left == right) return Direction::OFF;

    return left ? Direction::LEFT : Direction::RIGHT;
}

void writeLeds()
{
    digitalWrite(
        LED_LEFT_PIN,
        active == Direction::LEFT && ledOn ? HIGH : LOW
    );

    digitalWrite(
        LED_RIGHT_PIN,
        active == Direction::RIGHT && ledOn ? HIGH : LOW
    );
}

} // namespace

void initTurnSignals()
{
    pinMode(SWITCH_LEFT_PIN, INPUT_PULLUP);
    pinMode(SWITCH_RIGHT_PIN, INPUT_PULLUP);

    digitalWrite(LED_LEFT_PIN, LOW);
    digitalWrite(LED_RIGHT_PIN, LOW);
    pinMode(LED_LEFT_PIN, OUTPUT);
    pinMode(LED_RIGHT_PIN, OUTPUT);

    active = Direction::OFF;
    candidate = readSwitch();
    candidateSince = millis();
    lastBlink = candidateSince;
    ledOn = false;
    writeLeds();
}

void updateTurnSignals()
{
    const uint32_t now = millis();
    const Direction reading = readSwitch();

    // Chỉ nhận trạng thái mới khi ổn định ít nhất 30 ms.
    if (reading != candidate) {
        candidate = reading;
        candidateSince = now;
    }

    if (candidate != active &&
        now - candidateSince >= DEBOUNCE_MS) {
        active = candidate;
        ledOn = active != Direction::OFF;
        lastBlink = now;
    }

    if (active == Direction::OFF) {
        ledOn = false;
    } else if (now - lastBlink >= BLINK_MS) {
        lastBlink = now;
        ledOn = !ledOn;
    }

    writeLeds();
}
unsigned char getTurnSignalState()
{
    switch (active) {
        case Direction::LEFT:  return 1;
        case Direction::RIGHT: return 2;
        default:               return 0;
    }
}