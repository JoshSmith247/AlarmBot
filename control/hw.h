#pragma once
#include "ctl.h"

void hw_init(void);
void hw_motors(int8_t left, int8_t right);
void hw_sense(ctl_sense_t *s);
