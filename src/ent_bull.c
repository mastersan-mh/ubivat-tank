/*
 * bull.c
 *
 *  Created on: 30 нояб. 2016 г.
 *      Author: mastersan
 */

#include "game.h"
#include "entity.h"
#include "entity_helpers.h"
#include "model.h"
#include "sound.h"
#include "ent_bull.h"
#include "ent_explode.h"
#include "ent_player.h"
#include "ent_weap.h"

bullinfo_t bullinfo_table[__BULL_NUM] =
{
		{ 15,   7,  -1,  75, 2 },
		{100,  50,  -1,  80, 8 },
		{200, 100, 100, -80, 8 }
};

/*
 * проверка на попадание в игрока
 */
static int checkdamage(entity_t * player, entity_t * bull)
{
	bullinfo_t * bullinfo = &bullinfo_table[((bull_t*)bull->data)->type];

	if(
			bull->parent != player && //попали не в себя
			0 < player->alive &&
			(player->pos.x - c_p_MDL_box / 2 <= bull->pos.x + bullinfo->bodybox / 2)&&
			(bull->pos.x - bullinfo->bodybox / 2 <= player->pos.x + c_p_MDL_box / 2)&&
			(player->pos.y - c_p_MDL_box / 2 <= bull->pos.y + bullinfo->bodybox / 2)&&
			(bull->pos.y - bullinfo->bodybox / 2 <= player->pos.y + c_p_MDL_box / 2)
	) return true;
	return false;
}

/*
 * поведение пули
 */
void bull_common_handle(entity_t * this, bullinfo_t * bullinfo)
{

	vec_t dist = entity_move(this, this->dir, bullinfo->bodybox, bullinfo->speed);

	bool make_explode = false;
	bool Ul,Ur,Dl,Dr,Lu,Ld,Ru,Rd;
	bull_t * bull = this->data;
	//подсчитываем пройденный путь
	bull->delta_s = bull->delta_s + VEC_ABS(dist);
	//предельное расстояние пройдено
	if(bullinfo->range > -1 && bull->delta_s > bullinfo->range)
	{
		make_explode = true;
		goto end;
	}

	//найдем препятствия
	map_clip_find(
		&this->pos,
		bullinfo->bodybox,
		MAP_WALL_W0 | MAP_WALL_W1 | MAP_WALL_brick,
		&Ul,&Ur,&Dl,&Dr,&Lu,&Ld,&Ru,&Rd
	);
	if(Ul || Ur || Dl || Dr || Lu || Ld || Ru || Rd)
	{
		make_explode = true;
		goto end;
	}

	entity_t * entity;
	ENTITIES_FOREACH("player", entity)
	{
		if(checkdamage(entity, this))
		{
			//попадание в игрока, создать взрыв
			make_explode = true;
			goto end;
		}
	}

	ENTITIES_FOREACH("enemy", entity)
	{
		if(checkdamage(entity, this))
		{
			//попадание в игрока, создать взрыв
			make_explode = true;
			goto end;
		}
	}

	ENTITIES_FOREACH("boss", entity)
	{
		if(checkdamage(entity, this))
		{
			//попадание в игрока, создать взрыв
			make_explode = true;
			goto end;
		}
	}

	end:
	if(make_explode)
	{
		explodetype_t explodetype = entity_bull_type_to_explode_type(bull->type);
		entity = entity_new(
			entity_explodetype_to_mobjtype(explodetype),
			this->pos.x,
			this->pos.y,
			this->dir,
			this->parent,
			NULL
		);
		/* следим за взрывом */
		if(bull->type == BULL_MISSILE)
			ENT_PLAYER(this->parent)->bull = entity;
		ENTITY_ERASE(this);
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

static ENTITY_FUNCTION_INIT(bull_artillery_entity_init)
{
	bull_common_init(this, thisdata, parent, BULL_ARTILLERY);
}

static ENTITY_FUNCTION_HANDLE(bull_artillery_handle)
{
	bull_t * bull = this->data;
	bullinfo_t * bullinfo = &bullinfo_table[bull->type];
	bull_common_handle(this, bullinfo);
}

static const entityinfo_t bull_artillery_reginfo = {
		.name = "bull_artillery",
		.datasize = sizeof(bull_t),
		.init = bull_artillery_entity_init,
		.done = ENTITY_FUNCTION_DONE_DEFAULT,
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

static ENTITY_FUNCTION_INIT(bull_missile_entity_init)
{
	bull_common_init(this, thisdata, parent, BULL_MISSILE);
}

static ENTITY_FUNCTION_HANDLE(bull_missile_handle)
{
	bull_t * bull = this->data;
	bullinfo_t * bullinfo = &bullinfo_table[bull->type];
	bull_common_handle(this, bullinfo);
}

static const entityinfo_t bull_missile_reginfo = {
		.name = "bull_missile",
		.datasize = sizeof(bull_t),
		.init = bull_missile_entity_init,
		.done = ENTITY_FUNCTION_DONE_DEFAULT,
		.handle = bull_missile_handle,
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

static ENTITY_FUNCTION_INIT(bull_mine_entity_init)
{
	bull_common_init(this, thisdata, parent, BULL_MINE);
}

static ENTITY_FUNCTION_HANDLE(bull_mine_handle)
{
	bull_t * bull = this->data;
	bullinfo_t * bullinfo = &bullinfo_table[bull->type];
	bull_common_handle(this, bullinfo);
}

static const entityinfo_t bull_mine_reginfo = {
		.name = "bull_mine",
		.datasize = sizeof(bull_t),
		.init = bull_mine_entity_init,
		.done = ENTITY_FUNCTION_DONE_DEFAULT,
		.handle   = bull_mine_handle,
		.client_store = NULL,
		.client_restore = NULL,
		.entmodels_num = 1,
		.entmodels = bull_mine_models
};

/**
 * регистрация
 */
void entity_bull_init(void)
{
	entity_register(&bull_artillery_reginfo);
	entity_register(&bull_missile_reginfo);
	entity_register(&bull_mine_reginfo);
}
