#pragma once
#include "link.h"
#include "protocol.h"
#include "sched.h"

enum { FACE_AWAKE, FACE_ANGRY, FACE_HAPPY };

typedef struct {
    void (*send)(uint8_t type, const uint8_t *payload, uint8_t len);
    void (*buzzer)(int on);
    void (*face)(uint8_t face);
    void (*say)(const char *msg);
} app_io_t;

typedef struct {
    app_io_t io;
    sched_t sched;
    link_t link;
    uint8_t ringing, ctl_state, face, beep;
    uint32_t next_hb, next_start, ring_start, next_beep;
} app_t;

void app_init(app_t *a, const app_io_t *io);
void app_set_alarm(app_t *a, uint16_t minute, int enabled);
void app_ring(app_t *a, uint32_t now);
void app_stop(app_t *a);
void app_rx(app_t *a, const proto_msg_t *m, uint32_t now);
void app_tick(app_t *a, uint32_t local_s, uint32_t now);
