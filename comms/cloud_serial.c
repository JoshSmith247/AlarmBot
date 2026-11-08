#include <stdio.h>
#include "pico/stdlib.h"
#include "cloud.h"

void cloud_init(void)
{
}

int cloud_poll(char *line, size_t n)
{
    static size_t len;
    int c;
    while ((c = getchar_timeout_us(0)) != PICO_ERROR_TIMEOUT) {
        if (c == '\n' || c == '\r') {
            if (!len)
                continue;
            line[len] = 0;
            len = 0;
            return 1;
        }
        if (len < n - 1)
            line[len++] = (char)c;
    }
    return 0;
}

void cloud_publish(const char *msg)
{
    printf("pub: %s\n", msg);
}
