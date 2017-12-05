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

ENTITY spawner_player_spawn(ENTITY parent, const char * spawninfo);
ENTITY spawner_enemy_spawn(ENTITY parent, const char * spawninfo);
ENTITY spawner_boss_spawn(ENTITY parent, const char * spawninfo);

ENTITY client_player_spawn(const char * userinfo);

#endif /* SRC_ENT_SPAWN_H_ */
