#include <Arduino.h>
#include <string.h>
#include "driver/twai.h"
#include "can_bus.h"
#include "config.h"
#include <cmath>

// ===================== BIẾN NỘI BỘ MODULE =====================
static bool canReady = false;
static DriveMode driveMode = DRIVE_NORMAL;
static uint32_t  txFailCount = 0;

static uint32_t lastMotionTx  = 0;
static uint32_t lastSensorsTx = 0;
static uint32_t lastLightsTx  = 0;
static uint32_t lastBusCheck  = 0;

// ===================== GỬI 1 KHUNG =====================
// Timeout = 0: không chặn loop(). Nếu hàng đợi gửi đầy (ví dụ chưa có
// node nào ACK) thì bỏ khung này và đếm lỗi, không treo chương trình.
static bool sendFrame(uint32_t id, const uint8_t* data, uint8_t dlc) {
  twai_message_t msg = {};
  msg.identifier       = id;
  msg.data_length_code = dlc;
  memcpy(msg.data, data, dlc);

  if (twai_transmit(&msg, 0) != ESP_OK) {
    txFailCount++;
    return false;
  }
  return true;
}

// ===================== ĐÓNG GÓI TỪNG KHUNG =====================
static void sendMotionFrame(const CanTxData& d) {
  float rpmAbs = std::isfinite(d.wheelRpm)
                 ? std::fabs(d.wheelRpm)
                 : 0.0f;

if (rpmAbs > 6553.5f) rpmAbs = 6553.5f;
uint16_t rpmX10 =
    static_cast<uint16_t>(rpmAbs * 10.0f + 0.5f);

  uint8_t buf[4];
  buf[0] = rpmX10 & 0xFF;                          // little-endian
  buf[1] = (rpmX10 >> 8) & 0xFF;
  buf[2] = d.throttlePct > 100 ? 100 : d.throttlePct;
  buf[3] = (uint8_t)d.direction;                   // 1 -> 0x01, -1 -> 0xFF
  sendFrame(CAN_ID_MOTION, buf, 4);
}

static void sendSensorsFrame(const CanTxData& d) {
  uint8_t buf[2];
  buf[0] = d.fuelPct > 100 ? 100 : d.fuelPct;
  buf[1] = d.coolantTempC;
  sendFrame(CAN_ID_SENSORS, buf, 2);
}

static void sendLightsFrame(const CanTxData& d) {
  uint8_t buf[2];
  buf[0] = d.turnSignal;
  buf[1] = d.lampMode;
  sendFrame(CAN_ID_LIGHTS, buf, 2);
}

// ===================== NHẬN LỆNH TỪ PI =====================
static void handleReceive() {
  // Tránh xử lý CAN quá lâu nếu bus liên tục có dữ liệu.
  for (uint8_t i = 0; i < 8; ++i) {
    twai_message_t rx = {};

    if (twai_receive(&rx, 0) != ESP_OK) break;

    if (rx.extd || rx.rtr) continue;

    if (rx.identifier != CAN_ID_DRIVE_MODE_CMD ||
        rx.data_length_code != 1) {
      continue;
    }

    uint8_t mode = rx.data[0];

    if (mode <= DRIVE_SPORT) {
      driveMode = static_cast<DriveMode>(mode);
    }
  }
}

// ===================== PHỤC HỒI KHI BUS LỖI =====================
static void checkBusHealth() {
  twai_status_info_t status;
  if (twai_get_status_info(&status) != ESP_OK) return;

  if (status.state == TWAI_STATE_BUS_OFF) {
    twai_initiate_recovery();                      // sẽ chuyển sang STOPPED khi xong
  } else if (status.state == TWAI_STATE_STOPPED) {
    twai_start();                                  // chạy lại sau khi phục hồi
  }
}

// ===================== API CÔNG KHAI =====================
bool canBusInit() {
  if (canReady) return true;

  twai_mode_t mode =
      CAN_USE_NO_ACK_MODE ? TWAI_MODE_NO_ACK : TWAI_MODE_NORMAL;

  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(
      (gpio_num_t)CAN_TX_PIN,
      (gpio_num_t)CAN_RX_PIN,
      mode);

  g_config.tx_queue_len = 10;
  g_config.rx_queue_len = 10;

  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  esp_err_t err =
      twai_driver_install(&g_config, &t_config, &f_config);

  if (err != ESP_OK) {
    Serial.printf("[CAN] Install failed: %s\n", esp_err_to_name(err));
    return false;
  }

  err = twai_start();

  if (err != ESP_OK) {
    Serial.printf("[CAN] Start failed: %s\n", esp_err_to_name(err));
    twai_driver_uninstall();
    return false;
  }

  uint32_t now = millis();
  lastMotionTx = lastSensorsTx = lastLightsTx = lastBusCheck = now;

  canReady = true;
  Serial.println("[CAN] Started at 500 kbps");
  return true;
}

void canBusUpdate(const CanTxData& data) {
  if (!canReady) return;

  uint32_t now = millis();

  if (now - lastBusCheck >= 200) {
    lastBusCheck = now;
    checkBusHealth();
  }

  twai_status_info_t status = {};

  if (twai_get_status_info(&status) != ESP_OK ||
      status.state != TWAI_STATE_RUNNING) {
    return;
  }

  handleReceive();

  if (now - lastMotionTx >= CAN_PERIOD_MOTION_MS) {
    lastMotionTx = now;
    sendMotionFrame(data);
  }

  if (now - lastSensorsTx >= CAN_PERIOD_SENSORS_MS) {
    lastSensorsTx = now;
    sendSensorsFrame(data);
  }

  if (now - lastLightsTx >= CAN_PERIOD_LIGHTS_MS) {
    lastLightsTx = now;
    sendLightsFrame(data);
  }
}

DriveMode canBusGetDriveMode() {
  return driveMode;
}

uint32_t canBusGetTxFailCount() {
  return txFailCount;
}