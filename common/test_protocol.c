#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "protocol.h"

static int feed_all(proto_rx_t *rx, const uint8_t *b, size_t n, proto_msg_t *m)
{
    int r = 0;
    for (size_t i = 0; i < n; i++) {
        int x = proto_feed(rx, b[i], m);
        if (x)
            r = x;
    }
    return r;
}

int main(void)
{
    uint8_t f[PROTO_MAX_FRAME];
    proto_rx_t rx = {0};
    proto_msg_t m;

    uint8_t p[] = {1, 2, 3};
    size_t n = proto_encode(MSG_STATE, p, sizeof p, f);
    assert(n == 8);
    assert(feed_all(&rx, f, n, &m) == 1);
    assert(m.type == MSG_STATE && m.len == 3 && !memcmp(m.payload, p, 3));

    n = proto_encode(MSG_HEARTBEAT, NULL, 0, f);
    assert(feed_all(&rx, f, n, &m) == 1 && m.len == 0);

    n = proto_encode(MSG_STATE, p, sizeof p, f);
    f[4] ^= 0xFF;
    assert(feed_all(&rx, f, n, &m) == -1);

    uint8_t junk[] = {0x00, 0x13, 0x37};
    assert(feed_all(&rx, junk, sizeof junk, &m) == 0);
    n = proto_encode(MSG_ALARM_STOP, NULL, 0, f);
    assert(feed_all(&rx, f, n, &m) == 1 && m.type == MSG_ALARM_STOP);

    assert(proto_encode(0, NULL, PROTO_MAX_PAYLOAD + 1, f) == 0);
    puts("ok");
    return 0;
}
