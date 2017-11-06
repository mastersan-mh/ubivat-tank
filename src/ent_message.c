/*
 * ent_message.c
 *
 *  Created on: 9 дек. 2016 г.
 *      Author: mastersan
 */

#include "entity.h"
#include "ent_message.h"

static entityvarinfo_t message_vars[] =
{
		{ VARTYPE_STRING, "text" }
};

static const entityinfo_t message_reginfo = {
		.name = "message",
		.edatasize = 0,
		.bodybox = 16.0f,
		ENTITYINFO_VARS(message_vars),
		.init = ENTITY_FUNCTION_NONE,
		.done = ENTITY_FUNCTION_NONE,
		.handle = ENTITY_FUNCTION_NONE,
		.player_store = NULL,
		.player_restore = NULL
};

void entity_message_init(void)
{
	entity_register(&message_reginfo);
}
