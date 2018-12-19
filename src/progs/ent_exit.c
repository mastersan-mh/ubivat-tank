/*
 * ent_exit.c
 *
 *  Created on: 9 дек. 2016 г.
 *      Author: mastersan
 */

#include "progs.h"
#include "helpers.h"

#include "ent_exit.h"

entity_t * exit_spawn(entity_t * parent, const char * spawninfo)
{
    entity_t * self = entity_init("exit", parent);

    self->c.bodybox = 16.0f;
    entity_model_set(self, 0, ":/exit", 8.0f, 0.0f, 0.0f);

    entity_vars_set_all(self, spawninfo);
    return self;
}
