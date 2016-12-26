/*
 * items.c
 *
 *  Created on: 10 дек. 2016 г.
 *      Author: mastersan
 */

#include "mobjs.h"
#include "model.h"
#include "ent_items.h"

static entmodel_t item_scores_models[] =
{
		{
				.modelname = "item_scores",
				.modelscale = 8.0f,
				.translation = { 0.0, 0.0 }
		}
};

static entmodel_t item_health_models[] =
{
		{
				.modelname = "item_health",
				.modelscale = 8.0f,
				.translation = { 0.0, 0.0 }
		}
};

static entmodel_t item_armor_models[] =
{
		{
				.modelname = "item_armor",
				.modelscale = 8.0f,
				.translation = { 0.0, 0.0 }
		}
};

static entmodel_t item_ammo_missile_models[] =
{
		{
				.modelname = "item_ammo_missile",
				.modelscale = 8.0f,
				.translation = { 0.0, 0.0 }
		}
};

static entmodel_t item_ammo_mine_models[] =
{
		{
				.modelname = "item_ammo_mine",
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

static MOBJ_FUNCTION_INIT(item_init)
{
	item_t * item = thisdata;

	itemtype_t itemtype = items_mobjtype_to_itemtype(this->type);
	item->type = itemtype;
	item->exist = true;
	item->amount = *((int*)args);
}

static void item_handle(entity_t * this)
{
	MOBJ_ALLOW_HANDLE_SET(this, ((item_t *)this->data)->exist);
}

static const entityinfo_t item_scores_reginfo = {
		.name = "item_scores",
		.datasize = sizeof(item_t),
		.entityinit = item_init,
		.entitydone = MOBJ_FUNCTION_DONE_DEFAULT,
		.handle   = item_handle,
		.client_store = NULL,
		.client_restore = NULL,
		.entmodels_num = 1,
		.entmodels = item_scores_models
};

static const entityinfo_t item_health_reginfo = {
		.name = "item_health",
		.datasize = sizeof(item_t),
		.entityinit = item_init,
		.entitydone = MOBJ_FUNCTION_DONE_DEFAULT,
		.handle   = item_handle,
		.client_store = NULL,
		.client_restore = NULL,
		.entmodels_num = 1,
		.entmodels = item_health_models
};

static const entityinfo_t item_armor_reginfo = {
		.name = "item_armor",
		.datasize = sizeof(item_t),
		.entityinit = item_init,
		.entitydone = MOBJ_FUNCTION_DONE_DEFAULT,
		.handle   = item_handle,
		.client_store = NULL,
		.client_restore = NULL,
		.entmodels_num = 1,
		.entmodels = item_armor_models
};

static const entityinfo_t item_ammo_missile_reginfo = {
		.name = "item_ammo_missile",
		.datasize = sizeof(item_t),
		.entityinit = item_init,
		.entitydone = MOBJ_FUNCTION_DONE_DEFAULT,
		.handle   = item_handle,
		.client_store = NULL,
		.client_restore = NULL,
		.entmodels_num = 1,
		.entmodels = item_ammo_missile_models
};

static const entityinfo_t item_ammo_mine_reginfo = {
		.name = "item_ammo_mine",
		.datasize = sizeof(item_t),
		.entityinit = item_init,
		.entitydone = MOBJ_FUNCTION_DONE_DEFAULT,
		.handle   = item_handle,
		.client_store = NULL,
		.client_restore = NULL,
		.entmodels_num = 1,
		.entmodels = item_ammo_mine_models
};

void entity_items_init()
{
	entity_register(&item_scores_reginfo);
	entity_register(&item_health_reginfo);
	entity_register(&item_armor_reginfo);
	entity_register(&item_ammo_missile_reginfo);
	entity_register(&item_ammo_mine_reginfo);
}
