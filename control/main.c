#include <stdio.h>
#include "pico/stdlib.h"
#include "pins.h"
#include "protocol.h"

static void send(uint8_t type, const uint8_t *p, uint8_t len)
{
    uint8_t f[PROTO_MAX_FRAME];
    size_t n = proto_encode(type, p, len, f);
    uart_write_blocking(PROTO_UART, f, n);
}

int main(void)
{
    stdio_init_all();
    uart_init(PROTO_UART, PROTO_BAUD);
    gpio_set_function(PIN_UART_TX, GPIO_FUNC_UART);
    gpio_set_function(PIN_UART_RX, GPIO_FUNC_UART);

    proto_rx_t rx = {0};
    proto_msg_t msg;

    while (true) {
        if (uart_is_readable(PROTO_UART) && proto_feed(&rx, uart_getc(PROTO_UART), &msg) == 1) {
            if (msg.type == MSG_HEARTBEAT)
                send(MSG_HEARTBEAT, NULL, 0);
        }
    }
}
