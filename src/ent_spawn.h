/*
 * ent_spawn.h
 *
 *  Created on: 9 дек. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_ENT_SPAWN_H_
#define SRC_ENT_SPAWN_H_

#include "ent_items.h"
/*
 * точка респавнинга
 */
typedef struct
{
	int items[__ITEM_NUM];
} spawn_t;

void entity_spawn_init();

#endif /* SRC_ENT_SPAWN_H_ */
