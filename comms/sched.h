#pragma once
#include <stdint.h>

typedef struct {
    uint16_t minute;
    uint8_t enabled;
    uint32_t last_day;
} sched_t;

/* local_s is local seconds since 1970, 0 when the clock is not set. Fires once per day. */
int sched_due(sched_t *s, uint32_t local_s);
