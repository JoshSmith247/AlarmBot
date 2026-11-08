#include <stdio.h>
#include "lwip/apps/sntp.h"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "app.h"
#include "cloud.h"
#include "cmd.h"
#include "config.h"
#include "pins.h"
#include "ui.h"

static uint32_t clock_s, clock_at_ms;

void ntp_set(uint32_t sec)
{
    clock_s = sec;
    clock_at_ms = to_ms_since_boot(get_absolute_time());
}

static uint32_t local_now(uint32_t now)
{
    return clock_s ? clock_s + (now - clock_at_ms) / 1000 + TZ_OFFSET_S : 0;
}

static void send(uint8_t type, const uint8_t *p, uint8_t len)
{
    uint8_t f[PROTO_MAX_FRAME];
    uart_write_blocking(PROTO_UART, f, proto_encode(type, p, len, f));
}

static void say(const char *msg)
{
    cloud_publish(msg);
}

int main(void)
{
    stdio_init_all();
    uart_init(PROTO_UART, PROTO_BAUD);
    gpio_set_function(PIN_UART_TX, GPIO_FUNC_UART);
    gpio_set_function(PIN_UART_RX, GPIO_FUNC_UART);
    ui_init();
    cloud_init();

    if (cyw43_arch_init())
        return 1;
    cyw43_arch_enable_sta_mode();
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK, 30000))
        printf("wifi failed\n");
    cyw43_arch_lwip_begin();
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_init();
    cyw43_arch_lwip_end();

    app_t app;
    app_init(&app, &(app_io_t){send, ui_buzzer, ui_face, say});
    proto_rx_t rx = {0};
    proto_msg_t msg;
    char line[48];
    uint32_t next_wifi = 15000;

    while (true) {
        uint32_t now = to_ms_since_boot(get_absolute_time());
        while (uart_is_readable(PROTO_UART))
            if (proto_feed(&rx, uart_getc(PROTO_UART), &msg) == 1)
                app_rx(&app, &msg, now);
        if (cloud_poll(line, sizeof line) && cmd_run(&app, line, now))
            cloud_publish("bad command");
        app_tick(&app, local_now(now), now);
        if ((int32_t)(now - next_wifi) >= 0) {
            next_wifi = now + 15000;
            if (cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA) != CYW43_LINK_UP)
                cyw43_arch_wifi_connect_async(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK);
        }
        cyw43_arch_poll();
    }
}
