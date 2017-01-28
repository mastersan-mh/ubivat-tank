/*
 * ent_spawn.c
 *
 *  Created on: 9 дек. 2016 г.
 *      Author: mastersan
 */

#include "game.h"
#include "system.h"
#include "entity.h"
#include "entity_helpers.h"
#include "ent_spawn.h"

static void spawn_common_init(entity_t * this, void * thisdata, const entity_t * parent, const spawn_t * args)
{
	size_t i;
	for(i = 0; i < ITEM_NUM; i++)
		((spawn_t *)thisdata)->items[i] = args->items[i];
}

static ENTITY_FUNCTION_INIT(spawn_player_init)
{
	spawn_common_init(this, thisdata, parent, args);
}

static ENTITY_FUNCTION_INIT(spawn_enemy_init)
{
	spawn_common_init(this, thisdata, parent, args);
	entity_new("enemy", this->pos.x, this->pos.y, this->dir, this, NULL);
}

static ENTITY_FUNCTION_INIT(spawn_boss_init)
{
	spawn_common_init(this, thisdata, parent, args);
	entity_new("boss", this->pos.x, this->pos.y, this->dir, this, NULL);
}

static ENTITY_FUNCTION_CLIENT_SPAWN(spawn_player_spawn)
{
	entity_t * spawn = entity_get_random("spawn_player");

	entity_t * player = entity_new(
		"player",
		spawn->pos.x,
		spawn->pos.y,
		spawn->dir,
		spawn,
		NULL
	);
	return player;
}

static const entityinfo_t spawn_player_reginfo = {
		.name = "spawn_player",
		.datasize = sizeof(spawn_t),
		.init = spawn_player_init,
		.done = ENTITY_FUNCTION_DONE_DEFAULT,
		.handle = ENTITY_FUNCTION_HANDLE_DEFAULT,
		.client_join = spawn_player_spawn,
		.client_store = NULL,
		.client_restore = NULL
};

static const entityinfo_t spawn_enemy_reginfo = {
		.name = "spawn_enemy",
		.datasize = sizeof(spawn_t),
		.init = spawn_enemy_init,
		.done = ENTITY_FUNCTION_DONE_DEFAULT,
		.handle   = ENTITY_FUNCTION_HANDLE_DEFAULT,
		.client_store = NULL,
		.client_restore = NULL
};

static const entityinfo_t spawn_boss_reginfo = {
		.name = "spawn_boss",
		.datasize = sizeof(spawn_t),
		.init = spawn_boss_init,
		.done = ENTITY_FUNCTION_DONE_DEFAULT,
		.handle   = ENTITY_FUNCTION_HANDLE_DEFAULT,
		.client_store = NULL,
		.client_restore = NULL
};

void entity_spawn_init(void)
{
	entity_register(&spawn_player_reginfo);
	entity_register(&spawn_enemy_reginfo);
	entity_register(&spawn_boss_reginfo);
}
