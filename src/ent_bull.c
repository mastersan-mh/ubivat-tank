/*
 * bull.c
 *
 *  Created on: 30 нояб. 2016 г.
 *      Author: mastersan
 */

#include "game.h"
#include "weap.h"
#include "mobjs.h"
#include "model.h"
#include "ent_bull.h"
#include "ent_explode.h"
#include "ent_player.h"
#include "sound.h"

#define BULL_FPS 8

#define BULL_MISSILE_MODEL_FRAMES (2)
#define BULL_MINE_MODEL_FRAMES (2)

bullinfo_t bullinfo_table[__BULL_NUM] =
{
		{ 15,   7,  -1,  75, 2 },
		{100,  50,  -1,  80, 8 },
		{200, 100, 100, -80, 8 }
};

/*
 * проверка на попадание в игрока
 */
static int checkdamage(mobj_t * mobj, mobj_t * bull)
{
	bullinfo_t * bullinfo = &bullinfo_table[((bull_t*)bull->data)->type];

	player_t * pl = mobj->data;

	if(
			((bull_t*)bull->data)->owner != mobj && //попали не в себя
			0 < pl->items[ITEM_HEALTH] &&
			(mobj->pos.x - c_p_MDL_box / 2 <= bull->pos.x + bullinfo->bodybox / 2)&&
			(bull->pos.x - bullinfo->bodybox / 2 <= mobj->pos.x + c_p_MDL_box / 2)&&
			(mobj->pos.y - c_p_MDL_box / 2 <= bull->pos.y + bullinfo->bodybox / 2)&&
			(bull->pos.y - bullinfo->bodybox / 2 <= mobj->pos.y + c_p_MDL_box / 2)
	) return true;
	return false;
}

/*
 * поведение пули
 */
void bull_common_handle(mobj_t * this)
{
	bool Ul,Ur,Dl,Dr,Lu,Ld,Ru,Rd;
	mobj_t * player;
	mobj_t * mobj;
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

		explodetype_t explodetype = mobj_bull_type_to_explode_type(bull->type);
		mobj_type_t mobj_type = mobj_explodetype_to_mobjtype(explodetype);
		/* следим за взрывом */
		mobj = mobj_new(
			mobj_type,
			this->pos.x,
			this->pos.y,
			this->dir,
			bull->owner,
			NULL
		);
		if(bull->type == BULL_MISSILE)
			ENT_PLAYER(bull->owner)->bull = mobj;
		this->erase = true;
		return;
	}


	//предел на расстояние не превышен
	map_clip_find(                                                     //найдем препятствия
		&this->pos,
		bullinfo->bodybox,
		c_m_w_w0 | c_m_w_w1 | c_m_w_brick,
		&Ul,&Ur,&Dl,&Dr,&Lu,&Ld,&Ru,&Rd
	);
	if(Ul || Ur || Dl || Dr || Lu || Ld || Ru || Rd)
	{
		//пуля попала в стену
		explodetype_t explodetype = mobj_bull_type_to_explode_type(bull->type);
		mobj_type_t mobj_type = mobj_explodetype_to_mobjtype(explodetype);

		mobj = mobj_new(
			mobj_type,
			this->pos.x,
			this->pos.y,
			this->dir,
			bull->owner,
			NULL
		);
		if(bull->type == BULL_MISSILE)
			ENT_PLAYER(bull->owner)->bull = mobj;

		this->erase = true;
		return;
	}

	//пуля не попала в стену
	player = map.mobjs;
	bool flag = false;
	while(player && !flag)
	{                               //проверим на попадание в игрока
		if(
				player->type == MOBJ_PLAYER ||
				player->type == MOBJ_ENEMY ||
				player->type == MOBJ_BOSS
		)
		{
			flag = checkdamage(player, this);
		}
		player = player->next;
	};
	if(flag)
	{                                              //попадание в игрока
		//пуля попала в стену
		explodetype_t explodetype = mobj_bull_type_to_explode_type(bull->type);
		mobj_type_t mobj_type = mobj_explodetype_to_mobjtype(explodetype);
		mobj = mobj_new(
			mobj_type,
			this->pos.x,
			this->pos.y,
			this->dir,
			bull->owner,
			NULL
		);
		if(bull->type == BULL_MISSILE)
			ENT_PLAYER(bull->owner)->bull = mobj;
		this->erase = true;
		return;
	}
}

static void bull_common_modelaction_startplay(mobj_t * this, char * actionname)
{
	mobj_model_start_play(this, 0, actionname);
}

/**
 * @description содание пули
 */
static void bull_common_mobj_init(mobj_t * this, void * thisdata, const mobj_t * parent, bulltype_t bulltype)
{
	bull_t * bull = thisdata;
	bull->owner = (mobj_t *)parent;
	bull->type = bulltype;
	bull->delta_s = 0;

	if(this->type != MOBJ_BULL_ARTILLERY)
		bull_common_modelaction_startplay(this, "fly");

	if(bulltype == BULL_MISSILE) ((player_t *)parent->data)->bull = this;
}

/*
 * bull_artillery
 */

modelframe_t model_bull_artillery_frames[8] =
{
		{ .texcoord = model_texcoord_common }
};

model_t mdl_bull_artillery =
{
		.name = "bull_artillery",
		.vertexes_num = VERTEXES_COMMON_NUM,
		.vertexes = model_vertexes_common,
		.triangles_num = TRIANGLES_COMMON_NUM,
		.triangles = model_trianges_common,
		.itexture = IMG_BULL_ARTILLERY,
		.fps = 0,
		.frames_num = 1,
		.frames = model_bull_artillery_frames
};

entmodel_t bull_artillery_models[] =
{
		{
				.model = &mdl_bull_artillery,
				.modelscale = 2.0f / 2.0f, /* picturesize = 2 x 2 */
				.translation = { 0.0, 0.0 },
				.actions_num = 0,
				.actions = NULL
		}
};

/**
 * artillery
 */
static MOBJ_FUNCTION_INIT(bull_artillery_mobj_init)
{
	bull_common_mobj_init(this, thisdata, parent, BULL_ARTILLERY);
}

static void bull_artillery_handle(mobj_t * this)
{
	bull_common_handle(this);
}

static const mobj_reginfo_t bull_artillery_reginfo = {
		.name = "bull_artillery",
		.datasize = sizeof(bull_t),
		.mobjinit = bull_artillery_mobj_init,
		.mobjdone = MOBJ_FUNCTION_DONE_DEFAULT,
		.handle   = bull_artillery_handle,
		.client_store = NULL,
		.client_restore = NULL,
		.entmodels_num = 1,
		.entmodels = bull_artillery_models
};

/**
 * missile
 */

static MODEL_COMMON_VERTEX4_TEXCOORD_FRAME(8frame0 , 0, 8);
static MODEL_COMMON_VERTEX4_TEXCOORD_FRAME(8frame1 , 1, 8);

/*
В оригинальной текстуре ракеты 8 кадров, по 2 на каждое направление. Мы используем только 2 первых кадра.
*/
static modelframe_t model_bull_missile_frames[BULL_MISSILE_MODEL_FRAMES] =
{
		{ .texcoord = model_texcoord_8frame0 },
		{ .texcoord = model_texcoord_8frame1 }
};

static const model_t mdl_bull_missile =
{
		.name = "bull_missile",
		.vertexes_num = VERTEXES_COMMON_NUM,
		.vertexes = model_vertexes_common,
		.triangles_num = TRIANGLES_COMMON_NUM,
		.triangles = model_trianges_common,
		.itexture = IMG_BULL_MISSILE,
		.fps = BULL_FPS,
		.frames_num = BULL_MISSILE_MODEL_FRAMES,
		.frames = model_bull_missile_frames
};

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
				.model = &mdl_bull_missile,
				.modelscale = 8.0f / 2.0f, /* picturesize = 8 x 64 */
				.translation = { 0.0, 0.0 },
				.actions_num = 1,
				.actions = bull_missile_modelactions
		}
};

static MOBJ_FUNCTION_INIT(bull_missile_mobj_init)
{
	return bull_common_mobj_init(this, thisdata, parent, BULL_MISSILE);
}

static void bull_missile_handle(mobj_t * this)
{
	bull_common_handle(this);
}

static const mobj_reginfo_t bull_missile_reginfo = {
		.name = "bull_missile",
		.datasize = sizeof(bull_t),
		.mobjinit = bull_missile_mobj_init,
		.mobjdone = MOBJ_FUNCTION_DONE_DEFAULT,
		.handle   = bull_missile_handle,
		.client_store = NULL,
		.client_restore = NULL,
		.entmodels_num = 1,
		.entmodels = bull_missile_models
};

/**
 * mine
 */

/*
В оригинальной текстуре ракеты 8 кадров, по 2 на каждое направление. Мы используем только 2 первых кадра.
*/
static modelframe_t model_bull_mine_frames[BULL_MISSILE_MODEL_FRAMES] =
{
		{ .texcoord = model_texcoord_8frame0 },
		{ .texcoord = model_texcoord_8frame1 }
};

static const model_t mdl_bull_mine =
{
		.name = "bull_mine",
		.vertexes_num = VERTEXES_COMMON_NUM,
		.vertexes = model_vertexes_common,
		.triangles_num = TRIANGLES_COMMON_NUM,
		.triangles = model_trianges_common,
		.itexture = IMG_BULL_MINE,
		.fps = BULL_FPS,
		.frames_num = BULL_MISSILE_MODEL_FRAMES,
		.frames = model_bull_mine_frames
};

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
				.model = &mdl_bull_mine,
				.modelscale = 8.0f / 2.0f, /* picturesize = 8 x 64 */
				.translation = { 0.0, 0.0 },
				.actions_num = 1,
				.actions = bull_mine_modelactions
		}
};

static MOBJ_FUNCTION_INIT(bull_mine_mobj_init)
{
	return bull_common_mobj_init(this, thisdata, parent, BULL_MINE);
}

static void bull_mine_handle(mobj_t * this)
{
	bull_common_handle(this);
}

static const mobj_reginfo_t bull_mine_reginfo = {
		.name = "bull_mine",
		.datasize = sizeof(bull_t),
		.mobjinit = bull_mine_mobj_init,
		.mobjdone = MOBJ_FUNCTION_DONE_DEFAULT,
		.handle   = bull_mine_handle,
		.client_store = NULL,
		.client_restore = NULL,
		.entmodels_num = 1,
		.entmodels = bull_mine_models
};

/**
 * регистрация
 */
void mobj_bull_init()
{
	mobj_register(&bull_artillery_reginfo);
	mobj_register(&bull_missile_reginfo);
	mobj_register(&bull_mine_reginfo);
}
