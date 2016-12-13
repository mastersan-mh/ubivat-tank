/*
 * explode.c
 *
 *  Created on: 29 нояб. 2016 г.
 *      Author: mastersan
 */

#include "mobjs.h"
#include "model.h"
#include "explode.h"
#include "game.h"
#include "player.h"
#include "_gr2D.h"
#include "sound.h"

//скорость проигрывания кадров взрыва
#define EXPLODE_FPS   28
//количество кадров взрыва
#define EXPLODE_FRAMES_NUM 8

//оружия
explodeinfo_t explodeinfo_table[__EXPLODE_NUM] =
{
		{  15,   7,  7},
		{ 100,  50, 11},
		{ 200, 100, 11}
};


static void explode_detonate(mobj_t * this)
{
	explode_t * explode = this->data;

	mobj_t * player;
	vec_t r;
	vec_t dx, dy;
	bool self;
	int ix,iy;

	explodeinfo_t * explodeinfo = &explodeinfo_table[explode->type];

	//проверка попаданий в стены
	for(iy = -explodeinfo->radius; iy <= explodeinfo->radius; iy++)
	{
		for(ix = -explodeinfo->radius; ix <= explodeinfo->radius; ix++)
		{
			int x8 = VEC_TRUNC((this->pos.x + ix) / 8);
			int y8 = VEC_TRUNC((this->pos.y + iy) / 8);
			if(
					0 <= x8 && x8 < MAP_SX &&
					0 <= y8 && y8 < MAP_SY
			)
			{
				char wall = map.map[y8][x8];

				if(MAP_WALL_CLIPPED(wall)) /* присутствует зажим */
				{
					switch(MAP_WALL_TEXTURE(wall))
					{
						case c_m_w_brick:
							map.map[y8][x8] = 0;
							break;
						case c_m_w_w1   :
							if( explode->type == EXPLODE_MISSILE || explode->type == EXPLODE_MINE )
								map.map[y8][x8] = 0;
							break;
						case c_m_w_w0   :
						case c_m_water  :
							break;
					}
				}
			}
		}
	}
	//проверка попаданий в игрока
	for(player = map.mobjs; player ; player = player->next)
	{
		if(
				player->type != MOBJ_PLAYER &&
				player->type != MOBJ_ENEMY &&
				player->type != MOBJ_BOSS
		)
		{
			continue;
		}
		dx = player->pos.x - this->pos.x;
		dy = player->pos.y - this->pos.y;
		if(
				(VEC_ABS(dx) <= c_p_MDL_box/2) &&
				(VEC_ABS(dy) <= c_p_MDL_box/2)
		)
			r = 0;
		else
		{
			r = VEC_SQRT(dx * dx + dy * dy) - VEC_SQRT(sqrf(c_p_MDL_box/2) + VEC_SQRT(c_p_MDL_box/2))/2;
		};
		if(r <= explodeinfo->radius)
		{
			//r = dx < dy ? dx : dy;
			//взрывом задели себя или товарища по команде(не для монстров)
			self = (explode->owner == player) && (player->type == MOBJ_PLAYER);
			player_getdamage(player, this, self, r);
		}
	}

}

static void explode_common_modelaction_endframef(mobj_t * this, char * actionname)
{
	if(EXPLODE(this)->owner)
	{

		if(ENT_PLAYER(EXPLODE(this)->owner)->bull == this)
		{
			ENT_PLAYER(EXPLODE(this)->owner)->bull = NULL;
		}
	}
	MOBJ_ERASE(this);
}

static MODEL_COMMON_VERTEX4_TEXCOORD_FRAME(8frame0 , 0, 8);
static MODEL_COMMON_VERTEX4_TEXCOORD_FRAME(8frame1 , 1, 8);
static MODEL_COMMON_VERTEX4_TEXCOORD_FRAME(8frame2 , 2, 8);
static MODEL_COMMON_VERTEX4_TEXCOORD_FRAME(8frame3 , 3, 8);
static MODEL_COMMON_VERTEX4_TEXCOORD_FRAME(8frame4 , 4, 8);
static MODEL_COMMON_VERTEX4_TEXCOORD_FRAME(8frame5 , 5, 8);
static MODEL_COMMON_VERTEX4_TEXCOORD_FRAME(8frame6 , 6, 8);
static MODEL_COMMON_VERTEX4_TEXCOORD_FRAME(8frame7 , 7, 8);

/*
 * explode_small
 */

static modelframe_t model_explode_small_frames[8] =
{
		{ .texcoord = model_texcoord_8frame0 },
		{ .texcoord = model_texcoord_8frame1 },
		{ .texcoord = model_texcoord_8frame2 },
		{ .texcoord = model_texcoord_8frame3 },
		{ .texcoord = model_texcoord_8frame4 },
		{ .texcoord = model_texcoord_8frame5 },
		{ .texcoord = model_texcoord_8frame6 },
		{ .texcoord = model_texcoord_8frame7 }
};

static const model_t mdl_explode_small =
{
		.name = "explode_small",
		.vertexes_num = VERTEXES_COMMON_NUM,
		.vertexes = model_vertexes_common,
		.triangles_num = TRIANGLES_COMMON_NUM,
		.triangles = model_trianges_common,
		.itexture = IMG_EXPLODE_SMALL,
		.fps = EXPLODE_FPS,
		.frames_num = EXPLODE_FRAMES_NUM,
		.frames = model_explode_small_frames
};

static const ent_modelaction_t explode_small_modelactions[] =
{
		{
				.name = "explode",
				.startframe = 0,
				.endframe = 7,
				.endframef = explode_common_modelaction_endframef
		}
};

static entmodel_t explode_small_models[] =
{
		{
				.model = &mdl_explode_small,
				.modelscale = 14.0f / 2.0f,
				.translation = { 0.0, 0.0 },
				.actions_num = 1,
				.actions = explode_small_modelactions
		}
};

/*
 * explode_big
 */
static modelframe_t model_explode_big_frames[8] =
{
		{ .texcoord = model_texcoord_8frame0 },
		{ .texcoord = model_texcoord_8frame1 },
		{ .texcoord = model_texcoord_8frame2 },
		{ .texcoord = model_texcoord_8frame3 },
		{ .texcoord = model_texcoord_8frame4 },
		{ .texcoord = model_texcoord_8frame5 },
		{ .texcoord = model_texcoord_8frame6 },
		{ .texcoord = model_texcoord_8frame7 }
};

static const model_t mdl_explode_big =
{
		.name = "explode_big",
		.vertexes_num = VERTEXES_COMMON_NUM,
		.vertexes = model_vertexes_common,
		.triangles_num = TRIANGLES_COMMON_NUM,
		.triangles = model_trianges_common,
		.itexture = IMG_EXPLODE_BIG,
		.fps = EXPLODE_FPS,
		.frames_num = EXPLODE_FRAMES_NUM,
		.frames = model_explode_big_frames
};

static const ent_modelaction_t explode_big_modelactions[] =
{
		{
				.name = "explode",
				.startframe = 0,
				.endframe = 7,
				.endframef = explode_common_modelaction_endframef
		}
};

static entmodel_t explode_big_models[] =
{
		{
				.model = &mdl_explode_big,
				.modelscale = 22.0f / 2.0f,
				.translation = { 0.0, 0.0 },
				.actions_num = 1,
				.actions = explode_big_modelactions
		}
};

static void explode_common_init(mobj_t * this, explode_t * explode, const mobj_t * parent, explodetype_t type)
{
	static sound_index_t sound_list[] =
	{
			SOUND_EXPLODE_ARTILLERY,
			SOUND_EXPLODE_MISSILE,
			SOUND_EXPLODE_GRENADE
	};

	explode->owner = (mobj_t *)parent;
	explode->type  = type;

	sound_play_start(sound_list[type], 1);
	mobj_model_start_play(this, 0, "explode");

	explode_detonate(this);

}

static MOBJ_FUNCTION_INIT(explode_artillery_mobj_init)
{
	explode_common_init(this, thisdata, parent, EXPLODE_ARTILLERY);
}

static MOBJ_FUNCTION_INIT(explode_missile_mobj_init)
{
	explode_common_init(this, thisdata,  parent, EXPLODE_MISSILE);
}

static MOBJ_FUNCTION_INIT(explode_mine_mobj_init)
{
	explode_common_init(this,  thisdata, parent, EXPLODE_MINE);
}

static const mobj_reginfo_t explode_artillery_reginfo = {
		.name = "explode_artillery",
		.datasize = sizeof(explode_t),
		.mobjinit = explode_artillery_mobj_init,
		.mobjdone = MOBJ_FUNCTION_DONE_DEFAULT,
		.handle   = MOBJ_FUNCTION_HANDLE_DEFAULT,
		.client_store = NULL,
		.client_restore = NULL,
		.entmodels_num = 1,
		.entmodels = explode_small_models
};

static const mobj_reginfo_t explode_missile_reginfo = {
		.name = "explode_missile",
		.datasize = sizeof(explode_t),
		.mobjinit = explode_missile_mobj_init,
		.mobjdone = MOBJ_FUNCTION_DONE_DEFAULT,
		.handle   = MOBJ_FUNCTION_HANDLE_DEFAULT,
		.client_store = NULL,
		.client_restore = NULL,
		.entmodels_num = 1,
		.entmodels = explode_big_models
};

static const mobj_reginfo_t explode_mine_reginfo = {
		.name = "explode_mine",
		.datasize = sizeof(explode_t),
		.mobjinit = explode_mine_mobj_init,
		.mobjdone = MOBJ_FUNCTION_DONE_DEFAULT,
		.handle   = MOBJ_FUNCTION_HANDLE_DEFAULT,
		.client_store = NULL,
		.client_restore = NULL,
		.entmodels_num = 1,
		.entmodels = explode_big_models
};

void mobj_explode_init()
{
	mobj_register(&explode_artillery_reginfo);
	mobj_register(&explode_missile_reginfo);
	mobj_register(&explode_mine_reginfo);
}
