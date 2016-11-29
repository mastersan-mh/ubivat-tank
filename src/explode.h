/*
 * explode.h
 *
 *  Created on: 29 нояб. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_EXPLODE_H_
#define SRC_EXPLODE_H_

#include "player.h"

void explode_handle(mobj_t * mobj);

void explode_draw(camera_t * cam, mobj_t * explode);

void explode_new(coord_t x, coord_t y, mobj_explode_type_t explode_type, player_t * owner);


#endif /* SRC_EXPLODE_H_ */
