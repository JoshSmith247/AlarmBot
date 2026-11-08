#pragma once
#include <stddef.h>

void cloud_init(void);
/* Copies one pending command line into line and returns 1, or returns 0. */
int cloud_poll(char *line, size_t n);
void cloud_publish(const char *msg);
