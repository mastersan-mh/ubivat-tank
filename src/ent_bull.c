/*
 * bull.c
 *
 *  Created on: 30 нояб. 2016 г.
 *      Author: mastersan
 */

#include "game.h"
#include "weap.h"
#include "entity.h"
#include "model.h"
#include "ent_bull.h"
#include "ent_explode.h"
#include "ent_player.h"
#include "sound.h"

bullinfo_t bullinfo_table[__BULL_NUM] =
{
		{ 15,   7,  -1,  75, 2 },
		{100,  50,  -1,  80, 8 },
		{200, 100, 100, -80, 8 }
};

/*
 * проверка на попадание в игрока
 */
static int checkdamage(entity_t * entity, entity_t * bull)
{
	bullinfo_t * bullinfo = &bullinfo_table[((bull_t*)bull->data)->type];

	player_t * pl = entity->data;

	if(
			((bull_t*)bull->data)->owner != entity && //попали не в себя
			0 < pl->items[ITEM_HEALTH] &&
			(entity->pos.x - c_p_MDL_box / 2 <= bull->pos.x + bullinfo->bodybox / 2)&&
			(bull->pos.x - bullinfo->bodybox / 2 <= entity->pos.x + c_p_MDL_box / 2)&&
			(entity->pos.y - c_p_MDL_box / 2 <= bull->pos.y + bullinfo->bodybox / 2)&&
			(bull->pos.y - bullinfo->bodybox / 2 <= entity->pos.y + c_p_MDL_box / 2)
	) return true;
	return false;
}

/*
 * поведение пули
 */
void bull_common_handle(entity_t * this)
{
	bool Ul,Ur,Dl,Dr,Lu,Ld,Ru,Rd;
	entity_t * entity;
	bull_t * bull = this->data;

	bullinfo_t * bullinfo = &bullinfo_table[bull->type];

	vec_t s = bullinfo->speed * dtimed1000;

	switch(this->dir)
	{
	case DIR_UP   : this->pos.y = this->pos.y + s; break;
	case DIR_DOWN : this->pos.y = this->pos.y - s; break;
	case DIR_LEFT : this->pos.x = this->pos.x - s; break;
	case DIR_RIGHT: this->pos.x = this->pos.x + s; break;
	};

	bull->delta_s = bull->delta_s + VEC_ABS(s);                                //подсчитываем пройденный путь
	if(
			bullinfo->range > -1 && bull->delta_s > bullinfo->range
	)
	{//предельное расстояние пройдено

		explodetype_t explodetype = entity_bull_type_to_explode_type(bull->type);
		/* следим за взрывом */
		entity = entity_new(
			entity_explodetype_to_mobjtype(explodetype),
			this->pos.x,
			this->pos.y,
			this->dir,
			bull->owner,
			NULL
		);
		if(bull->type == BULL_MISSILE)
			ENT_PLAYER(bull->owner)->bull = entity;
		MOBJ_ERASE(this);
		return;
	}


	//предел на расстояние не превышен
	map_clip_find(                                                     //найдем препятствия
		&this->pos,
		bullinfo->bodybox,
		MAP_WALL_W0 | MAP_WALL_w1 | MAP_WALL_brick,
		&Ul,&Ur,&Dl,&Dr,&Lu,&Ld,&Ru,&Rd
	);
	if(Ul || Ur || Dl || Dr || Lu || Ld || Ru || Rd)
	{
		//пуля попала в стену
		explodetype_t explodetype = entity_bull_type_to_explode_type(bull->type);
		entity = entity_new(
			entity_explodetype_to_mobjtype(explodetype),
			this->pos.x,
			this->pos.y,
			this->dir,
			bull->owner,
			NULL
		);
		if(bull->type == BULL_MISSILE)
			ENT_PLAYER(bull->owner)->bull = entity;

		MOBJ_ERASE(this);
		return;
	}

	bool flag = false;
	int i;
	for(i = 0; i < 3; i++)
	{
	//пуля не попала в стену
		entity_t * player;
		switch(i)
		{
		case 0: player = entity_getfirst("player"); break;
		case 1: player = entity_getfirst("enemy"); break;
		case 2: player = entity_getfirst("boss"); break;
		}
		while(player && !flag)
		{                               //проверим на попадание в игрока
			flag = checkdamage(player, this);
			player = player->next;
		};
	}

	if(flag)
	{
		//попадание в игрока
		explodetype_t explodetype = entity_bull_type_to_explode_type(bull->type);
		entity = entity_new(
			entity_explodetype_to_mobjtype(explodetype),
			this->pos.x,
			this->pos.y,
			this->dir,
			bull->owner,
			NULL
		);
		if(bull->type == BULL_MISSILE)
			ENT_PLAYER(bull->owner)->bull = entity;
		MOBJ_ERASE(this);
		return;
	}
}

static void bull_common_modelaction_startplay(entity_t * this, unsigned int imodel, char * actionname)
{
	entity_model_play_start(this, imodel, actionname);
}

/**
 * @description содание пули
 */
static void bull_common_init(entity_t * this, void * thisdata, const entity_t * parent, bulltype_t bulltype)
{
	bull_t * bull = thisdata;
	bull->owner = (entity_t *)parent;
	bull->type = bulltype;
	bull->delta_s = 0;

	if(bulltype != BULL_ARTILLERY)
		bull_common_modelaction_startplay(this, 0, "fly");

	if(bulltype == BULL_MISSILE)
		((player_t *)parent->data)->bull = this;
}

/*
 * bull_artillery
 */
entmodel_t bull_artillery_models[] =
{
		{
				.modelname = "bull_artillery",
				.modelscale = 2.0f / 2.0f, /* picturesize = 2 x 2 */
				.translation = { 0.0, 0.0 },
				.actions_num = 0,
				.actions = NULL
		}
};

static MOBJ_FUNCTION_INIT(bull_artillery_entity_init)
{
	bull_common_init(this, thisdata, parent, BULL_ARTILLERY);
}

static void bull_artillery_handle(entity_t * this)
{
	bull_common_handle(this);
}

static const entityinfo_t bull_artillery_reginfo = {
		.name = "bull_artillery",
		.datasize = sizeof(bull_t),
		.entityinit = bull_artillery_entity_init,
		.entitydone = MOBJ_FUNCTION_DONE_DEFAULT,
		.handle   = bull_artillery_handle,
		.client_store = NULL,
		.client_restore = NULL,
		.entmodels_num = 1,
		.entmodels = bull_artillery_models
};

/**
 * bull_missile
 */

static const ent_modelaction_t bull_missile_modelactions[] =
{
		{
				.name = "fly",
				.startframe = 0,
				.endframe = 1,
				.endframef = bull_common_modelaction_startplay
		}
};

static entmodel_t bull_missile_models[] =
{
		{
				.modelname = "bull_missile",
				.modelscale = 8.0f / 2.0f, /* picturesize = 8 x 64 */
				.translation = { 0.0, 0.0 },
				.actions_num = 1,
				.actions = bull_missile_modelactions
		}
};

static MOBJ_FUNCTION_INIT(bull_missile_entity_init)
{
	bull_common_init(this, thisdata, parent, BULL_MISSILE);
}

static void bull_missile_handle(entity_t * this)
{
	bull_common_handle(this);
}

static const entityinfo_t bull_missile_reginfo = {
		.name = "bull_missile",
		.datasize = sizeof(bull_t),
		.entityinit = bull_missile_entity_init,
		.entitydone = MOBJ_FUNCTION_DONE_DEFAULT,
		.handle   = bull_missile_handle,
		.client_store = NULL,
		.client_restore = NULL,
		.entmodels_num = 1,
		.entmodels = bull_missile_models
};

/**
 * bull_mine
 */


static const ent_modelaction_t bull_mine_modelactions[] =
{
		{
				.name = "fly",
				.startframe = 0,
				.endframe = 1,
				.endframef = bull_common_modelaction_startplay
		}
};

static entmodel_t bull_mine_models[] =
{
		{
				.modelname = "bull_mine",
				.modelscale = 8.0f / 2.0f, /* picturesize = 8 x 64 */
				.translation = { 0.0, 0.0 },
				.actions_num = 1,
				.actions = bull_mine_modelactions
		}
};

static MOBJ_FUNCTION_INIT(bull_mine_entity_init)
{
	bull_common_init(this, thisdata, parent, BULL_MINE);
}

static void bull_mine_handle(entity_t * this)
{
	bull_common_handle(this);
}

static const entityinfo_t bull_mine_reginfo = {
		.name = "bull_mine",
		.datasize = sizeof(bull_t),
		.entityinit = bull_mine_entity_init,
		.entitydone = MOBJ_FUNCTION_DONE_DEFAULT,
		.handle   = bull_mine_handle,
		.client_store = NULL,
		.client_restore = NULL,
		.entmodels_num = 1,
		.entmodels = bull_mine_models
};

/**
 * регистрация
 */
void entity_bull_init()
{
	entity_register(&bull_artillery_reginfo);
	entity_register(&bull_missile_reginfo);
	entity_register(&bull_mine_reginfo);
}
