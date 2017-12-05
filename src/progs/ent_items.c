/*
 * items.c
 *
 *  Created on: 10 дек. 2016 г.
 *      Author: mastersan
 */

#include "progs.h"

#include "ent_items.h"

#include <string.h>

ENTITY item_scores_spawn(ENTITY parent, const char * spawninfo)
{
    ENTITY self = entity_spawn("item_scores", parent);

    entity_bodybox_set(self, 16.0f);
    entity_model_set(self, 0, ":/item_scores", 8.0f, 0.0f, 0.0f);

    entity_vars_set_all(self, spawninfo);
    return self;
}

ENTITY item_health_spawn(ENTITY parent, const char * spawninfo)
{
    ENTITY self = entity_spawn("item_health", parent);
    if(!self)
        return NULL;
    entity_bodybox_set(self, 16.0f);
    entity_model_set(self, 0, ":/item_health", 8.0f, 0.0f, 0.0f);

    entity_vars_set_all(self, spawninfo);
    return self;
}

ENTITY item_armor_spawn(ENTITY parent, const char * spawninfo)
{
    ENTITY self = entity_spawn("item_armor", parent);

    entity_bodybox_set(self, 16.0f);
    entity_model_set(self, 0, ":/item_armor", 8.0f, 0.0f, 0.0f);

    entity_vars_set_all(self, spawninfo);
    return self;
}

ENTITY item_ammo_missile_spawn(ENTITY parent, const char * spawninfo)
{
    ENTITY self = entity_spawn("item_ammo_missile", parent);

    entity_bodybox_set(self, 16.0f);
    entity_model_set(self, 0, ":/item_ammo_missile", 8.0f, 0.0f, 0.0f);

    entity_vars_set_all(self, spawninfo);
    return self;
}

ENTITY item_ammo_mine_spawn(ENTITY parent, const char * spawninfo)
{
    ENTITY self = entity_spawn("item_ammo_mine", parent);

    entity_bodybox_set(self, 16.0f);
    entity_model_set(self, 0, ":/item_ammo_mine", 8.0f, 0.0f, 0.0f);

    entity_vars_set_all(self, spawninfo);
    return self;
}
