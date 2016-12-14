/*
 * ent_exit.c
 *
 *  Created on: 9 дек. 2016 г.
 *      Author: mastersan
 */

#include "ent_exit.h"
#include "mobjs.h"
#include "model.h"

static entmodel_t exit_models[] =
{
		{
				.modelname = "exit",
				.modelscale = 8.0f,
				.translation = { 0.0, 0.0 }
		}
};

static MOBJ_FUNCTION_INIT(exit_mobj_init)
{
	ent_exit_t * ent_exit = thisdata;
	ent_exit->message = Z_strdup((char *)args);
}

static void exit_mobj_done(mobj_t * this, void * thisdata)
{
	Z_free( ((ent_exit_t *)thisdata)->message );
}

static const mobj_reginfo_t exit_reginfo = {
		.name = "exit",
		.datasize = sizeof(ent_exit_t),
		.mobjinit = exit_mobj_init,
		.mobjdone = exit_mobj_done,
		.handle   = MOBJ_FUNCTION_HANDLE_DEFAULT,
		.client_store = NULL,
		.client_restore = NULL,
		.entmodels_num = 1,
		.entmodels = exit_models
};

void mobj_exit_init()
{
	mobj_register(&exit_reginfo);
}


