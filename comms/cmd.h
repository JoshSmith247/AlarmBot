#pragma once
#include "app.h"

/* set HH:MM | off | stop | ring. Returns 0 when handled, -1 otherwise. */
int cmd_run(app_t *a, const char *line, uint32_t now);
