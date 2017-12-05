/*
 * ent_items.h
 *
 *  Created on: 3 дек. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_ENT_ITEMS_H_
#define SRC_ENT_ITEMS_H_

#include "progs_main.h"

ENTITY item_scores_spawn(ENTITY parent, const char * spawninfo);
ENTITY item_health_spawn(ENTITY parent, const char * spawninfo);
ENTITY item_armor_spawn(ENTITY parent, const char * spawninfo);
ENTITY item_ammo_missile_spawn(ENTITY parent, const char * spawninfo);
ENTITY item_ammo_mine_spawn(ENTITY parent, const char * spawninfo);

#endif /* SRC_ENT_ITEMS_H_ */
