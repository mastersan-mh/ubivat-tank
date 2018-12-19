/*
 * ent_spawn.c
 *
 *  Created on: 9 дек. 2016 г.
 *      Author: mastersan
 */

#include "progs.h"
#include "helpers.h"

#include "ent_spawn.h"
#include "ent_player.h"

entity_t * spawner_player_spawn(entity_t * parent, const char * spawninfo)
{
    entity_t * self = entity_init("spawner_player", parent);

    self->c.bodybox = 16.0f;

    entity_vars_set_all(self, spawninfo);
    return self;
}

entity_t * spawner_enemy_spawn(entity_t * parent, const char * spawninfo)
{
    entity_t * self = entity_init("spawner_enemy", parent);
    self->c.bodybox = 16.0f;
    entity_vars_set_all(self, spawninfo);

    entity_t * enemy = enemy_spawn(self, NULL);
    VEC2_COPY(enemy->c.origin, self->c.origin);
    enemy->c.dir = self->c.dir;
    return self;
}

entity_t * spawner_boss_spawn(entity_t * parent, const char * spawninfo)
{
    entity_t * self = entity_init("spawner_boss", parent);
    self->c.bodybox = 16.0f;
    entity_vars_set_all(self, spawninfo);

    entity_t * boss = boss_spawn(self, NULL);
    VEC2_COPY(boss->c.origin, self->c.origin);
    boss->c.dir = self->c.dir;
    return self;
}

entity_common_t * client_player_spawn(const char * spawninfo)
{
    entity_t * spawn = entity_get_random("spawner_player");
    entity_t * player = player_spawn(spawn, spawninfo);
    return (entity_common_t*)player;
}
