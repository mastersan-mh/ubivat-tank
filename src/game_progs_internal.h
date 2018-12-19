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

#define ENTITIES_MAX 255

int game_progs_load(void);

const entity_action_t * game_progs_player_action_find(const char * action_str);



void game_progs_init(void);
void game_progs_done(void);
size_t game_progs_clients_max_get(void);
size_t game_progs_entities_max_get(void);

void game_progs_world_create(void);
void game_progs_world_destroy(void);
void game_progs_world_handle(void);
void game_progs_entity_on_read(const char *classname, const char * info);
bool game_progs_client_connect(const char * clientinfo);
entity_common_t * game_progs_client_player_spawn(const char * spawninfo);

entity_common_t * game_entity_find(size_t igclient);

#endif /* SRC_GAME_PROGS_INTERNAL_H_ */
