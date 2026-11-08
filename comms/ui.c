#include <stdio.h>
#include "pico/stdlib.h"
#include "pins.h"
#include "ui.h"

void ui_init(void)
{
    gpio_init(PIN_BUZZER);
    gpio_set_dir(PIN_BUZZER, GPIO_OUT);
}

void ui_buzzer(int on)
{
    gpio_put(PIN_BUZZER, on);
}

void ui_face(uint8_t face)
{
    printf("face %u\n", face);
}
