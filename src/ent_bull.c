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

typedef enum
{
	BULL_ARTILLERY,
	BULL_MISSILE,
	BULL_MINE,
	BULL_NUM
} bulltype_t;

typedef struct
{
	//повреждение
	int damage;
	//повреждение
	int selfdamage;
	//дальность
	vec_t range;
	//начальная скорость пули
	vec_t speed;
} bullinfo_t;

static const bullinfo_t bullinfo_table[BULL_NUM] =
{
		{ 15,   7,  -1,  75 },
		{100,  50,  -1,  80 },
		{200, 100, 100,  80 }
};

static void bull_artillery_detonate(entity_t * this, entity_t * that)
{
	if(that && this->parent == that)
		return;
	entity_new(
		"explode_artillery",
		this->pos.x,
		this->pos.y,
		this->dir,
		this->parent
	);
	ENTITY_ERASE(this);
}

static void bull_missile_detonate(entity_t * this, entity_t * that)
{
	if(that && this->parent == that)
		return;
	ENT_PLAYER(this->parent)->bull =
	entity_new(
		"explode_missile",
		this->pos.x,
		this->pos.y,
		this->dir,
		this->parent
	);
	ENTITY_ERASE(this);
}

static void bull_mine_detonate(entity_t * this, entity_t * that)
{
	if(that && this->parent == that)
		return;
	entity_new(
		"explode_mine",
		this->pos.x,
		this->pos.y,
		this->dir,
		this->parent
	);
	ENTITY_ERASE(this);
}

/*
 * обработка пули
 * @return true  = сдетонировала
 * @return false = полёт продолжается
 */
static bool bull_common_handle(entity_t * this, const bullinfo_t * bullinfo)
{
	entity_move(this, this->dir, bullinfo->speed, false);

	//предельное расстояние пройдено
	if(bullinfo->range > -1 && this->stat_traveled_distance > bullinfo->range)
		return true;

	//найдем препятствия
	bool Ul,Ur,Dl,Dr,Lu,Ld,Ru,Rd;
	map_clip_find(
		&this->pos,
		this->info->bodybox,
		MAP_WALL_W0 | MAP_WALL_W1 | MAP_WALL_brick,
		&Ul,&Ur,&Dl,&Dr,&Lu,&Ld,&Ru,&Rd
	);
	if(Ul || Ur || Dl || Dr || Lu || Ld || Ru || Rd)
		return true;
	return false;
}

static void bull_common_modelaction_startplay(entity_t * this, unsigned int imodel, char * actionname)
{
	entity_model_play_start(this, imodel, actionname);
}

/*
 * bull_artillery
 */
entitymodel_t bull_artillery_models[] =
{
		{
				.modelname = "bull_artillery",
				.modelscale = 2.0f / 2.0f, /* picturesize = 2 x 2 */
				.translation = { 0.0, 0.0 },
				.actions_num = 0,
				.actions = NULL
		}
};

static ENTITY_FUNCTION_HANDLE(bull_artillery_handle)
{
	const bullinfo_t * bullinfo = &bullinfo_table[BULL_ARTILLERY];
	bool detonate = bull_common_handle(this, bullinfo);
	if(detonate)
		bull_artillery_detonate(this, NULL);
}

ENTITY_FUNCTION_TOUCH(bull_artillery_touch)
{
	bull_artillery_detonate(this, that);
}

entitytouch_t bull_artillery_touchs[] =
{
		{ "player", bull_artillery_touch },
		{ "enemy" , bull_artillery_touch },
		{ "boss"  , bull_artillery_touch }
};

static const entityinfo_t bull_artillery_reginfo = {
		.name = "bull_artillery",
		.datasize = 0,
		.bodybox = 2.0f,
		ENTITYINFO_ENTMODELS(bull_artillery_models),
		.init = ENTITY_FUNCTION_NONE,
		.done = ENTITY_FUNCTION_NONE,
		.handle   = bull_artillery_handle,
		.client_store = NULL,
		.client_restore = NULL,
		ENTITYINFO_TOUCHS(bull_artillery_touchs)
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

static entitymodel_t bull_missile_models[] =
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
	bull_common_modelaction_startplay(this, 0, "fly");
	((player_t *)parent->data)->bull = this;
}

static ENTITY_FUNCTION_HANDLE(bull_missile_handle)
{
	const bullinfo_t * bullinfo = &bullinfo_table[BULL_MISSILE];
	bool detonate = bull_common_handle(this, bullinfo);
	if(detonate)
		bull_missile_detonate(this, NULL);
}

ENTITY_FUNCTION_TOUCH(bull_missile_touch)
{
	bull_missile_detonate(this, that);
}

entitytouch_t bull_missile_touchs[] =
{
		{ "player", bull_missile_touch },
		{ "enemy" , bull_missile_touch },
		{ "boss"  , bull_missile_touch }
};

static const entityinfo_t bull_missile_reginfo = {
		.name = "bull_missile",
		.datasize = 0,
		.bodybox = 8.0f,
		ENTITYINFO_ENTMODELS(bull_missile_models),
		.init = bull_missile_entity_init,
		.done = ENTITY_FUNCTION_NONE,
		.handle = bull_missile_handle,
		.client_store = NULL,
		.client_restore = NULL,
		ENTITYINFO_TOUCHS(bull_missile_touchs)
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

static entitymodel_t bull_mine_models[] =
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
	bull_common_modelaction_startplay(this, 0, "fly");
}

static ENTITY_FUNCTION_HANDLE(bull_mine_handle)
{
	const bullinfo_t * bullinfo = &bullinfo_table[BULL_MINE];
	bool detonate = bull_common_handle(this, bullinfo);
	if(detonate)
		bull_mine_detonate(this, NULL);
}

ENTITY_FUNCTION_TOUCH(bull_mine_touch)
{
	bull_mine_detonate(this, that);
}

entitytouch_t bull_mine_touchs[] =
{
		{ "player", bull_mine_touch },
		{ "enemy" , bull_mine_touch },
		{ "boss"  , bull_mine_touch }
};

static const entityinfo_t bull_mine_reginfo = {
		.name = "bull_mine",
		.datasize = 0,
		.bodybox = 8.0f,
		ENTITYINFO_ENTMODELS(bull_mine_models),
		.init = bull_mine_entity_init,
		.done = ENTITY_FUNCTION_NONE,
		.handle = bull_mine_handle,
		.client_store = NULL,
		.client_restore = NULL,
		ENTITYINFO_TOUCHS(bull_mine_touchs)
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
