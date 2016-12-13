/*
 * ent_spawn.c
 *
 *  Created on: 9 дек. 2016 г.
 *      Author: mastersan
 */

#include "mobjs.h"
#include "game.h"
#include "ent_spawn.h"

static void spawn_common_init(mobj_t * this, void * thisdata, const mobj_t * parent, const spawn_t * args)
{
	this->img = NULL;
	int i;
	for(i = 0; i < __ITEM_NUM; i++) ((spawn_t *)thisdata)->items[i] = args->items[i];
}

static MOBJ_FUNCTION_INIT(spawn_player_mobj_init)
{
	spawn_common_init(this, thisdata, parent, args);
}

static MOBJ_FUNCTION_INIT(spawn_enemy_mobj_init)
{
	spawn_common_init(this, thisdata, parent, args);
	mobj_new(MOBJ_ENEMY, this->pos.x, this->pos.y, this->dir, this, NULL);
}

static MOBJ_FUNCTION_INIT(spawn_boss_mobj_init)
{
	spawn_common_init(this, thisdata, parent, args);
	mobj_new(MOBJ_BOSS, this->pos.x, this->pos.y, this->dir, this, NULL);
}

static const mobj_reginfo_t spawn_player_reginfo = {
		.name = "spawn_player",
		.datasize = sizeof(spawn_t),
		.mobjinit = spawn_player_mobj_init,
		.mobjdone = MOBJ_FUNCTION_DONE_DEFAULT,
		.handle   = MOBJ_FUNCTION_HANDLE_DEFAULT,
		.client_store = NULL,
		.client_restore = NULL
};

static const mobj_reginfo_t spawn_enemy_reginfo = {
		.name = "spawn_enemy",
		.datasize = sizeof(spawn_t),
		.mobjinit = spawn_enemy_mobj_init,
		.mobjdone = MOBJ_FUNCTION_DONE_DEFAULT,
		.handle   = MOBJ_FUNCTION_HANDLE_DEFAULT,
		.client_store = NULL,
		.client_restore = NULL
};

static const mobj_reginfo_t spawn_boss_reginfo = {
		.name = "spawn_boss",
		.datasize = sizeof(spawn_t),
		.mobjinit = spawn_boss_mobj_init,
		.mobjdone = MOBJ_FUNCTION_DONE_DEFAULT,
		.handle   = MOBJ_FUNCTION_HANDLE_DEFAULT,
		.client_store = NULL,
		.client_restore = NULL
};

void mobj_spawn_init()
{
	mobj_register(&spawn_player_reginfo);
	mobj_register(&spawn_enemy_reginfo);
	mobj_register(&spawn_boss_reginfo);
}
