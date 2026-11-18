#ifndef CONFIG_H
#define CONFIG_H
 
// ===================== BIẾN TRỞ GA (THROTTLE) =====================
#define THROTTLE_PIN        34   // SENSOR_VP - ADC1, input only
 
// ===================== ĐỘNG CƠ - TB6612FNG =====================
#define MOTOR_PWM_PIN        25   // PWMA
#define MOTOR_AI1_PIN         26   // AI1
#define MOTOR_AI2_PIN         27   // AI2
#define MOTOR_STBY_PIN        14   // STBY (HIGH = cho phép driver chạy)
 
#define PWM_CHANNEL           0
#define PWM_FREQ              5000   // 5kHz
#define PWM_RESOLUTION        8      // 0-255
 
// ===================== ENCODER =====================
#define ENCODER_PIN_A         19
#define ENCODER_PIN_B         18
 
#define PULSES_PER_OUTPUT_REV  248.7
 
#define SPEED_CALC_INTERVAL_MS 100   // tính RPM mỗi 100ms

// ===================== CAN BUS =====================
#define CAN_TX_PIN           16
#define CAN_RX_PIN           17
#define CAN_USE_NO_ACK_MODE  0
#endif