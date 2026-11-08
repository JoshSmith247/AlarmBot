#pragma once
#include <stddef.h>
#include <stdint.h>

#define PROTO_SOF 0xA5
#define PROTO_MAX_PAYLOAD 32
#define PROTO_MAX_FRAME (PROTO_MAX_PAYLOAD + 5)

enum {
    MSG_HEARTBEAT = 0x01,
    MSG_ALARM_START = 0x10,
    MSG_ALARM_STOP = 0x11,
    MSG_STATE = 0x20,
};

typedef struct {
    uint8_t type;
    uint8_t len;
    uint8_t payload[PROTO_MAX_PAYLOAD];
} proto_msg_t;

typedef struct {
    uint8_t buf[PROTO_MAX_FRAME];
    uint8_t pos;
} proto_rx_t;

size_t proto_encode(uint8_t type, const uint8_t *payload, uint8_t len, uint8_t *out);

/* Returns 1 when msg is filled, -1 on a bad frame, 0 otherwise. */
int proto_feed(proto_rx_t *rx, uint8_t byte, proto_msg_t *msg);
