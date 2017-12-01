/*
 * ent_exit.c
 *
 *  Created on: 9 дек. 2016 г.
 *      Author: mastersan
 */

#include "progs.h"

#include "ent_exit.h"

static var_descr_t exit_vars[] =
{
        ENTITY_VARS_COMMON,
        VAR_DESCR( VARTYPE_STRING, entity_exit_t, text)
};


static ENTITY_FUNCTION_INIT(exit_init)
{
    entity_bodybox_set(self, 16.0f);
    entity_model_set(self, 0, ":/exit", 8.0f, 0.0f, 0.0f);
}

static const entityinfo_t exit_reginfo = {
		.name_ = "exit",
		ENTITYINFO_VARS(entity_exit_t, exit_vars),
		.models_num = 1,
		.init = exit_init,
		.done = ENTITY_FUNCTION_NONE,
		.handle = ENTITY_FUNCTION_NONE,
};

void entity_exit_init(void)
{
	entity_register(&exit_reginfo);
}


