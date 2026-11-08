#pragma once
#include "fsm.h"
#include "lift.h"
#include "link.h"
#include "protocol.h"

#define OBSTACLE_CM 20
#define SONAR_NONE 0xFFFF

typedef void (*send_fn)(uint8_t type, const uint8_t *payload, uint8_t len);

typedef struct {
    uint8_t edge_l, edge_r;
    uint16_t dist_cm[2];
    int16_t acc[3];
} ctl_sense_t;

typedef struct {
    fsm_t fsm;
    lift_t lift;
    link_t link;
    send_fn send;
    uint8_t last_state, last_flags;
    uint32_t next_report;
} ctl_t;

void ctl_init(ctl_t *c, send_fn send);
void ctl_rx(ctl_t *c, const proto_msg_t *m, uint32_t now);
void ctl_tick(ctl_t *c, const ctl_sense_t *s, uint32_t now, int8_t *left, int8_t *right);
