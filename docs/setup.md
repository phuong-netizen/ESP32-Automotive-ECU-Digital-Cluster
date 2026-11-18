# Build and hardware setup

## ESP32 firmware

Use PlatformIO with the Arduino framework and the `esp32doit-devkit-v1` environment:

```sh
cd firmware
pio run -e esp32doit-devkit-v1
pio run -e esp32doit-devkit-v1 -t upload
pio device monitor -b 115200
```

The supplied configuration also lists a `board_test` environment, but its source file is absent from this snapshot. Use the normal application environment above. The original platform dependency is unpinned; verify compatible ESP32 Arduino APIs before upgrading it.

Review [`include/config.h`](../firmware/include/config.h) and the peripheral modules before wiring. The supplied configuration uses CAN TX/RX GPIO16/17, throttle GPIO34, encoder GPIO19/18 and motor PWM GPIO25. Match the encoder pulses-per-revolution setting to the fitted motor. Use appropriate power, shared ground, CAN transceivers and bus termination.

## Raspberry Pi cluster

The receiver requires Linux SocketCAN. Configure the MCP2515 SPI driver and interrupt/oscillator settings for the actual board; these board-specific settings are not supplied as a reproducible image.

Once `can0` exists, configure 500 kbit/s:

```sh
sudo ip link set can0 down
sudo ip link set can0 type can bitrate 500000
sudo ip link set can0 up
```

The CMake project requires Qt 6.8 or later with Qt Quick, CMake 3.16+ and C++20. Point CMake to the installed Qt prefix if necessary:

```sh
cmake -S cluster -B build/cluster
cmake --build build/cluster
./build/cluster/appECU_DigitalCluster
```

The application binds to `can0` and accepts standard data frames `0x100`–`0x102`. It rejects incorrect DLCs and selected out-of-range fields. No valid frame for 1,000 ms marks that group stale; checks occur every 100 ms.

## Suggested bench checks

1. Confirm matching bitrate and proper CAN wiring; observe serial output and, if installed, `candump can0`.
2. Adjust throttle and verify motor rotation, encoder RPM and dashboard changes.
3. Adjust simulated fuel/temperature and select lighting modes.
4. Interrupt CAN data and check the missing-data state; reconnect and observe valid updates.

The report records demonstrations of normal operation, warnings and disconnection. Fresh firmware builds and hardware tests have not been run as part of this documentation update.
