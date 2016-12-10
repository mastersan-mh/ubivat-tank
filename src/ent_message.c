/*
 * ent_message.c
 *
 *  Created on: 9 дек. 2016 г.
 *      Author: mastersan
 */

#include "mobjs.h"
#include "game.h"
#include "ent_message.h"

static MOBJ_FUNCTION_INIT(message_mobj_init)
{
	this->img = NULL;
	ent_message_t * ent_message = thisdata;
	ent_message->message = Z_strdup((char *)args);
}

static void message_mobj_done(mobj_t * this, void * thisdata)
{
	Z_free( ((ent_message_t *) thisdata)->message );
}

static const mobj_reginfo_t message_reginfo = {
		.name = "message",
		.datasize = sizeof(ent_message_t),
		.mobjinit = message_mobj_init,
		.mobjdone = message_mobj_done,
		.handle   = MOBJ_FUNCTION_HANDLE_DEFAULT,
		.client_store = NULL,
		.client_restore = NULL
};

void mobj_message_init()
{
	mobj_register(&message_reginfo);
}
