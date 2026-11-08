#include <math.h>
#include "lift.h"

#define ONE_G 16393.0f
#define DEV_LIMIT 0.30f
#define TILT_LIMIT 0.80f
#define SAMPLES 10

int lift_update(lift_t *l, const int16_t a[3])
{
    float x = a[0], y = a[1], z = a[2];
    float m = sqrtf(x * x + y * y + z * z);
    int bad = fabsf(m - ONE_G) > DEV_LIMIT * ONE_G || z < TILT_LIMIT * m;
    l->n = bad ? (l->n < 255 ? l->n + 1 : l->n) : 0;
    return l->n >= SAMPLES;
}
