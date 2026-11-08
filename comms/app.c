#include "app.h"
#include "messages.h"

void app_init(app_t *a, const app_io_t *io)
{
    *a = (app_t){.io = *io, .face = 0xFF};
    a->io.face(a->face = FACE_AWAKE);
}

static void set_face(app_t *a, uint8_t f)
{
    if (f != a->face)
        a->io.face(a->face = f);
}

void app_set_alarm(app_t *a, uint16_t minute, int enabled)
{
    a->sched.minute = minute;
    a->sched.enabled = enabled;
}

void app_ring(app_t *a, uint32_t now)
{
    uint8_t speed = DEFAULT_SPEED;
    a->ringing = 1;
    a->io.buzzer(a->beep = 1);
    a->ring_start = now;
    a->next_beep = now + BEEP_MS;
    set_face(a, FACE_ANGRY);
    a->io.send(MSG_ALARM_START, &speed, 1);
    a->next_start = now + HEARTBEAT_MS;
    a->io.say("ringing");
}

void app_stop(app_t *a)
{
    a->ringing = 0;
    a->io.buzzer(0);
    a->io.send(MSG_ALARM_STOP, 0, 0);
    set_face(a, FACE_AWAKE);
    a->io.say("stopped");
}

void app_rx(app_t *a, const proto_msg_t *m, uint32_t now)
{
    link_rx(&a->link, now);
    if (m->type != MSG_STATE || m->len < 1)
        return;
    a->ctl_state = m->payload[0];
    if (a->ctl_state == ST_CAUGHT && a->ringing) {
        app_stop(a);
        set_face(a, FACE_HAPPY);
        a->io.say("caught");
    }
}

void app_tick(app_t *a, uint32_t local_s, uint32_t now)
{
    if ((int32_t)(now - a->next_hb) >= 0) {
        a->io.send(MSG_HEARTBEAT, 0, 0);
        a->next_hb = now + HEARTBEAT_MS;
    }
    if (sched_due(&a->sched, local_s))
        app_ring(a, now);
    if (a->ringing && now - a->ring_start >= RING_MAX_MS) {
        app_stop(a);
        a->io.say("timeout");
    }
    if (a->ringing && (int32_t)(now - a->next_beep) >= 0) {
        a->io.buzzer(a->beep ^= 1);
        a->next_beep = now + BEEP_MS;
    }
    if (a->ringing && a->ctl_state != ST_FLEEING && (int32_t)(now - a->next_start) >= 0) {
        uint8_t speed = DEFAULT_SPEED;
        a->io.send(MSG_ALARM_START, &speed, 1);
        a->next_start = now + HEARTBEAT_MS;
    }
    if (!a->ringing && a->face == FACE_HAPPY && a->ctl_state == ST_IDLE)
        set_face(a, FACE_AWAKE);
}
