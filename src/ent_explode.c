/*
 * explode.c
 *
 *  Created on: 29 нояб. 2016 г.
 *      Author: mastersan
 */

#include "game.h"
#include "entity.h"
#include "model.h"
#include "ent_explode.h"
#include "ent_player.h"
#include "_gr2D.h"
#include "sound.h"

//оружия
explodeinfo_t explodeinfo_table[EXPLODE_NUM] =
{
		{  15,   7,  7, MAP_WALL_brick               , SOUND_EXPLODE_ARTILLERY},
		{ 100,  50, 11, MAP_WALL_brick | MAP_WALL_W1 , SOUND_EXPLODE_MISSILE  },
		{ 200, 100, 11, MAP_WALL_brick | MAP_WALL_W1 , SOUND_EXPLODE_GRENADE  }
};

static void explode_detonate(entity_t * this, const explodeinfo_t * explodeinfo)
{
	vec_t r;
	vec_t dx, dy;
	bool self;
	int ix,iy;

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
				if(MAP_WALL_CLIPPED(wall) && ( MAP_WALL_TEXTURE(wall) & explodeinfo->wall ) )
					map.map[y8][x8] = 0;
			}
		}
	}
	//проверка попаданий в игрока

	static char *list[] =
	{
		"player",
		"enemy",
		"boss"
	};

	int i;
	for(i = 0; i < 3; i++)
	{
	//пуля не попала в стену
		entity_t * attacked;
		ENTITIES_FOREACH(list[i], attacked)
		{
			dx = attacked->pos.x - this->pos.x;
			dy = attacked->pos.y - this->pos.y;
			if(
					(VEC_ABS(dx) <= c_p_MDL_box/2) &&
					(VEC_ABS(dy) <= c_p_MDL_box/2)
			)
				r = 0;
			else
			{
				r = VEC_SQRT(dx * dx + dy * dy) - VEC_SQRT(sqrf(c_p_MDL_box/2) + VEC_SQRT(c_p_MDL_box/2))/2;
			}
			if(r <= explodeinfo->radius)
			{
				//r = dx < dy ? dx : dy;
				//взрывом задели себя или товарища по команде(не для монстров)
				self = ( this->parent == attacked ) && ENTITY_IS(attacked, "player") ;
				player_getdamage(attacked, this, self, r, explodeinfo);
			}
		}

	}

}

static void explode_common_modelaction_endframef(entity_t * this, unsigned int imodel, char * actionname)
{
	if(this->parent)
	{
		if(ENT_PLAYER(this->parent)->bull == this)
		{
			ENT_PLAYER(this->parent)->bull = NULL;
		}
	}
	ENTITY_ERASE(this);
}

/*
 * explode_small
 */

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
				.modelname = "explode_small",
				.modelscale = 14.0f / 2.0f,
				.translation = { 0.0, 0.0 },
				.actions_num = 1,
				.actions = explode_small_modelactions
		}
};

/*
 * explode_big
 */

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
				.modelname = "explode_big",
				.modelscale = 22.0f / 2.0f,
				.translation = { 0.0, 0.0 },
				.actions_num = 1,
				.actions = explode_big_modelactions
		}
};

static void explode_common_init(entity_t * this, explodetype_t type)
{
	const explodeinfo_t * explodeinfo = &explodeinfo_table[type];

	sound_play_start(explodeinfo->soundIndex, 1);
	entity_model_play_start(this, 0, "explode");

	explode_detonate(this, explodeinfo);

}

static ENTITY_FUNCTION_INIT(explode_artillery_entity_init)
{
	explode_common_init(this, EXPLODE_ARTILLERY);
}

static ENTITY_FUNCTION_INIT(explode_missile_entity_init)
{
	explode_common_init(this, EXPLODE_MISSILE);
}

static ENTITY_FUNCTION_INIT(explode_mine_entity_init)
{
	explode_common_init(this, EXPLODE_MINE);
}

static const entityinfo_t explode_artillery_reginfo = {
		.name = "explode_artillery",
		.datasize = 0,
		.init = explode_artillery_entity_init,
		.done = ENTITY_FUNCTION_NONE,
		.handle = ENTITY_FUNCTION_NONE,
		.client_store = NULL,
		.client_restore = NULL,
		ENTITYINFO_ENTMODELS(explode_small_models)
};

static const entityinfo_t explode_missile_reginfo = {
		.name = "explode_missile",
		.datasize = 0,
		.init = explode_missile_entity_init,
		.done = ENTITY_FUNCTION_NONE,
		.handle = ENTITY_FUNCTION_NONE,
		.client_store = NULL,
		.client_restore = NULL,
		ENTITYINFO_ENTMODELS(explode_big_models)
};

static const entityinfo_t explode_mine_reginfo = {
		.name = "explode_mine",
		.datasize = 0,
		.init = explode_mine_entity_init,
		.done = ENTITY_FUNCTION_NONE,
		.handle = ENTITY_FUNCTION_NONE,
		.client_store = NULL,
		.client_restore = NULL,
		ENTITYINFO_ENTMODELS(explode_big_models)
};

void entity_explode_init(void)
{
	entity_register(&explode_artillery_reginfo);
	entity_register(&explode_missile_reginfo);
	entity_register(&explode_mine_reginfo);
}
