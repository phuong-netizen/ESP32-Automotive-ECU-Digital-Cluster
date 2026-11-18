#ifndef ENCODER_H
#define ENCODER_H
 
// Cấu hình chân encoder + gắn ngắt
void initEncoder();
 
// Gọi liên tục trong loop() - tự tính RPM mỗi SPEED_CALC_INTERVAL_MS
void updateSpeed();
 
// Lấy tốc độ hiện tại (RPM ở trục ra, sau giảm tốc)
float getRPM();
 
// Lấy chiều quay hiện tại: 1 = tiến, -1 = lùi
int8_t getDirection();
 
// ===== Dùng khi hiệu chỉnh (calibration) =====
// Lấy tổng số xung thô đã đếm được từ lúc initEncoder() (không tự reset).
// Dùng để quay tay/chạy động cơ đúng N vòng ở trục ra, rồi tính:
//   PULSES_PER_OUTPUT_REV = getRawPulseCount() / N
long getRawPulseCount();
 
#endif
 