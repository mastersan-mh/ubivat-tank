/*
 * ent_spawn.h
 *
 * точка респавнинга
 *
 *  Created on: 9 дек. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_ENT_SPAWN_H_
#define SRC_ENT_SPAWN_H_

#include "progs.h"

typedef struct
{
    ENTITY_VARS_COMMON_STRUCT;
    INTEGER item_scores;
    INTEGER item_health;
    INTEGER item_armor ;
    INTEGER item_ammo_missile;
    INTEGER item_ammo_mine   ;
} spawn_vars_t;

extern void entity_spawn_init(void);

#endif /* SRC_ENT_SPAWN_H_ */
