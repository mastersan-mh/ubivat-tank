/*
 * ent_spawn.c
 *
 *  Created on: 9 дек. 2016 г.
 *      Author: mastersan
 */

#include "mobjs.h"
#include "game.h"
#include "ent_spawn.h"

static void spawn_common_init(entity_t * this, void * thisdata, const entity_t * parent, const spawn_t * args)
{
	int i;
	for(i = 0; i < __ITEM_NUM; i++) ((spawn_t *)thisdata)->items[i] = args->items[i];
}

static MOBJ_FUNCTION_INIT(spawn_player_init)
{
	spawn_common_init(this, thisdata, parent, args);
}

static MOBJ_FUNCTION_INIT(spawn_enemy_init)
{
	spawn_common_init(this, thisdata, parent, args);
	entity_new("enemy", this->pos.x, this->pos.y, this->dir, this, NULL);
}

static MOBJ_FUNCTION_INIT(spawn_boss_init)
{
	spawn_common_init(this, thisdata, parent, args);
	entity_new("boss", this->pos.x, this->pos.y, this->dir, this, NULL);
}

static const entityinfo_t spawn_player_reginfo = {
		.name = "spawn_player",
		.datasize = sizeof(spawn_t),
		.entityinit = spawn_player_init,
		.entitydone = MOBJ_FUNCTION_DONE_DEFAULT,
		.handle   = MOBJ_FUNCTION_HANDLE_DEFAULT,
		.client_store = NULL,
		.client_restore = NULL
};

static const entityinfo_t spawn_enemy_reginfo = {
		.name = "spawn_enemy",
		.datasize = sizeof(spawn_t),
		.entityinit = spawn_enemy_init,
		.entitydone = MOBJ_FUNCTION_DONE_DEFAULT,
		.handle   = MOBJ_FUNCTION_HANDLE_DEFAULT,
		.client_store = NULL,
		.client_restore = NULL
};

static const entityinfo_t spawn_boss_reginfo = {
		.name = "spawn_boss",
		.datasize = sizeof(spawn_t),
		.entityinit = spawn_boss_init,
		.entitydone = MOBJ_FUNCTION_DONE_DEFAULT,
		.handle   = MOBJ_FUNCTION_HANDLE_DEFAULT,
		.client_store = NULL,
		.client_restore = NULL
};

void entity_spawn_init()
{
	entity_register(&spawn_player_reginfo);
	entity_register(&spawn_enemy_reginfo);
	entity_register(&spawn_boss_reginfo);
}
