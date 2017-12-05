/*
 * entity_helpers.h
 *
 *  Created on: 9 янв. 2017 г.
 *      Author: mastersan
 */

#ifndef SRC_ENTITY_HELPERS_H_
#define SRC_ENTITY_HELPERS_H_
#include "progs.h"

#define MIN(a, b) ((a)<(b) ? (a) : (b))

void coerce_value_int(INTEGER * val, INTEGER min, INTEGER max);

#ifdef HAVE_STRLCPY
error We have strlcpy(), check the code before use it.
#else
size_t strlcpy(char *__restrict dest, const char *__restrict src, size_t n);
#endif

char * s_snprintf(char * str, size_t n, const char *format, ...);
void entity_vars_set_all(ENTITY entity, const char * info);
ENTITY entity_get_random(const char * entityname);
direction_t entity_direction_invert(direction_t dir);
void entity_move(ENTITY entity, direction_t dir, vec_t speed, bool check_clip);

#endif /* SRC_ENTITY_HELPERS_H_ */
