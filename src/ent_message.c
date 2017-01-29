/*
 * ent_message.c
 *
 *  Created on: 9 дек. 2016 г.
 *      Author: mastersan
 */

#include "entity.h"
#include "game.h"
#include "ent_message.h"

static entityvarinfo_t message_vars[] =
{
		{ "message", ENTITYVARTYPE_STRING }
};

static ENTITY_FUNCTION_INIT(message_init)
{
	ENTITY_VARIABLE_STRING(this, "message") = ENTITY_VARIABLE_STRING_DUP((char *)args);
}

static void message_done(entity_t * this, void * thisdata)
{
}

static const entityinfo_t message_reginfo = {
		.name = "message",
		.datasize = 0,
		ENTITYINFO_VARS(message_vars),
		.init = message_init,
		.done = message_done,
		.handle = ENTITY_FUNCTION_NONE,
		.client_store = NULL,
		.client_restore = NULL
};

void entity_message_init(void)
{
	entity_register(&message_reginfo);
}
