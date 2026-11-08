#pragma once
#include <stdint.h>

typedef struct {
    uint8_t n;
} lift_t;

/* Feed one accelerometer sample (LSM6DS3, +-2 g). Returns 1 while the bot looks picked up. */
int lift_update(lift_t *l, const int16_t a[3]);
