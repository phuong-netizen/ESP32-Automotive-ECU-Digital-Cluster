#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H
 
//Khoi tao chan dieu khien dong co + PWM
void initMotor();
 
// Điều khiển tốc độ/chiều động cơ
// speedPercent: -100..100 (âm = lùi, dương = tiến, 0 = dừng/thắng)
void setMotor(int speedPercent);
 
#endif