/*
 * ent_spawn.c
 *
 *  Created on: 9 дек. 2016 г.
 *      Author: mastersan
 */

#include "entity.h"
#include "entity_helpers.h"
#include "ent_spawn.h"
#include "ent_player.h"

static entityvarinfo_t spawn_player_vars[] =
{
		{ "item_scores", ENTITYVARTYPE_INTEGER },
		{ "item_health", ENTITYVARTYPE_INTEGER },
		{ "item_armor" , ENTITYVARTYPE_INTEGER },
		{ "item_ammo_missile", ENTITYVARTYPE_INTEGER },
		{ "item_ammo_mine"   , ENTITYVARTYPE_INTEGER },
};

static ENTITY_FUNCTION_INIT(spawn_player_init)
{
}

static ENTITY_FUNCTION_INIT(spawn_enemy_init)
{
	entity_new("enemy", this->pos.x, this->pos.y, this->dir, this, NULL);
}

static ENTITY_FUNCTION_INIT(spawn_boss_init)
{
	entity_new("boss", this->pos.x, this->pos.y, this->dir, this, NULL);
}

static ENTITY_FUNCTION_CLIENT_SPAWN(spawn_client_join)
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
		.datasize = 0,
		ENTITYINFO_VARS(spawn_player_vars),
		.init = spawn_player_init,
		.done = ENTITY_FUNCTION_NONE,
		.handle = ENTITY_FUNCTION_NONE,
		.client_join = spawn_client_join,
		.client_store = NULL,
		.client_restore = NULL
};

static const entityinfo_t spawn_enemy_reginfo = {
		.name = "spawn_enemy",
		.datasize = 0,
		ENTITYINFO_VARS(spawn_player_vars),
		.init = spawn_enemy_init,
		.done = ENTITY_FUNCTION_NONE,
		.handle = ENTITY_FUNCTION_NONE,
		.client_store = NULL,
		.client_restore = NULL
};

static const entityinfo_t spawn_boss_reginfo = {
		.name = "spawn_boss",
		.datasize = 0,
		ENTITYINFO_VARS(spawn_player_vars),
		.init = spawn_boss_init,
		.done = ENTITY_FUNCTION_NONE,
		.handle = ENTITY_FUNCTION_NONE,
		.client_store = NULL,
		.client_restore = NULL
};

void entity_spawn_init(void)
{
	entity_register(&spawn_player_reginfo);
	entity_register(&spawn_enemy_reginfo);
	entity_register(&spawn_boss_reginfo);
}
