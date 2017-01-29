/*
 * items.c
 *
 *  Created on: 10 дек. 2016 г.
 *      Author: mastersan
 */

#include "entity.h"
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


itemtype_t items_enttype_to_itemtype(const char * entname)
{
	static const char *list[] =
	{
			"item_scores" , /* ITEM_SCORES */
			"item_health" , /* ITEM_HEALTH */
			"item_armor"  , /* ITEM_ARMOR */
			"item_ammo_artillery", /* ITEM_AMMO_ARTILLERY */
			"item_ammo_missile", /* ITEM_AMMO_MISSILE */
			"item_ammo_mine"     /* ITEM_AMMO_MINE */
	};
	size_t i;

	for(i = 0; i < sizeof(list) / sizeof(*list); i++)
	{
		if(strcmp(list[i], entname) == 0)
			return i;
	}
	return -1;
}

static ENTITY_FUNCTION_INIT(item_init)
{
	item_t * item = thisdata;

	itemtype_t itemtype = items_enttype_to_itemtype(this->info->name);
	item->type = itemtype;
	item->exist = true;
	item->amount = *((int*)args);
}

static ENTITY_FUNCTION_HANDLE(item_handle)
{
	ENTITY_ALLOW_HANDLE_SET(this, ((item_t *)thisdata)->exist);
}

static const entityinfo_t item_scores_reginfo = {
		.name = "item_scores",
		.datasize = sizeof(item_t),
		.init = item_init,
		.done = ENTITY_FUNCTION_NONE,
		.handle = item_handle,
		.client_store = NULL,
		.client_restore = NULL,
		.entmodels_num = 1,
		.entmodels = item_scores_models
};

static const entityinfo_t item_health_reginfo = {
		.name = "item_health",
		.datasize = sizeof(item_t),
		.init = item_init,
		.done = ENTITY_FUNCTION_NONE,
		.handle   = item_handle,
		.client_store = NULL,
		.client_restore = NULL,
		.entmodels_num = 1,
		.entmodels = item_health_models
};

static const entityinfo_t item_armor_reginfo = {
		.name = "item_armor",
		.datasize = sizeof(item_t),
		.init = item_init,
		.done = ENTITY_FUNCTION_NONE,
		.handle   = item_handle,
		.client_store = NULL,
		.client_restore = NULL,
		.entmodels_num = 1,
		.entmodels = item_armor_models
};

static const entityinfo_t item_ammo_missile_reginfo = {
		.name = "item_ammo_missile",
		.datasize = sizeof(item_t),
		.init = item_init,
		.done = ENTITY_FUNCTION_NONE,
		.handle   = item_handle,
		.client_store = NULL,
		.client_restore = NULL,
		.entmodels_num = 1,
		.entmodels = item_ammo_missile_models
};

static const entityinfo_t item_ammo_mine_reginfo = {
		.name = "item_ammo_mine",
		.datasize = sizeof(item_t),
		.init = item_init,
		.done = ENTITY_FUNCTION_NONE,
		.handle   = item_handle,
		.client_store = NULL,
		.client_restore = NULL,
		.entmodels_num = 1,
		.entmodels = item_ammo_mine_models
};

void entity_items_init(void)
{
	entity_register(&item_scores_reginfo);
	entity_register(&item_health_reginfo);
	entity_register(&item_armor_reginfo);
	entity_register(&item_ammo_missile_reginfo);
	entity_register(&item_ammo_mine_reginfo);
}
