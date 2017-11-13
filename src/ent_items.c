/*
 * items.c
 *
 *  Created on: 10 дек. 2016 г.
 *      Author: mastersan
 */

#include "model.h"
#include "ent_items.h"

static var_descr_t item_vars[] =
{
        ENTITY_VARS_COMMON,
        VAR_DESCR( VARTYPE_INTEGER, entity_item_t, amount )
};

static entity_model_t item_scores_models[] =
{
		{
				.modelname = "item_scores",
				.modelscale = 8.0f,
				.translation = { 0.0, 0.0 }
		}
};

static entity_model_t item_health_models[] =
{
		{
				.modelname = "item_health",
				.modelscale = 8.0f,
				.translation = { 0.0, 0.0 }
		}
};

static entity_model_t item_armor_models[] =
{
		{
				.modelname = "item_armor",
				.modelscale = 8.0f,
				.translation = { 0.0, 0.0 }
		}
};

static entity_model_t item_ammo_missile_models[] =
{
		{
				.modelname = "item_ammo_missile",
				.modelscale = 8.0f,
				.translation = { 0.0, 0.0 }
		}
};

static entity_model_t item_ammo_mine_models[] =
{
		{
				.modelname = "item_ammo_mine",
				.modelscale = 8.0f,
				.translation = { 0.0, 0.0 }
		}
};

static const entityinfo_t item_scores_reginfo = {
		.name = "item_scores",
		.bodybox = 16.0f,
		ENTITYINFO_VARS(entity_item_t, item_vars),
		ENTITYINFO_ENTMODELS(item_scores_models),
		.init = ENTITY_FUNCTION_NONE,
		.done = ENTITY_FUNCTION_NONE,
		.handle = ENTITY_FUNCTION_NONE,
};

static const entityinfo_t item_health_reginfo = {
		.name = "item_health",
		.bodybox = 16.0f,
		ENTITYINFO_VARS(entity_item_t, item_vars),
		ENTITYINFO_ENTMODELS(item_health_models),
		.init = ENTITY_FUNCTION_NONE,
		.done = ENTITY_FUNCTION_NONE,
		.handle = ENTITY_FUNCTION_NONE,
};

static const entityinfo_t item_armor_reginfo = {
		.name = "item_armor",
		.bodybox = 16.0f,
		ENTITYINFO_VARS(entity_item_t, item_vars),
		ENTITYINFO_ENTMODELS(item_armor_models),
		.init = ENTITY_FUNCTION_NONE,
		.done = ENTITY_FUNCTION_NONE,
		.handle = ENTITY_FUNCTION_NONE,
};

static const entityinfo_t item_ammo_missile_reginfo = {
		.name = "item_ammo_missile",
		.bodybox = 16.0f,
		ENTITYINFO_VARS(entity_item_t, item_vars),
		ENTITYINFO_ENTMODELS(item_ammo_missile_models),
		.init = ENTITY_FUNCTION_NONE,
		.done = ENTITY_FUNCTION_NONE,
		.handle = ENTITY_FUNCTION_NONE,
};

static const entityinfo_t item_ammo_mine_reginfo = {
		.name = "item_ammo_mine",
		.bodybox = 16.0f,
		ENTITYINFO_VARS(entity_item_t, item_vars),
		ENTITYINFO_ENTMODELS(item_ammo_mine_models),
		.init = ENTITY_FUNCTION_NONE,
		.done = ENTITY_FUNCTION_NONE,
		.handle = ENTITY_FUNCTION_NONE,
};

void entity_items_init(void)
{
	entity_register(&item_scores_reginfo);
	entity_register(&item_health_reginfo);
	entity_register(&item_armor_reginfo);
	entity_register(&item_ammo_missile_reginfo);
	entity_register(&item_ammo_mine_reginfo);
}
