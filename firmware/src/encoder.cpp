#include <Arduino.h>
#include "encoder.h"
#include "config.h"
 
// ===================== BIẾN NỘI BỘ MODULE =====================
static volatile long encoderCount = 0;      // reset mỗi lần updateSpeed() tính RPM
static volatile long rawPulseTotal = 0;     // KHÔNG reset - dùng để hiệu chỉnh
static volatile int8_t encoderDirection = 1;
 
static unsigned long lastSpeedCalcTime = 0;
static float currentRPM = 0.0;
 
// ===================== NGẮT ĐỌC ENCODER =====================
// Decode x1: bắt cạnh lên kênh A, đọc trạng thái kênh B để suy ra chiều quay.
// LƯU Ý: theo sơ đồ, nhãn dây "Encoder A/B Phase" bị đảo giữa 2 đầu
// (đầu ESP32 và đầu terminal động cơ ghi ngược nhau). Nếu chiều quay
// đọc được ngược với thực tế, chỉ cần đảo ENCODER_PIN_A và ENCODER_PIN_B
// trong config.h, không cần sửa code ở đây.
void IRAM_ATTR encoderISR() {
  if (digitalRead(ENCODER_PIN_B) == HIGH) {
    encoderDirection = 1;
    encoderCount++;
    rawPulseTotal++;
  } else {
    encoderDirection = -1;
    encoderCount--;
    rawPulseTotal++;   // dùng trị tuyệt đối số xung khi hiệu chỉnh, không quan tâm chiều
  }
}
 
void initEncoder() {
  pinMode(ENCODER_PIN_A, INPUT_PULLUP);
  pinMode(ENCODER_PIN_B, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_A), encoderISR, RISING);
 
  lastSpeedCalcTime = millis();
}
 
void updateSpeed() {
  unsigned long now = millis();
  if (now - lastSpeedCalcTime < SPEED_CALC_INTERVAL_MS) return;
 
  // Lấy và reset bộ đếm an toàn (tắt ngắt tạm thời để tránh race condition)
  noInterrupts();
  long pulses = encoderCount;
  encoderCount = 0;
  interrupts();
 
  float dtMinutes = (now - lastSpeedCalcTime) / 60000.0;
  lastSpeedCalcTime = now;
 
  float revolutions = pulses / PULSES_PER_OUTPUT_REV;
  currentRPM = revolutions / dtMinutes;
}
 
float getRPM() {
  return currentRPM;
}
 
int8_t getDirection() {
  return encoderDirection;
}
 
long getRawPulseCount() {
  noInterrupts();
  long total = rawPulseTotal;
  interrupts();
  return total;
}