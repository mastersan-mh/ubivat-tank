/*
 * game_progs_internal.h
 *
 *  Created on: 1 дек. 2017 г.
 *      Author: mastersan
 */

#ifndef SRC_GAME_PROGS_INTERNAL_H_
#define SRC_GAME_PROGS_INTERNAL_H_

#include "entity_internal.h"

#include "game_progs.h"

void game_progs_init();

const entity_action_t * game_player_action_find(const char * action_str);
bool game_client_player_connect(const char * clientinfo);
entity_t * game_client_player_spawn(const char * spawninfo);
void game_entity_on_read(const char *classname, const char * info);

#endif /* SRC_GAME_PROGS_INTERNAL_H_ */
