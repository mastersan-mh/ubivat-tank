/*
 * items.c
 *
 *  Created on: 10 дек. 2016 г.
 *      Author: mastersan
 */

#include "progs.h"

#include "ent_items.h"

static var_descr_t item_vars[] =
{
        ENTITY_VARS_COMMON,
        VAR_DESCR( VARTYPE_INTEGER, entity_item_t, amount )
};

static ENTITY_FUNCTION_INIT(item_scores_init)
{
    entity_bodybox_set(this, 16.0f);
    entity_model_set(this, 0, ":/item_scores", 8.0f, 0.0f, 0.0f);
}

static const entityinfo_t item_scores_reginfo = {
		.name_ = "item_scores",
		ENTITYINFO_VARS(entity_item_t, item_vars),
		.models_num = 1,
		.init = item_scores_init,
		.done = ENTITY_FUNCTION_NONE,
		.handle = ENTITY_FUNCTION_NONE,
};

static ENTITY_FUNCTION_INIT(item_heath_init)
{
    entity_bodybox_set(this, 16.0f);
    entity_model_set(this, 0, ":/item_health", 8.0f, 0.0f, 0.0f);
}

static const entityinfo_t item_health_reginfo = {
		.name_ = "item_health",
		ENTITYINFO_VARS(entity_item_t, item_vars),
        .models_num = 1,
		.init = item_heath_init,
		.done = ENTITY_FUNCTION_NONE,
		.handle = ENTITY_FUNCTION_NONE,
};

static ENTITY_FUNCTION_INIT(item_armor_init)
{
    entity_bodybox_set(this, 16.0f);
    entity_model_set(this, 0, ":/item_armor", 8.0f, 0.0f, 0.0f);
}

static const entityinfo_t item_armor_reginfo = {
		.name_ = "item_armor",
		ENTITYINFO_VARS(entity_item_t, item_vars),
        .models_num = 1,
		.init = item_armor_init,
		.done = ENTITY_FUNCTION_NONE,
		.handle = ENTITY_FUNCTION_NONE,
};

static ENTITY_FUNCTION_INIT(item_ammo_missile_init)
{
    entity_bodybox_set(this, 16.0f);
    entity_model_set(this, 0, ":/item_ammo_missile", 8.0f, 0.0f, 0.0f);
}

static const entityinfo_t item_ammo_missile_reginfo = {
		.name_ = "item_ammo_missile",
		ENTITYINFO_VARS(entity_item_t, item_vars),
        .models_num = 1,
		.init = item_ammo_missile_init,
		.done = ENTITY_FUNCTION_NONE,
		.handle = ENTITY_FUNCTION_NONE,
};

static ENTITY_FUNCTION_INIT(item_ammo_mine_init)
{
    entity_bodybox_set(this, 16.0f);
    entity_model_set(this, 0, ":/item_ammo_mine", 8.0f, 0.0f, 0.0f);
}

static const entityinfo_t item_ammo_mine_reginfo = {
		.name_ = "item_ammo_mine",
		ENTITYINFO_VARS(entity_item_t, item_vars),
        .models_num = 1,
		.init = item_ammo_mine_init,
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
