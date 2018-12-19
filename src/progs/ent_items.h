/*
 * ent_items.h
 *
 *  Created on: 3 дек. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_ENT_ITEMS_H_
#define SRC_ENT_ITEMS_H_

#include "progs_main.h"

entity_t * item_scores_spawn(entity_t * parent, const char * spawninfo);
entity_t * item_health_spawn(entity_t * parent, const char * spawninfo);
entity_t * item_armor_spawn(entity_t * parent, const char * spawninfo);
entity_t * item_ammo_missile_spawn(entity_t * parent, const char * spawninfo);
entity_t * item_ammo_mine_spawn(entity_t * parent, const char * spawninfo);

#endif /* SRC_ENT_ITEMS_H_ */
