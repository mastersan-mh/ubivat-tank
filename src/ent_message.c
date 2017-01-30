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
		{ "text", ENTITYVARTYPE_STRING }
};

static const entityinfo_t message_reginfo = {
		.name = "message",
		.datasize = 0,
		.bodybox = 16.0f,
		ENTITYINFO_VARS(message_vars),
		.init = ENTITY_FUNCTION_NONE,
		.done = ENTITY_FUNCTION_NONE,
		.handle = ENTITY_FUNCTION_NONE,
		.client_store = NULL,
		.client_restore = NULL
};

void entity_message_init(void)
{
	entity_register(&message_reginfo);
}
