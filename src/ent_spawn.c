/*
 * ent_spawn.c
 *
 *  Created on: 9 дек. 2016 г.
 *      Author: mastersan
 */

#include "game.h"
#include "system.h"
#include "entity.h"
#include "ent_spawn.h"

/**
 * получить любой спавн-поинт для игрока
 */
static entity_t * spawn_get_random()
{

	int count = 0;

	entity_t * ent;

	//считаем количество спавн-поинтов
	FOR_ENTITIES("spawn_player", ent)
	{
		count++;
	};
	if(count == 0)
		return NULL;
	count = xrand(count);

	//выбираем случайным образом
	FOR_ENTITIES("spawn_player", ent)
	{
		if(count == 0) return ent;
		count--;
	};
	return NULL;
}


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

static ENTITY_FUNCTION_CLIENT_SPAWN(spawn_player_spawn)
{
	entity_t * spawn = spawn_get_random();

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
		.done = MOBJ_FUNCTION_DONE_DEFAULT,
		.handle = MOBJ_FUNCTION_HANDLE_DEFAULT,
		.client_spawn = spawn_player_spawn,
		.client_store = NULL,
		.client_restore = NULL
};

static const entityinfo_t spawn_enemy_reginfo = {
		.name = "spawn_enemy",
		.datasize = sizeof(spawn_t),
		.init = spawn_enemy_init,
		.done = MOBJ_FUNCTION_DONE_DEFAULT,
		.handle   = MOBJ_FUNCTION_HANDLE_DEFAULT,
		.client_store = NULL,
		.client_restore = NULL
};

static const entityinfo_t spawn_boss_reginfo = {
		.name = "spawn_boss",
		.datasize = sizeof(spawn_t),
		.init = spawn_boss_init,
		.done = MOBJ_FUNCTION_DONE_DEFAULT,
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
