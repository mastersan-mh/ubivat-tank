/*
 * entity_helpers.h
 *
 *  Created on: 9 янв. 2017 г.
 *      Author: mastersan
 */

#ifndef SRC_ENTITY_HELPERS_H_
#define SRC_ENTITY_HELPERS_H_
#include "entity.h"

extern ENTITY entity_get_random(const char * entityname);
extern direction_t entity_direction_invert(direction_t dir);
extern void entity_move(ENTITY entity, direction_t dir, vec_t speed, bool check_clip);

#endif /* SRC_ENTITY_HELPERS_H_ */
