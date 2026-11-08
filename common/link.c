#include "link.h"
#include "messages.h"

void link_rx(link_t *l, uint32_t now)
{
    l->last_rx = now;
    l->seen = 1;
}

int link_alive(const link_t *l, uint32_t now)
{
    return l->seen && (uint32_t)(now - l->last_rx) < LINK_TIMEOUT_MS;
}
