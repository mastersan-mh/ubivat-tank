/*
 * ent_exit.c
 *
 *  Created on: 9 дек. 2016 г.
 *      Author: mastersan
 */

#include "ent_exit.h"
#include "mobjs.h"
#include "model.h"

static const model_t mdl_exit =
{
		.name = "exit",
		.vertexes_num = VERTEXES_COMMON_NUM,
		.vertexes = model_vertexes_common,
		.triangles_num = TRIANGLES_COMMON_NUM,
		.triangles = model_trianges_common,
		.itexture = IMG_OBJ_EXIT,
		.fps = 0,
		.frames_num = MODEL_FRAMES_COMMON_NUM,
		.frames = model_frames_common
};

static entmodel_t exit_models[] =
{
		{
				.model = &mdl_exit,
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


