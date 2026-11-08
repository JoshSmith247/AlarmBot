#include "protocol.h"
#include <string.h>

static uint16_t crc16(const uint8_t *d, size_t n)
{
    uint16_t c = 0xFFFF;
    while (n--) {
        c ^= (uint16_t)*d++ << 8;
        for (int i = 0; i < 8; i++)
            c = (c & 0x8000) ? (uint16_t)((c << 1) ^ 0x1021) : (uint16_t)(c << 1);
    }
    return c;
}

size_t proto_encode(uint8_t type, const uint8_t *payload, uint8_t len, uint8_t *out)
{
    if (len > PROTO_MAX_PAYLOAD)
        return 0;
    out[0] = PROTO_SOF;
    out[1] = type;
    out[2] = len;
    if (len)
        memcpy(out + 3, payload, len);
    uint16_t c = crc16(out + 1, 2 + len);
    out[3 + len] = c >> 8;
    out[4 + len] = c & 0xFF;
    return 5 + len;
}

int proto_feed(proto_rx_t *rx, uint8_t b, proto_msg_t *m)
{
    if (rx->pos == 0 && b != PROTO_SOF)
        return 0;
    rx->buf[rx->pos++] = b;
    if (rx->pos < 3)
        return 0;
    uint8_t len = rx->buf[2];
    if (len > PROTO_MAX_PAYLOAD) {
        rx->pos = 0;
        return -1;
    }
    if (rx->pos < 5 + len)
        return 0;
    rx->pos = 0;
    uint16_t want = (uint16_t)(rx->buf[3 + len] << 8 | rx->buf[4 + len]);
    if (crc16(rx->buf + 1, 2 + len) != want)
        return -1;
    m->type = rx->buf[1];
    m->len = len;
    memcpy(m->payload, rx->buf + 3, len);
    return 1;
}
