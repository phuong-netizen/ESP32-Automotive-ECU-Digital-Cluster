#include <Arduino.h>
#include "config.h"
#include "motor_control.h"
#include "throttle_sensor.h"
#include "encoder.h"
#include "turn_signals.h"
#include "aux_sensors.h"
#include "headlights.h"
#include "can_bus.h"

static bool canOK = false;

// Chuyển trạng thái đèn sang quy ước của giao thức CAN.
static uint8_t getLampStateForCAN()
{
    switch (getHeadlightMode()) {
        case HeadlightMode::LOW_BEAM:
            return LAMP_LOW;

        case HeadlightMode::HIGH_BEAM:
            return LAMP_HIGH;

        default:
            return LAMP_OFF;
    }
}

void setup()
{
    initHeadlights();

    Serial.begin(115200);
    delay(200);

    initMotor();
    initThrottle();
    initEncoder();

    initTurnSignals();
    initAuxSensors();

    canOK = canBusInit();

    if (canOK) {
        Serial.println("[ECU] CAN ready");
    } else {
        Serial.println("[ECU] CAN init failed");
    }
}

void loop()
{
    
    // ===== Cập nhật các chức năng hiện tại =====
    int throttlePercent = constrain(readThrottlePercent(), 0, 100);
    setMotor(throttlePercent);

    updateSpeed();
    updateTurnSignals();
    updateAuxSensors();
    updateHeadlights();

    // ===== Chụp dữ liệu hiện tại để gửi CAN =====
    CanTxData data = {};

    data.wheelRpm = getRPM();
    data.throttlePct = static_cast<uint8_t>(throttlePercent);
    data.direction = getDirection();

    // RV2: mô phỏng nhiên liệu 0..100%.
    int fuelPercent = constrain(getAux2Percent(), 0, 100);
    data.fuelPct = static_cast<uint8_t>(fuelPercent);

    // RV3: mô phỏng nhiệt độ nước 20..120 độ C.
    int temperaturePercent = constrain(getAux3Percent(), 0, 100);
    data.coolantTempC = static_cast<uint8_t>(
        map(temperaturePercent, 0, 100, 20, 120)
    );

    data.turnSignal = getTurnSignalState();
    data.lampMode = getLampStateForCAN();

    // Module CAN tự quản lý chu kỳ gửi và nhận lệnh.
    if (canOK) {
        canBusUpdate(data);
    }

    // ===== In thông tin debug mỗi 500 ms =====
    static uint32_t lastPrint = 0;
    uint32_t now = millis();

    if (now - lastPrint >= 500) {
        lastPrint = now;

        Serial.printf(
    "Ga:%u%% RPM:%.1f Chieu:%d "
    "Nhien lieu:%u%% Nhiet do:%uC "
    "Xi nhan:%u Den:%u Mode:%u CAN queue fail:%lu ENC_RAW:%ld\n",
    static_cast<unsigned>(data.throttlePct),
    data.wheelRpm,
    static_cast<int>(data.direction),
    static_cast<unsigned>(data.fuelPct),
    static_cast<unsigned>(data.coolantTempC),
    static_cast<unsigned>(data.turnSignal),
    static_cast<unsigned>(data.lampMode),
    static_cast<unsigned>(canBusGetDriveMode()),
    static_cast<unsigned long>(canBusGetTxFailCount()),
    getRawPulseCount()
);
    }
    delay(10);
}