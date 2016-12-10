/*
 * items.c
 *
 *  Created on: 10 дек. 2016 г.
 *      Author: mastersan
 */

#include "items.h"

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
	static image_index_t item_imageindexes[] = {
			IMG_ITEM_STAR  ,
			IMG_ITEM_HEALTH,
			IMG_ITEM_ARMOR ,
			IMG_WEAPON_MISSILE,
			IMG_WEAPON_MINE
	};

	item_t * item = thisdata;

	itemtype_t itemtype = items_mobjtype_to_itemtype(this->type);
	item->type = itemtype;
	item->exist = true;
	item->amount = *((int*)args);

	this->img = image_get(item_imageindexes[itemtype]);
}


static const mobj_reginfo_t item_scores_reginfo = {
		.name = "item_scores",
		.datasize = sizeof(item_t),
		.mobjinit = item_mobj_init,
		.mobjdone = MOBJ_FUNCTION_DONE_DEFAULT,
		.handle   = MOBJ_FUNCTION_HANDLE_DEFAULT,
		.client_store = NULL,
		.client_restore = NULL
};

static const mobj_reginfo_t item_health_reginfo = {
		.name = "item_health",
		.datasize = sizeof(item_t),
		.mobjinit = item_mobj_init,
		.mobjdone = MOBJ_FUNCTION_DONE_DEFAULT,
		.handle   = MOBJ_FUNCTION_HANDLE_DEFAULT,
		.client_store = NULL,
		.client_restore = NULL
};

static const mobj_reginfo_t item_armor_reginfo = {
		.name = "item_armor",
		.datasize = sizeof(item_t),
		.mobjinit = item_mobj_init,
		.mobjdone = MOBJ_FUNCTION_DONE_DEFAULT,
		.handle   = MOBJ_FUNCTION_HANDLE_DEFAULT,
		.client_store = NULL,
		.client_restore = NULL
};

static const mobj_reginfo_t item_ammo_missile_reginfo = {
		.name = "item_ammo_missile",
		.datasize = sizeof(item_t),
		.mobjinit = item_mobj_init,
		.mobjdone = MOBJ_FUNCTION_DONE_DEFAULT,
		.handle   = MOBJ_FUNCTION_HANDLE_DEFAULT,
		.client_store = NULL,
		.client_restore = NULL
};

static const mobj_reginfo_t item_ammo_mine_reginfo = {
		.name = "item_ammo_mine",
		.datasize = sizeof(item_t),
		.mobjinit = item_mobj_init,
		.mobjdone = MOBJ_FUNCTION_DONE_DEFAULT,
		.handle   = MOBJ_FUNCTION_HANDLE_DEFAULT,
		.client_store = NULL,
		.client_restore = NULL
};

void mobj_items_init()
{
	mobj_register(&item_scores_reginfo);
	mobj_register(&item_health_reginfo);
	mobj_register(&item_armor_reginfo);
	mobj_register(&item_ammo_missile_reginfo);
	mobj_register(&item_ammo_mine_reginfo);
}
