/*
 * ent_spawn.c
 *
 *  Created on: 9 дек. 2016 г.
 *      Author: mastersan
 */

#include "progs.h"

#include "ent_spawn.h"
#include "ent_player.h"

ENTITY spawner_player_spawn(ENTITY parent, const char * spawninfo)
{
    ENTITY self = entity_spawn("spawner_player", parent);

    entity_bodybox_set(self, 16.0f);

    entity_vars_set_all(self, spawninfo);
    return self;
}

ENTITY spawner_enemy_spawn(ENTITY parent, const char * spawninfo)
{
    ENTITY self = entity_spawn("spawner_enemy", parent);
    entity_bodybox_set(self, 16.0f);
    entity_vars_set_all(self, spawninfo);
    spawn_vars_t * sp = entity_vars(self);

    ENTITY ent = enemy_spawn(self, NULL);
    player_vars_t * vars = entity_vars(ent);
    VEC2_COPY(vars->origin, sp->origin);
    vars->dir = sp->dir;
    return self;
}

ENTITY spawner_boss_spawn(ENTITY parent, const char * spawninfo)
{
    ENTITY self = entity_spawn("spawner_boss", parent);
    entity_bodybox_set(self, 16.0f);
    entity_vars_set_all(self, spawninfo);
    spawn_vars_t * sp = entity_vars(self);

    ENTITY ent = boss_spawn(self, NULL);
    player_vars_t * vars = entity_vars(ent);
    VEC2_COPY(vars->origin, sp->origin);
    vars->dir = sp->dir;
    return self;
}

ENTITY client_player_spawn(const char * spawninfo)
{
    ENTITY spawn = entity_get_random("spawner_player");
    return player_spawn(spawn, spawninfo);
}
