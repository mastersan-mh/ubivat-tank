/*
 * items.c
 *
 *  Created on: 10 дек. 2016 г.
 *      Author: mastersan
 */

#include "progs.h"
#include "helpers.h"

#include "ent_items.h"

#include <string.h>

entity_t * item_scores_spawn(entity_t * parent, const char * spawninfo)
{
    entity_t * self = entity_init("item_scores", parent);

    self->c.bodybox = 16.0f;
    entity_model_set(self, 0, ":/item_scores", 8.0f, 0.0f, 0.0f);

    entity_vars_set_all(self, spawninfo);
    return self;
}

entity_t * item_health_spawn(entity_t * parent, const char * spawninfo)
{
    entity_t * self = entity_init("item_health", parent);
    if(!self)
        return NULL;
    self->c.bodybox = 16.0f;
    entity_model_set(self, 0, ":/item_health", 8.0f, 0.0f, 0.0f);

    entity_vars_set_all(self, spawninfo);
    return self;
}

entity_t * item_armor_spawn(entity_t * parent, const char * spawninfo)
{
    entity_t * self = entity_init("item_armor", parent);

    self->c.bodybox = 16.0f;
    entity_model_set(self, 0, ":/item_armor", 8.0f, 0.0f, 0.0f);

    entity_vars_set_all(self, spawninfo);
    return self;
}

entity_t * item_ammo_missile_spawn(entity_t * parent, const char * spawninfo)
{
    entity_t * self = entity_init("item_ammo_missile", parent);

    self->c.bodybox = 16.0f;
    entity_model_set(self, 0, ":/item_ammo_missile", 8.0f, 0.0f, 0.0f);

    entity_vars_set_all(self, spawninfo);
    return self;
}

entity_t * item_ammo_mine_spawn(entity_t * parent, const char * spawninfo)
{
    entity_t * self = entity_init("item_ammo_mine", parent);

    self->c.bodybox = 16.0f;
    entity_model_set(self, 0, ":/item_ammo_mine", 8.0f, 0.0f, 0.0f);

    entity_vars_set_all(self, spawninfo);
    return self;
}
