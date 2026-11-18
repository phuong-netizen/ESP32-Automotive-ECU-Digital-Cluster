#include <Arduino.h>
#include "throttle_sensor.h"
#include "config.h"
 
void initThrottle() {
  analogReadResolution(12);   // ESP32: 0-4095
  pinMode(THROTTLE_PIN, INPUT);
}
 
int readThrottlePercent() {
  int raw = analogRead(THROTTLE_PIN);
  int percent = map(raw, 0, 4095, 0, 100);
  return constrain(percent, 0, 100);
}
 