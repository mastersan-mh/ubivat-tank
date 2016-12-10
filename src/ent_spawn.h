/*
 * ent_spawn.h
 *
 *  Created on: 9 дек. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_ENT_SPAWN_H_
#define SRC_ENT_SPAWN_H_

/*
 * точка респавнинга
 */
typedef struct
{
	int items[__ITEM_NUM];
} spawn_t;

void mobj_spawn_init();

#endif /* SRC_ENT_SPAWN_H_ */
