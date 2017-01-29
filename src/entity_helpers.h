/*
 * entity_helpers.h
 *
 *  Created on: 9 янв. 2017 г.
 *      Author: mastersan
 */

#ifndef SRC_ENTITY_HELPERS_H_
#define SRC_ENTITY_HELPERS_H_

#include "entity.h"

entity_t * entity_get_random(const char * entityname);
vec_t entity_move(entity_t * this, direction_t dir, vec_t bodybox, vec_t speed);

#endif /* SRC_ENTITY_HELPERS_H_ */
