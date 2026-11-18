#include <Arduino.h>
#include "aux_sensors.h"

namespace {

constexpr uint8_t AUX2_PIN = 35; // RV2
constexpr uint8_t AUX3_PIN = 32; // RV3

constexpr uint32_t SAMPLE_MS = 20;

float filtered2 = 0;
float filtered3 = 0;
uint32_t lastSample = 0;

int toPercent(float value)
{
    return constrain(
        static_cast<int>(value * 100.0f / 4095.0f + 0.5f),
        0, 100
    );
}

} // namespace

void initAuxSensors()
{
    pinMode(AUX2_PIN, INPUT);
    pinMode(AUX3_PIN, INPUT);

    analogReadResolution(12);
    analogSetPinAttenuation(AUX2_PIN, ADC_11db);
    analogSetPinAttenuation(AUX3_PIN, ADC_11db);

    filtered2 = analogRead(AUX2_PIN);
    filtered3 = analogRead(AUX3_PIN);
    lastSample = millis();
}

void updateAuxSensors()
{
    const uint32_t now = millis();

    if (now - lastSample < SAMPLE_MS) return;
    lastSample = now;

    // Lọc nhẹ để giảm rung số khi xoay biến trở.
    filtered2 += 0.2f * (analogRead(AUX2_PIN) - filtered2);
    filtered3 += 0.2f * (analogRead(AUX3_PIN) - filtered3);
}

int getAux2Raw()
{
    return static_cast<int>(filtered2 + 0.5f);
}

int getAux3Raw()
{
    return static_cast<int>(filtered3 + 0.5f);
}

int getAux2Percent()
{
    return toPercent(filtered2);
}

int getAux3Percent()
{
    return toPercent(filtered3);
}