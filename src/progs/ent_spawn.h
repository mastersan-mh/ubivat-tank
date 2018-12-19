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
#include "progs_main.h"

entity_t * spawner_player_spawn(entity_t * parent, const char * spawninfo);
entity_t * spawner_enemy_spawn(entity_t * parent, const char * spawninfo);
entity_t * spawner_boss_spawn(entity_t * parent, const char * spawninfo);

entity_common_t * client_player_spawn(const char * userinfo);

#endif /* SRC_ENT_SPAWN_H_ */
