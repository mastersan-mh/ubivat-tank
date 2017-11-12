/*
 * ent_message.c
 *
 *  Created on: 9 дек. 2016 г.
 *      Author: mastersan
 */

#include "entity.h"
#include "ent_message.h"

static var_descr_t message_vars[] =
{
        ENTITY_COMMON_VARS,
        VAR_DESCR( VARTYPE_STRING, entity_message_t, text )
};

static const entityinfo_t message_reginfo = {
		.name = "message",
		.bodybox = 16.0f,
		ENTITYINFO_VARS(entity_message_t, message_vars),
		.init = ENTITY_FUNCTION_NONE,
		.done = ENTITY_FUNCTION_NONE,
		.handle = ENTITY_FUNCTION_NONE,
};

void entity_message_init(void)
{
	entity_register(&message_reginfo);
}
