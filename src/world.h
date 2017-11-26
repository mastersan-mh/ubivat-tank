/*
 * world.h
 *
 *  Created on: 23 нояб. 2017 г.
 *      Author: mastersan
 */

#ifndef SRC_WORLD_H_
#define SRC_WORLD_H_

#include "map.h"

typedef struct
{
    char * mapfilename;
    map_t * map;
} world_t;

extern int world_create(const char * mapfilename);
extern void world_destroy(void);
extern const char * world_mapfilename_get(void);
extern map_t * world_map_get(void);
extern void world_handle(void);

#endif /* SRC_WORLD_H_ */
