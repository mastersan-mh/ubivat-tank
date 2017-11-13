/*
 * ent_exit.c
 *
 *  Created on: 9 дек. 2016 г.
 *      Author: mastersan
 */

#include "ent_exit.h"
#include "entity.h"
#include "model.h"

static var_descr_t exit_vars[] =
{
        ENTITY_VARS_COMMON,
        VAR_DESCR( VARTYPE_STRING, entity_exit_t, text)
};

static entity_model_t exit_models[] =
{
		{
				.modelname = "exit",
				.modelscale = 8.0f,
				.translation = { 0.0, 0.0 }
		}
};

static const entityinfo_t exit_reginfo = {
		.name = "exit",
		.bodybox = 16.0f,
		ENTITYINFO_VARS(entity_exit_t, exit_vars),
		ENTITYINFO_ENTMODELS(exit_models),
		.init = ENTITY_FUNCTION_NONE,
		.done = ENTITY_FUNCTION_NONE,
		.handle = ENTITY_FUNCTION_NONE,
};

void entity_exit_init(void)
{
	entity_register(&exit_reginfo);
}


