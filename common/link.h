#pragma once
#include <stdint.h>

typedef struct {
    uint32_t last_rx;
    uint8_t seen;
} link_t;

void link_rx(link_t *l, uint32_t now);
int link_alive(const link_t *l, uint32_t now);
