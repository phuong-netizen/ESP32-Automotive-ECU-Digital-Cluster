#ifndef CAN_BUS_H
#define CAN_BUS_H
#include <Arduino.h>
// =====================================================================
//  BẢNG CAN ID  (tốc độ bus: 500 kbps, khung chuẩn 11-bit)
//  Quy ước: dữ liệu nhiều byte dùng little-endian (byte thấp trước)
//
//  0x100  ECU -> Pi  mỗi 50 ms   DLC 4
//         B0-B1: tốc độ quay trục ra, đơn vị 0.1 RPM (uint16)
//         B2   : mức ga 0..100 (%)
//         B3   : chiều quay (int8: 0x01 = tiến, 0xFF = lùi)
//
//  0x101  ECU -> Pi  mỗi 100 ms  DLC 2
//         B0   : mức nhiên liệu 0..100 (%)
//         B1   : nhiệt độ nước làm mát (°C, uint8)
//
//  0x102  ECU -> Pi  mỗi 100 ms  DLC 2
//         B0   : xi-nhan (0 = tắt, 1 = trái, 2 = phải)
//         B1   : đèn (0 = tắt, 1 = cốt, 2 = pha)
//
//  0x200  Pi -> ECU  khi đổi chế độ  DLC 1
//         B0   : Drive Mode (0 = Eco, 1 = Normal, 2 = Sport)
// =====================================================================
#define CAN_ID_MOTION          0x100
#define CAN_ID_SENSORS         0x101
#define CAN_ID_LIGHTS          0x102
#define CAN_ID_DRIVE_MODE_CMD  0x200

#define CAN_PERIOD_MOTION_MS   50
#define CAN_PERIOD_SENSORS_MS  100
#define CAN_PERIOD_LIGHTS_MS   100

enum TurnSignalState : uint8_t { TURN_OFF = 0, TURN_LEFT = 1, TURN_RIGHT = 2 };
enum LampState       : uint8_t { LAMP_OFF = 0, LAMP_LOW = 1, LAMP_HIGH = 2 };
enum DriveMode       : uint8_t { DRIVE_ECO = 0, DRIVE_NORMAL = 1, DRIVE_SPORT = 2 };

// Dữ liệu ECU muốn gửi lên Pi. Các module khác (encoder, throttle,
// fuel_temp_sim, turn_signal, headlight) điền vào rồi truyền cho canBusUpdate().
struct CanTxData {
  float    wheelRpm;      // RPM trục ra (có thể âm, hàm sẽ lấy trị tuyệt đối)
  uint8_t  throttlePct;   // 0..100
  int8_t   direction;     // 1 = tiến, -1 = lùi
  uint8_t  fuelPct;       // 0..100
  uint8_t  coolantTempC;  // °C
  uint8_t  turnSignal;    // TurnSignalState
  uint8_t  lampMode;      // LampState
};

// Khởi tạo driver TWAI và bắt đầu chạy. Trả về false nếu lỗi.
bool canBusInit();

// Gọi liên tục trong loop(): tự gửi các khung theo chu kỳ,
// nhận lệnh từ Pi, và tự phục hồi nếu bus bị lỗi (bus-off).
void canBusUpdate(const CanTxData& data);

// Drive Mode gần nhất nhận được từ Pi (mặc định Normal).
DriveMode canBusGetDriveMode();

// Số lần đẩy khung vào hàng đợi gửi thất bại (dùng để debug).
uint32_t canBusGetTxFailCount();

#endif