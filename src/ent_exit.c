/*
 * ent_exit.c
 *
 *  Created on: 9 дек. 2016 г.
 *      Author: mastersan
 */

#include "ent_exit.h"
#include "entity.h"
#include "model.h"

static entmodel_t exit_models[] =
{
		{
				.modelname = "exit",
				.modelscale = 8.0f,
				.translation = { 0.0, 0.0 }
		}
};

static ENTITY_FUNCTION_INIT(exit_entity_init)
{
	ent_exit_t * ent_exit = thisdata;
	ent_exit->message = Z_strdup((char *)args);
}

static void exit_entity_done(entity_t * this, void * thisdata)
{
	Z_free( ((ent_exit_t *)thisdata)->message );
}

static const entityinfo_t exit_reginfo = {
		.name = "exit",
		.datasize = sizeof(ent_exit_t),
		.init = exit_entity_init,
		.done = exit_entity_done,
		.handle   = ENTITY_FUNCTION_HANDLE_DEFAULT,
		.client_store = NULL,
		.client_restore = NULL,
		.entmodels_num = 1,
		.entmodels = exit_models
};

void entity_exit_init(void)
{
	entity_register(&exit_reginfo);
}


