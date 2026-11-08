# AlarmBot

A Pico 2W handles Wi-Fi and the cloud, a Pico 2 handles motion. They talk over UART.

| Dir | What | Board |
|---|---|---|
| `comms/` | Wi-Fi, NTP, alarm schedule, commands, buzzer, face | Pico 2W (`pico2_w`) |
| `control/` | Servos, edge and sonar sensors, IMU pick-up, flee state machine | Pico 2 (`pico2`) |
| `common/` | UART frame protocol shared by both | |
| `cad/` | Printable parts, one folder per group | |
| `server/` | Node command server and phone page (no dependencies) | your laptop or a host |
| `tests/` | Host-side end-to-end test of both boards' logic | |
| `docs/` | Protocol spec | |

## Build

Needs the Pico SDK (`PICO_SDK_PATH`) and an ARM toolchain.

    cp comms/config.example.h comms/config.h   # then fill in Wi-Fi
    cmake -S comms -B build/comms && cmake --build build/comms
    cmake -S control -B build/control && cmake --build build/control

Host tests (protocol, plus both boards talking through the real framing):

    make test

Layout: `fsm`, `lift`, `ctl` (control) and `app`, `cmd`, `sched` (comms) are plain C with no SDK calls. `hw.c`, `ui.c`, `cloud_serial.c` and the `main.c` files are the Pico layer.

Commands are text lines (`set 07:30`, `off`, `stop`, `ring`). The comms board polls `server/` every 3 s over plain HTTP, gets the queued lines back and runs them through `cmd_run`. It reports events (`ringing`, `caught`) on the same request. On boot it asks the server to resend the saved alarm, so a reboot does not lose it. Build with `-DCLOUD=serial` to take the same lines from USB serial instead.

Server:

    cd server && KEY=secret node server.js     # page on http://<host>:3000
    node test.js

The page is an installable PWA (manifest, icons, service worker). Full install needs HTTPS; over plain HTTP, iOS Safari's Add to Home Screen still gives a standalone icon, but the service worker does not run.

Set `SERVER_HOST`, `SERVER_PORT` and `SERVER_KEY` in `comms/config.h`. The Pico and the server must be on the same network, or the server must be reachable from it. The link is plain HTTP, so use a private network or add TLS before exposing it.

Not done yet: TLS to a hosted server, OLED and eye matrices (`ui_face` only prints), DST. Firmware compiles for both boards but has not run on hardware.
