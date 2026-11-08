#include <stdio.h>
#include <string.h>
#include "lwip/dns.h"
#include "lwip/tcp.h"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "cloud.h"
#include "config.h"

#define POLL_MS 3000
#define TIMEOUT_MS 8000

static struct tcp_pcb *pcb;
static ip_addr_t server;
static char rx[512], cmds[256], say[96];
static size_t rx_len, cmds_len;
static uint8_t busy = 1, boot = 1;
static uint32_t started, next_poll;

static void done(void)
{
    char *body = strstr(rx, "\r\n\r\n");
    if (body && !strncmp(rx, "HTTP/1.", 7) && rx[9] == '2') {
        body += 4;
        size_t n = strlen(body);
        if (cmds_len + n < sizeof cmds) {
            memcpy(cmds + cmds_len, body, n);
            cmds_len += n;
        }
        say[0] = 0;
        boot = 0;
    }
    busy = 0;
}

static err_t recv_cb(void *arg, struct tcp_pcb *p, struct pbuf *b, err_t err)
{
    (void)arg;
    (void)err;
    if (!b) {
        done();
        tcp_close(p);
        pcb = NULL;
        return ERR_OK;
    }
    size_t n = pbuf_copy_partial(b, rx + rx_len, sizeof rx - 1 - rx_len, 0);
    rx[rx_len += n] = 0;
    tcp_recved(p, b->tot_len);
    pbuf_free(b);
    return ERR_OK;
}

static void err_cb(void *arg, err_t err)
{
    (void)arg;
    (void)err;
    pcb = NULL;
    busy = 0;
}

static err_t connected_cb(void *arg, struct tcp_pcb *p, err_t err)
{
    (void)arg;
    if (err != ERR_OK)
        return err;
    char req[256];
    int n = snprintf(req, sizeof req, "GET /poll?key=%s&boot=%d&say=%s HTTP/1.0\r\nHost: %s\r\n\r\n", SERVER_KEY, boot, say, SERVER_HOST);
    tcp_write(p, req, n, TCP_WRITE_FLAG_COPY);
    return tcp_output(p);
}

static void connect_now(void)
{
    pcb = tcp_new();
    tcp_recv(pcb, recv_cb);
    tcp_err(pcb, err_cb);
    if (tcp_connect(pcb, &server, SERVER_PORT, connected_cb) != ERR_OK) {
        tcp_abort(pcb);
        pcb = NULL;
        busy = 0;
    }
}

static void dns_cb(const char *name, const ip_addr_t *ip, void *arg)
{
    (void)name;
    (void)arg;
    if (!ip) {
        busy = 0;
        return;
    }
    server = *ip;
    connect_now();
}

static void start(void)
{
    busy = 1;
    started = to_ms_since_boot(get_absolute_time());
    rx_len = 0;
    rx[0] = 0;
    err_t e = dns_gethostbyname(SERVER_HOST, &server, dns_cb, NULL);
    if (e == ERR_OK)
        connect_now();
    else if (e != ERR_INPROGRESS)
        busy = 0;
}

void cloud_init(void)
{
    busy = 0;
}

int cloud_poll(char *line, size_t n)
{
    uint32_t now = to_ms_since_boot(get_absolute_time());
    int got = 0;
    cyw43_arch_lwip_begin();
    char *nl = memchr(cmds, '\n', cmds_len);
    if (nl) {
        size_t len = nl - cmds;
        if (len >= n)
            len = n - 1;
        memcpy(line, cmds, len);
        line[len] = 0;
        cmds_len -= nl + 1 - cmds;
        memmove(cmds, nl + 1, cmds_len);
        got = 1;
    } else if (busy && now - started > TIMEOUT_MS) {
        if (pcb)
            tcp_abort(pcb);
        pcb = NULL;
        busy = 0;
    } else if (!busy && (int32_t)(now - next_poll) >= 0) {
        next_poll = now + POLL_MS;
        if (cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA) == CYW43_LINK_UP)
            start();
    }
    cyw43_arch_lwip_end();
    return got;
}

void cloud_publish(const char *msg)
{
    cyw43_arch_lwip_begin();
    size_t n = strlen(say);
    if (n + strlen(msg) + 2 < sizeof say) {
        if (n)
            say[n++] = ',';
        for (; *msg; msg++)
            say[n++] = *msg == ' ' ? '_' : *msg;
        say[n] = 0;
    }
    cyw43_arch_lwip_end();
}
