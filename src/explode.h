/*
 * explode.h
 *
 *  Created on: 29 нояб. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_EXPLODE_H_
#define SRC_EXPLODE_H_

#include "player.h"

typedef struct
{
	//повреждение
	int damage;
	//повреждение
	int selfdamage;
	//радиус действия
	coord_t radius;
} explodeinfo_t;

extern explodeinfo_t explodeinfo_table[__EXPLODE_NUM];

mobj_t * explode_new(coord_t x, coord_t y, mobj_explodetype_t explode_type, player_t * owner);

void explode_handle(mobj_t * mobj);

void explode_draw(camera_t * cam, mobj_t * explode);

#endif /* SRC_EXPLODE_H_ */
