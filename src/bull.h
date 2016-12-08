/*
 * bull.h
 *
 *  Created on: 30 нояб. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_BULL_H_
#define SRC_BULL_H_

#include "player.h"

typedef struct
{
	//повреждение
	int damage;
	//повреждение
	int selfdamage;
	//дальность
	vec_t range;
	//начальная скорость пули
	vec_t speed;
	//bodybox
	vec_t bodybox;
	//изображение оружия
	image_index_t icon;
} bullinfo_t;

extern bullinfo_t bullinfo_table[__BULL_NUM];

mobj_t * bull_new(vec_t x, vec_t y, mobj_bulltype_t bulltype, direction_t dir, mobj_t * owner);

void bull_handle(mobj_t * mobj);

void bull_draw(camera_t * cam, mobj_t * bull);

#endif /* SRC_BULL_H_ */
