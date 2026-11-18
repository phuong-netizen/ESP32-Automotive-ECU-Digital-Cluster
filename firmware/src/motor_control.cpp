#include <Arduino.h>
#include "motor_control.h"
#include "config.h"
 
void initMotor() {
  pinMode(MOTOR_AI1_PIN, OUTPUT);
  pinMode(MOTOR_AI2_PIN, OUTPUT);
  pinMode(MOTOR_STBY_PIN, OUTPUT);
  digitalWrite(MOTOR_STBY_PIN, HIGH);   // thoát chế độ standby, cho phép driver chạy
 
  ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(MOTOR_PWM_PIN, PWM_CHANNEL);
 
  setMotor(0); // đảm bảo động cơ dừng lúc khởi động
}
 
void setMotor(int speedPercent) {
  speedPercent = constrain(speedPercent, -100, 100);
 
  if (speedPercent > 0) {
    digitalWrite(MOTOR_AI1_PIN, HIGH);
    digitalWrite(MOTOR_AI2_PIN, LOW);
  } else if (speedPercent < 0) {
    digitalWrite(MOTOR_AI1_PIN, LOW);
    digitalWrite(MOTOR_AI2_PIN, HIGH);
  } else {
    // Thắng ngắn mạch (short brake)
    digitalWrite(MOTOR_AI1_PIN, LOW);
    digitalWrite(MOTOR_AI2_PIN, LOW);
  }
 
  int duty = map(abs(speedPercent), 0, 100, 0, 255);
  ledcWrite(PWM_CHANNEL, duty);
}
 