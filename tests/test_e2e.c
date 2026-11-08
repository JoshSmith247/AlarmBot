#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "app.h"
#include "cmd.h"
#include "ctl.h"
#include "messages.h"

typedef struct {
    uint8_t buf[4096];
    size_t r, w;
} pipe_t;

static pipe_t to_ctl, to_app;
static int link_cut;
static int buzzer;
static uint8_t face;

static void push(pipe_t *p, uint8_t type, const uint8_t *pl, uint8_t len)
{
    uint8_t f[PROTO_MAX_FRAME];
    size_t n = proto_encode(type, pl, len, f);
    memcpy(p->buf + p->w, f, n);
    p->w += n;
}

static void app_send(uint8_t t, const uint8_t *p, uint8_t l)
{
    if (!link_cut)
        push(&to_ctl, t, p, l);
}
static void ctl_send(uint8_t t, const uint8_t *p, uint8_t l) { push(&to_app, t, p, l); }
static void set_buzzer(int on) { buzzer = on; }
static void set_face(uint8_t f) { face = f; }
static void say(const char *m) { printf("  app: %s\n", m); }

static app_t app;
static ctl_t ctl;
static proto_rx_t rx_c, rx_a;
static ctl_sense_t s;
static uint32_t now, base_local;
static int8_t left, right;

static void tick(void)
{
    proto_msg_t m;
    while (to_ctl.r < to_ctl.w)
        if (proto_feed(&rx_c, to_ctl.buf[to_ctl.r++], &m) == 1)
            ctl_rx(&ctl, &m, now);
    to_ctl.r = to_ctl.w = 0;
    while (to_app.r < to_app.w)
        if (proto_feed(&rx_a, to_app.buf[to_app.r++], &m) == 1)
            app_rx(&app, &m, now);
    to_app.r = to_app.w = 0;
    ctl_tick(&ctl, &s, now, &left, &right);
    app_tick(&app, base_local + now / 1000, now);
    now += 20;
}

static void run_ms(uint32_t ms)
{
    for (uint32_t end = now + ms; now < end;)
        tick();
}

static void flat(void) { s.acc[0] = 0; s.acc[1] = 0; s.acc[2] = 16393; }

int main(void)
{
    app_init(&app, &(app_io_t){app_send, set_buzzer, set_face, say});
    ctl_init(&ctl, ctl_send);
    s = (ctl_sense_t){.dist_cm = {SONAR_NONE, SONAR_NONE}};
    flat();
    base_local = (20000u * 86400u) + 7 * 3600 + 29 * 60 + 56;

    assert(cmd_run(&app, "set 07:30", now) == 0);
    assert(cmd_run(&app, "nonsense", now) == -1);

    run_ms(2000);
    assert(!app.ringing && !buzzer && ctl.fsm.state == ST_IDLE && left == 0);

    run_ms(3000);
    puts("alarm time reached");
    assert(app.ringing && face == FACE_ANGRY);
    assert(ctl.fsm.state == ST_FLEEING && app.ctl_state == ST_FLEEING);
    assert(left == DEFAULT_SPEED && right == DEFAULT_SPEED);

    s.edge_l = 1;
    run_ms(40);
    s.edge_l = 0;
    assert(left < 0 && right < 0);
    run_ms(420);
    assert(left > 0 && right < 0);
    run_ms(540);
    assert(left == DEFAULT_SPEED && right == DEFAULT_SPEED);

    s.dist_cm[1] = 12;
    run_ms(40);
    s.dist_cm[1] = SONAR_NONE;
    assert(left < 0 && right < 0);
    run_ms(1000);

    link_cut = 1;
    run_ms(4000);
    assert(ctl.fsm.state == ST_FAULT && left == 0 && right == 0 && app.ringing);
    link_cut = 0;
    run_ms(2500);
    assert(ctl.fsm.state == ST_FLEEING && left == DEFAULT_SPEED);

    s.acc[1] = 12000;
    s.acc[2] = 11000;
    run_ms(400);
    assert(!app.ringing && !buzzer && face != FACE_ANGRY);
    assert(ctl.fsm.state == ST_IDLE && left == 0 && right == 0);

    flat();
    run_ms(20000);
    assert(!app.ringing && ctl.fsm.state == ST_IDLE);

    assert(cmd_run(&app, "ring now", now) == -1);
    assert(cmd_run(&app, "ring\r", now) == 0);
    assert(buzzer == 1);
    run_ms(500);
    assert(buzzer == 0);
    run_ms(400);
    assert(buzzer == 1);
    run_ms(RING_MAX_MS);
    assert(!app.ringing && !buzzer && ctl.fsm.state == ST_IDLE && left == 0);

    puts("ok");
    return 0;
}
