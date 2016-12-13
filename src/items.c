/*
 * items.c
 *
 *  Created on: 10 дек. 2016 г.
 *      Author: mastersan
 */

#include "items.h"
#include "model.h"

model_t mdl_item_scores =
{
		.name = "item_scores",
		.vertexes_num = VERTEXES_COMMON_NUM,
		.vertexes = vertexes_common,
		.triangles_num = TRIANGLES_COMMON_NUM,
		.triangles = trianges_common,
		.itexture = IMG_ITEM_STAR,
		.fps = 0,
		.frames_num = MODEL_FRAMES_COMMON_NUM,
		.frames = model_frames_common
};

model_t mdl_item_health =
{
		.name = "item_health",
		.vertexes_num = VERTEXES_COMMON_NUM,
		.vertexes = vertexes_common,
		.triangles_num = TRIANGLES_COMMON_NUM,
		.triangles = trianges_common,
		.itexture = IMG_ITEM_HEALTH,
		.frames_num = MODEL_FRAMES_COMMON_NUM,
		.frames = model_frames_common
};

model_t mdl_item_armor =
{
		.name = "item_armor",
		.vertexes_num = VERTEXES_COMMON_NUM,
		.vertexes = vertexes_common,
		.triangles_num = TRIANGLES_COMMON_NUM,
		.triangles = trianges_common,
		.itexture = IMG_ITEM_ARMOR,
		.frames_num = MODEL_FRAMES_COMMON_NUM,
		.frames = model_frames_common
};

model_t mdl_item_ammo_missile =
{
		.name = "item_ammo_missile",
		.vertexes_num = VERTEXES_COMMON_NUM,
		.vertexes = vertexes_common,
		.triangles_num = TRIANGLES_COMMON_NUM,
		.triangles = trianges_common,
		.itexture = IMG_WEAPON_MISSILE,
		.frames_num = MODEL_FRAMES_COMMON_NUM,
		.frames = model_frames_common
};

model_t mdl_item_ammo_mine =
{
		.name = "item_ammo_mine",
		.vertexes_num = VERTEXES_COMMON_NUM,
		.vertexes = vertexes_common,
		.triangles_num = TRIANGLES_COMMON_NUM,
		.triangles = trianges_common,
		.itexture = IMG_WEAPON_MINE,
		.frames_num = MODEL_FRAMES_COMMON_NUM,
		.frames = model_frames_common
};

ent_model_t item_scores_models[] =
{
		{
				.model = &mdl_item_scores,
				.modelscale = 8.0f,
				.translation = { 0.0, 0.0 }
		}
};

ent_model_t item_health_models[] =
{
		{
				.model = &mdl_item_health,
				.modelscale = 8.0f,
				.translation = { 0.0, 0.0 }
		}
};

ent_model_t item_armor_models[] =
{
		{
				.model = &mdl_item_armor,
				.modelscale = 8.0f,
				.translation = { 0.0, 0.0 }
		}
};

ent_model_t item_ammo_missile_models[] =
{
		{
				.model = &mdl_item_ammo_missile,
				.modelscale = 8.0f,
				.translation = { 0.0, 0.0 }
		}
};

ent_model_t item_ammo_mine_models[] =
{
		{
				.model = &mdl_item_ammo_mine,
				.modelscale = 8.0f,
				.translation = { 0.0, 0.0 }
		}
};


itemtype_t items_mobjtype_to_itemtype(mobj_type_t mobjtype)
{
	switch(mobjtype)
	{
	case MOBJ_ITEM_SCORES: return ITEM_SCORES;
	case MOBJ_ITEM_HEALTH: return ITEM_HEALTH;
	case MOBJ_ITEM_ARMOR : return ITEM_ARMOR;
	case MOBJ_ITEM_AMMO_MISSILE: return ITEM_AMMO_MISSILE;
	case MOBJ_ITEM_AMMO_MINE   : return ITEM_AMMO_MINE;
	default: ;
	}
	return -1;
}

static MOBJ_FUNCTION_INIT(item_mobj_init)
{
	item_t * item = thisdata;

	itemtype_t itemtype = items_mobjtype_to_itemtype(this->type);
	item->type = itemtype;
	item->exist = true;
	item->amount = *((int*)args);
}

static void item_handle(mobj_t * this)
{
	this->show = ((item_t *)this->data)->exist;
}

static const mobj_reginfo_t item_scores_reginfo = {
		.name = "item_scores",
		.datasize = sizeof(item_t),
		.mobjinit = item_mobj_init,
		.mobjdone = MOBJ_FUNCTION_DONE_DEFAULT,
		.handle   = item_handle,
		.client_store = NULL,
		.client_restore = NULL,
		.models_num = 1,
		.models = item_scores_models
};

static const mobj_reginfo_t item_health_reginfo = {
		.name = "item_health",
		.datasize = sizeof(item_t),
		.mobjinit = item_mobj_init,
		.mobjdone = MOBJ_FUNCTION_DONE_DEFAULT,
		.handle   = item_handle,
		.client_store = NULL,
		.client_restore = NULL,
		.models_num = 1,
		.models = item_health_models
};

static const mobj_reginfo_t item_armor_reginfo = {
		.name = "item_armor",
		.datasize = sizeof(item_t),
		.mobjinit = item_mobj_init,
		.mobjdone = MOBJ_FUNCTION_DONE_DEFAULT,
		.handle   = item_handle,
		.client_store = NULL,
		.client_restore = NULL,
		.models_num = 1,
		.models = item_armor_models
};

static const mobj_reginfo_t item_ammo_missile_reginfo = {
		.name = "item_ammo_missile",
		.datasize = sizeof(item_t),
		.mobjinit = item_mobj_init,
		.mobjdone = MOBJ_FUNCTION_DONE_DEFAULT,
		.handle   = item_handle,
		.client_store = NULL,
		.client_restore = NULL,
		.models_num = 1,
		.models = item_ammo_missile_models
};

static const mobj_reginfo_t item_ammo_mine_reginfo = {
		.name = "item_ammo_mine",
		.datasize = sizeof(item_t),
		.mobjinit = item_mobj_init,
		.mobjdone = MOBJ_FUNCTION_DONE_DEFAULT,
		.handle   = item_handle,
		.client_store = NULL,
		.client_restore = NULL,
		.models_num = 1,
		.models = item_ammo_mine_models
};

void mobj_items_init()
{
	mobj_register(&item_scores_reginfo);
	mobj_register(&item_health_reginfo);
	mobj_register(&item_armor_reginfo);
	mobj_register(&item_ammo_missile_reginfo);
	mobj_register(&item_ammo_mine_reginfo);
}
