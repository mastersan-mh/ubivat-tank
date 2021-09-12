
#include "system.h"

#include <stdlib.h>

static struct system_time system_time;

void system_time_update(void)
{
    gettimeofday(&system_time.tv, NULL);
    system_time.usec = system_time.tv.tv_sec * 1000000 + system_time.tv.tv_usec;
    system_time.msec = system_time.usec / 1000;
    system_time.sec = system_time.msec / 1000;
}

const struct system_time * system_time_get(void)
{
    return &system_time;
}
