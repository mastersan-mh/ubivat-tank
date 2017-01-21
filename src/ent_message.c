/*
 * ent_message.c
 *
 *  Created on: 9 дек. 2016 г.
 *      Author: mastersan
 */

#include "entity.h"
#include "game.h"
#include "ent_message.h"

static ENTITY_FUNCTION_INIT(message_init)
{
	((ent_message_t *)thisdata)->message = Z_strdup((char *)args);
}

static void message_done(entity_t * this, void * thisdata)
{
	Z_free( ((ent_message_t *) thisdata)->message );
}

static const entityinfo_t message_reginfo = {
		.name = "message",
		.datasize = sizeof(ent_message_t),
		.init = message_init,
		.done = message_done,
		.handle   = ENTITY_FUNCTION_HANDLE_DEFAULT,
		.client_store = NULL,
		.client_restore = NULL
};

void entity_message_init(void)
{
	entity_register(&message_reginfo);
}
