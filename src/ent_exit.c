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
		{ "text", ENTITYVARTYPE_STRING }
};

static entitymodel_t exit_models[] =
{
		{
				.modelname = "exit",
				.modelscale = 8.0f,
				.translation = { 0.0, 0.0 }
		}
};

static const entityinfo_t exit_reginfo = {
		.name = "exit",
		.datasize = 0,
		ENTITYINFO_VARS(exit_vars),
		ENTITYINFO_ENTMODELS(exit_models),
		.init = ENTITY_FUNCTION_NONE,
		.done = ENTITY_FUNCTION_NONE,
		.handle = ENTITY_FUNCTION_NONE,
		.client_store = NULL,
		.client_restore = NULL,
};

void entity_exit_init(void)
{
	entity_register(&exit_reginfo);
}


