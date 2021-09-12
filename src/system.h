#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdint.h>
#include <sys/time.h>

struct system_time
{
    struct timeval tv;
    uint64_t usec;
    uint64_t msec;
    uint64_t sec;
};

/**
 * @brief Udpate system time
 */
void system_time_update(void);

/**
 * @brief Get current system time
 */
const struct system_time * system_time_get(void);

#endif // SYSTEM_H
