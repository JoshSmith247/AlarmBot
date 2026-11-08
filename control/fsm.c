#include "fsm.h"
#include "messages.h"

enum { PH_RUN, PH_BACK, PH_TURN };
#define BACK_MS 400
#define TURN_MS 500

void fsm_init(fsm_t *f)
{
    f->state = ST_IDLE;
    f->phase = PH_RUN;
    f->speed = DEFAULT_SPEED;
}

void fsm_start(fsm_t *f, uint8_t speed)
{
    f->speed = speed ? (speed > 100 ? 100 : speed) : DEFAULT_SPEED;
    if (f->state != ST_FLEEING)
        f->phase = PH_RUN;
    f->state = ST_FLEEING;
}

void fsm_stop(fsm_t *f)
{
    f->state = ST_IDLE;
}

uint8_t fsm_flags(const fsm_in_t *in)
{
    return (in->edge_l || in->edge_r ? FL_EDGE : 0) | (in->obstacle ? FL_OBSTACLE : 0) | (in->lifted ? FL_LIFTED : 0);
}

void fsm_step(fsm_t *f, const fsm_in_t *in, uint32_t now, int8_t *left, int8_t *right)
{
    *left = *right = 0;

    if (f->state == ST_FAULT && in->link_ok)
        f->state = ST_IDLE;
    if (f->state != ST_IDLE && !in->link_ok)
        f->state = ST_FAULT;
    if (f->state == ST_FLEEING && in->lifted)
        f->state = ST_CAUGHT;
    if (f->state != ST_FLEEING)
        return;

    int8_t v = (int8_t)f->speed;
    uint32_t dt = now - f->t0;

    if (f->phase == PH_RUN && (in->edge_l || in->edge_r || in->obstacle)) {
        f->phase = PH_BACK;
        f->t0 = now;
        f->dir = in->edge_l ? 1 : in->edge_r ? -1 : 1;
        dt = 0;
    }
    if (f->phase == PH_BACK && dt >= BACK_MS) {
        f->phase = PH_TURN;
        f->t0 = now;
        dt = 0;
    }
    if (f->phase == PH_TURN && dt >= TURN_MS)
        f->phase = PH_RUN;

    if (f->phase == PH_RUN) {
        *left = *right = v;
    } else if (f->phase == PH_BACK) {
        *left = *right = -v;
    } else {
        *left = f->dir * v;
        *right = -f->dir * v;
    }
}
