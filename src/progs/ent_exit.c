/*
 * ent_exit.c
 *
 *  Created on: 9 дек. 2016 г.
 *      Author: mastersan
 */

#include "progs.h"

#include "ent_exit.h"

ENTITY exit_spawn(ENTITY parent, const char * spawninfo)
{
    ENTITY self = entity_spawn("exit", parent);

    entity_bodybox_set(self, 16.0f);
    entity_model_set(self, 0, ":/exit", 8.0f, 0.0f, 0.0f);

    entity_vars_set_all(self, spawninfo);
    return self;
}
