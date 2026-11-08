#pragma once
#include "hardware/uart.h"

#define PROTO_UART uart1
#define PROTO_BAUD 115200
#define PIN_UART_TX 4
#define PIN_UART_RX 5
#define PIN_WAKE 6

#ifdef BOARD_CONTROL
#define PIN_SERVO_L 10
#define PIN_SERVO_R 11
#define MOTOR_SIGN_L 1
#define MOTOR_SIGN_R -1
#define PIN_EDGE_L 12
#define PIN_EDGE_R 13
#define EDGE_ACTIVE 1
#define PIN_TRIG0 14
#define PIN_ECHO0 15
#define PIN_TRIG1 16
#define PIN_ECHO1 17
#define PIN_IMU_SDA 20
#define PIN_IMU_SCL 21
#endif

#ifdef BOARD_COMMS
#define PIN_EYES_L 2
#define PIN_EYES_R 3
#define PIN_OLED_SDA 14
#define PIN_OLED_SCL 15
#define PIN_BUZZER 16
#endif
