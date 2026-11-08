#include <stdio.h>
#include "pico/stdlib.h"
#include "ctl.h"
#include "hw.h"
#include "pins.h"

static void send(uint8_t type, const uint8_t *p, uint8_t len)
{
    uint8_t f[PROTO_MAX_FRAME];
    uart_write_blocking(PROTO_UART, f, proto_encode(type, p, len, f));
}

int main(void)
{
    stdio_init_all();
    uart_init(PROTO_UART, PROTO_BAUD);
    gpio_set_function(PIN_UART_TX, GPIO_FUNC_UART);
    gpio_set_function(PIN_UART_RX, GPIO_FUNC_UART);
    hw_init();

    ctl_t ctl;
    ctl_init(&ctl, send);
    ctl_sense_t s = {.dist_cm = {SONAR_NONE, SONAR_NONE}, .acc = {0, 0, 16393}};
    proto_rx_t rx = {0};
    proto_msg_t msg;
    uint32_t next = 0;

    while (true) {
        uint32_t now = to_ms_since_boot(get_absolute_time());
        while (uart_is_readable(PROTO_UART))
            if (proto_feed(&rx, uart_getc(PROTO_UART), &msg) == 1)
                ctl_rx(&ctl, &msg, now);
        if ((int32_t)(now - next) >= 0) {
            next = now + 20;
            int8_t l, r;
            hw_sense(&s);
#ifdef LOG_IMU
            static uint8_t n;
            if (++n % 10 == 0)
                printf("acc %d %d %d\n", s.acc[0], s.acc[1], s.acc[2]);
#endif
            ctl_tick(&ctl, &s, now, &l, &r);
            hw_motors(l, r);
        }
    }
}
