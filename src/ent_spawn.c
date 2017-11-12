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

static var_descr_t spawn_player_vars[] =
{
        ENTITY_COMMON_VARS,
        VAR_DESCR( VARTYPE_INTEGER, spawn_vars_t, item_scores ),
        VAR_DESCR( VARTYPE_INTEGER, spawn_vars_t, item_health ),
        VAR_DESCR( VARTYPE_INTEGER, spawn_vars_t, item_armor  ),
        VAR_DESCR( VARTYPE_INTEGER, spawn_vars_t, item_ammo_missile ),
        VAR_DESCR( VARTYPE_INTEGER, spawn_vars_t, item_ammo_mine    ),
};

static ENTITY_FUNCTION_INIT(spawn_player_init)
{
}

static ENTITY_FUNCTION_INIT(spawn_enemy_init)
{
    spawn_vars_t * sp = entity_vars(this);
    void * ent = entity_new("enemy", this);
    player_vars_t * vars = entity_vars(ent);
    VEC2_COPY(vars->origin, sp->origin);
    vars->dir = sp->dir;
}

static ENTITY_FUNCTION_INIT(spawn_boss_init)
{
    spawn_vars_t * sp = entity_vars(this);
    void * ent = entity_new("boss", this);
    player_vars_t * vars = entity_vars(ent);
    VEC2_COPY(vars->origin, sp->origin);
    vars->dir = sp->dir;
}

static ENTITY_FUNCTION_PLAYER_SPAWN(spawn_player_spawn)
{
    ENTITY spawn = entity_get_random("spawn_player");

    ENTITY player = entity_new("player", spawn);

    if(!player)
        return NULL;

    entity_restore(player, storage);

    /*
    spawn_vars_t * sp = entity_vars(spawn);
    player_vars_t * vars = entity_vars(player);
    VEC2_COPY(vars->origin, sp->origin);
    vars->dir = sp->dir;
    vars->scores = sp->item_scores;
    vars->item_health = sp->item_health;
    vars->item_armor = sp->item_armor;
    vars->item_ammo_missile = sp->item_ammo_missile;
    vars->item_ammo_mine = sp->item_ammo_mine;
*/
    player_spawn_init(player , spawn);

    return player;
}

static const entityinfo_t spawn_player_reginfo = {
		.name = "spawn_player",
		.bodybox = 16.0f,
		ENTITYINFO_VARS(spawn_vars_t, spawn_player_vars),
		.init = spawn_player_init,
		.done = ENTITY_FUNCTION_NONE,
		.handle = ENTITY_FUNCTION_NONE,
		.player_spawn = spawn_player_spawn,
};

static const entityinfo_t spawn_enemy_reginfo = {
		.name = "spawn_enemy",
		.bodybox = 16.0f,
		ENTITYINFO_VARS(spawn_vars_t, spawn_player_vars),
		.init = spawn_enemy_init,
		.done = ENTITY_FUNCTION_NONE,
		.handle = ENTITY_FUNCTION_NONE,
};

static const entityinfo_t spawn_boss_reginfo = {
		.name = "spawn_boss",
		.bodybox = 16.0f,
		ENTITYINFO_VARS(spawn_vars_t, spawn_player_vars),
		.init = spawn_boss_init,
		.done = ENTITY_FUNCTION_NONE,
		.handle = ENTITY_FUNCTION_NONE,
};

void entity_spawn_init(void)
{
	entity_register(&spawn_player_reginfo);
	entity_register(&spawn_enemy_reginfo);
	entity_register(&spawn_boss_reginfo);
}
