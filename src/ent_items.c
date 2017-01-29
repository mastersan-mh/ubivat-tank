/*
 * items.c
 *
 *  Created on: 10 дек. 2016 г.
 *      Author: mastersan
 */

#include "entity.h"
#include "model.h"
#include "ent_items.h"

static entityvarinfo_t item_vars[] =
{
		{ "amount", ENTITYVARTYPE_INTEGER }
};

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

static const entityinfo_t item_scores_reginfo = {
		.name = "item_scores",
		.datasize = 0,
		ENTITYINFO_VARS(item_vars),
		.init = ENTITY_FUNCTION_NONE,
		.done = ENTITY_FUNCTION_NONE,
		.handle = ENTITY_FUNCTION_NONE,
		.client_store = NULL,
		.client_restore = NULL,
		ENTITYINFO_ENTMODELS(item_scores_models)
};

static const entityinfo_t item_health_reginfo = {
		.name = "item_health",
		.datasize = 0,
		ENTITYINFO_VARS(item_vars),
		.init = ENTITY_FUNCTION_NONE,
		.done = ENTITY_FUNCTION_NONE,
		.handle = ENTITY_FUNCTION_NONE,
		.client_store = NULL,
		.client_restore = NULL,
		ENTITYINFO_ENTMODELS(item_health_models)
};

static const entityinfo_t item_armor_reginfo = {
		.name = "item_armor",
		.datasize = 0,
		ENTITYINFO_VARS(item_vars),
		.init = ENTITY_FUNCTION_NONE,
		.done = ENTITY_FUNCTION_NONE,
		.handle = ENTITY_FUNCTION_NONE,
		.client_store = NULL,
		.client_restore = NULL,
		ENTITYINFO_ENTMODELS(item_armor_models)
};

static const entityinfo_t item_ammo_missile_reginfo = {
		.name = "item_ammo_missile",
		.datasize = 0,
		ENTITYINFO_VARS(item_vars),
		.init = ENTITY_FUNCTION_NONE,
		.done = ENTITY_FUNCTION_NONE,
		.handle = ENTITY_FUNCTION_NONE,
		.client_store = NULL,
		.client_restore = NULL,
		ENTITYINFO_ENTMODELS(item_ammo_missile_models)
};

static const entityinfo_t item_ammo_mine_reginfo = {
		.name = "item_ammo_mine",
		.datasize = 0,
		ENTITYINFO_VARS(item_vars),
		.init = ENTITY_FUNCTION_NONE,
		.done = ENTITY_FUNCTION_NONE,
		.handle = ENTITY_FUNCTION_NONE,
		.client_store = NULL,
		.client_restore = NULL,
		ENTITYINFO_ENTMODELS(item_ammo_mine_models)
};

void entity_items_init(void)
{
	entity_register(&item_scores_reginfo);
	entity_register(&item_health_reginfo);
	entity_register(&item_armor_reginfo);
	entity_register(&item_ammo_missile_reginfo);
	entity_register(&item_ammo_mine_reginfo);
}
