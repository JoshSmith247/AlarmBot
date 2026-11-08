#pragma once
#include <stdint.h>

typedef struct {
    uint8_t edge_l, edge_r, obstacle, lifted, link_ok;
} fsm_in_t;

typedef struct {
    uint8_t state, speed, phase;
    int8_t dir;
    uint32_t t0;
} fsm_t;

void fsm_init(fsm_t *f);
void fsm_start(fsm_t *f, uint8_t speed);
void fsm_stop(fsm_t *f);
/* left/right are -100..100, positive is forward. */
void fsm_step(fsm_t *f, const fsm_in_t *in, uint32_t now, int8_t *left, int8_t *right);
uint8_t fsm_flags(const fsm_in_t *in);
