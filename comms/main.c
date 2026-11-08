#include <stdio.h>
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "config.h"
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

    if (cyw43_arch_init())
        return 1;
    cyw43_arch_enable_sta_mode();
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK, 30000))
        printf("wifi failed\n");

    proto_rx_t rx = {0};
    proto_msg_t msg;
    absolute_time_t next_hb = get_absolute_time();

    while (true) {
        while (uart_is_readable(PROTO_UART)) {
            if (proto_feed(&rx, uart_getc(PROTO_UART), &msg) == 1)
                printf("rx type=0x%02x len=%u\n", msg.type, msg.len);
        }
        if (absolute_time_diff_us(get_absolute_time(), next_hb) <= 0) {
            send(MSG_HEARTBEAT, NULL, 0);
            next_hb = make_timeout_time_ms(1000);
        }
        cyw43_arch_poll();
    }
}
