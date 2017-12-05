/*
 * ent_message.c
 *
 *  Created on: 9 дек. 2016 г.
 *      Author: mastersan
 */

#include "progs.h"

#include "ent_message.h"

ENTITY message_spawn(ENTITY parent, const char * spawninfo)
{
    ENTITY self = entity_spawn("message", parent);

    entity_bodybox_set(self, 16.0f);

    entity_vars_set_all(self, spawninfo);
    return self;
}
