/*
 * ent_message.c
 *
 *  Created on: 9 дек. 2016 г.
 *      Author: mastersan
 */

#include "progs.h"

#include "ent_message.h"

static var_descr_t message_vars[] =
{
        ENTITY_VARS_COMMON,
        VAR_DESCR( VARTYPE_STRING, entity_message_t, text )
};

static ENTITY_FUNCTION_INIT(message_init)
{
    entity_bodybox_set(this, 16.0f);
}

static const entityinfo_t message_reginfo = {
        .name_ = "message",
        ENTITYINFO_VARS(entity_message_t, message_vars),
        .init = message_init,
        .done = ENTITY_FUNCTION_NONE,
        .handle = ENTITY_FUNCTION_NONE,
};

void entity_message_init(void)
{
    entity_register(&message_reginfo);
}
