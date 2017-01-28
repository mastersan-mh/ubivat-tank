/*
 * ent_exit.c
 *
 *  Created on: 9 дек. 2016 г.
 *      Author: mastersan
 */

#include "ent_exit.h"
#include "entity.h"
#include "model.h"

static entityvarinfo_t exit_vars[] =
{
		{ "message", ENTITYVARTYPE_STRING }
};

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
	ENTITY_VARIABLE_STRING(this, "message") = ENTITY_VARIABLE_STRING_DUP((char *)args);
}

static void exit_entity_done(entity_t * this, void * thisdata)
{
}

static const entityinfo_t exit_reginfo = {
		.name = "exit",
		.datasize = 0,
		ENTITYINFO_VARS(exit_vars),
		.init = exit_entity_init,
		.done = exit_entity_done,
		.handle   = ENTITY_FUNCTION_HANDLE_DEFAULT,
		.client_store = NULL,
		.client_restore = NULL,
		ENTITYINFO_ENTMODELS(exit_models)
};

void entity_exit_init(void)
{
	entity_register(&exit_reginfo);
}


