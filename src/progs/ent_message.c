/*
 * ent_message.c
 *
 *  Created on: 9 дек. 2016 г.
 *      Author: mastersan
 */

#include "progs.h"
#include "helpers.h"

#include "ent_message.h"

entity_t * message_spawn(entity_t * parent, const char * spawninfo)
{
    entity_t * self = entity_init("message", parent);

    self->c.bodybox = 16.0f;

    entity_vars_set_all(self, spawninfo);
    return self;
}
