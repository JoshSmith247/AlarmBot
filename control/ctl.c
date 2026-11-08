#include "ctl.h"
#include "messages.h"

void ctl_init(ctl_t *c, send_fn send)
{
    *c = (ctl_t){.send = send, .last_state = 0xFF};
    fsm_init(&c->fsm);
}

void ctl_rx(ctl_t *c, const proto_msg_t *m, uint32_t now)
{
    link_rx(&c->link, now);
    if (m->type == MSG_HEARTBEAT)
        c->send(MSG_HEARTBEAT, 0, 0);
    else if (m->type == MSG_ALARM_START)
        fsm_start(&c->fsm, m->len ? m->payload[0] : 0);
    else if (m->type == MSG_ALARM_STOP)
        fsm_stop(&c->fsm);
}

void ctl_tick(ctl_t *c, const ctl_sense_t *s, uint32_t now, int8_t *left, int8_t *right)
{
    fsm_in_t in = {
        .edge_l = s->edge_l,
        .edge_r = s->edge_r,
        .obstacle = s->dist_cm[0] < OBSTACLE_CM || s->dist_cm[1] < OBSTACLE_CM,
        .lifted = lift_update(&c->lift, s->acc),
        .link_ok = link_alive(&c->link, now),
    };
    fsm_step(&c->fsm, &in, now, left, right);

    uint8_t flags = fsm_flags(&in);
    if (c->fsm.state != c->last_state || flags != c->last_flags || (int32_t)(now - c->next_report) >= 0) {
        uint8_t p[2] = {c->fsm.state, flags};
        c->send(MSG_STATE, p, 2);
        c->last_state = c->fsm.state;
        c->last_flags = flags;
        c->next_report = now + 500;
    }
}
