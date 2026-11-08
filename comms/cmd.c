#include <stdio.h>
#include <string.h>
#include "cmd.h"

static int is(const char *line, const char *word)
{
    size_t n = strlen(word);
    return !strncmp(line, word, n) && !line[n + strspn(line + n, " \r")];
}

int cmd_run(app_t *a, const char *line, uint32_t now)
{
    unsigned h, m;
    if (sscanf(line, "set %u:%u", &h, &m) == 2 && h < 24 && m < 60) {
        app_set_alarm(a, h * 60 + m, 1);
        a->io.say("alarm set");
    } else if (is(line, "off")) {
        app_set_alarm(a, a->sched.minute, 0);
        a->io.say("alarm off");
    } else if (is(line, "stop")) {
        app_stop(a);
    } else if (is(line, "ring")) {
        app_ring(a, now);
    } else {
        return -1;
    }
    return 0;
}
