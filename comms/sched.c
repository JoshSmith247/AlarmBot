#include "sched.h"

int sched_due(sched_t *s, uint32_t local_s)
{
    if (!s->enabled || local_s < 1700000000u)
        return 0;
    uint32_t day = local_s / 86400;
    if (day == s->last_day || (local_s % 86400) / 60 != s->minute)
        return 0;
    s->last_day = day;
    return 1;
}
