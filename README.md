# AlarmBot

A Pico 2W handles Wi-Fi and the cloud, a Pico 2 handles motion. They talk over UART.

| Dir | What | Board |
|---|---|---|
| `comms/` | Wi-Fi, NTP, cloud, alarm logic, display, sound | Pico 2W (`pico2_w`) |
| `control/` | IMU, servos, edge stop | Pico 2 (`pico2`) |
| `common/` | UART frame protocol shared by both | |
| `cad/` | Chassis (CadQuery) | |
| `docs/` | Protocol spec | |

## Build

Needs the Pico SDK (`PICO_SDK_PATH`) and an ARM toolchain.

    cp comms/config.example.h comms/config.h   # then fill in Wi-Fi
    cmake -S comms -B build/comms && cmake --build build/comms
    cmake -S control -B build/control && cmake --build build/control

Host test for the protocol:

    cc -I common common/protocol.c common/test_protocol.c -o build/test_protocol && build/test_protocol
