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
		{  15,   7, MAP_WALL_brick               , SOUND_EXPLODE_ARTILLERY},
		{ 100,  50, MAP_WALL_brick | MAP_WALL_W1 , SOUND_EXPLODE_MISSILE  },
		{ 200, 100, MAP_WALL_brick | MAP_WALL_W1 , SOUND_EXPLODE_GRENADE  }
};

static void explode_common_destroy_walls(entity_t * this, const explodeinfo_t * explodeinfo)
{
	vec_t ix;
	vec_t iy;

	vec_t halfbox = this->info->bodybox * 0.5f;

	//проверка попаданий в стены
	for(iy = -halfbox; iy <= halfbox; iy++)
	{
		for(ix = -halfbox; ix <= halfbox; ix++)
		{
			int x8 = VEC_TRUNC((this->origin[0] + ix) / 8);
			int y8 = VEC_TRUNC((this->origin[1] + iy) / 8);
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
}

static void explode_touch_common(entity_t * this, entity_t * that, const explodeinfo_t * explodeinfo)
{
	vec_t r;
	vec2_t d;
	bool self;

	vec_t that_halfbox = this->info->bodybox * 0.5f;
	VEC2_SUB(that->origin, this->origin, d);
	if(
			(VEC_ABS(d[0]) <= that_halfbox) &&
			(VEC_ABS(d[1]) <= that_halfbox)
	)
		r = 0;
	else
	{
		r = VEC_SQRT(DOT_PRODUCT2(d, d)) - VEC_SQRT(sqrf(that_halfbox) + VEC_SQRT(that_halfbox))/2;
	}
	if(r <= this->info->bodybox * 0.5f)
	{
		//r = dx < dy ? dx : dy;
		//взрывом задели себя или товарища по команде(не для монстров)
		self = ( this->parent == that ) && ENTITY_IS(that, "player") ;
		player_getdamage(that, this, self, r, explodeinfo);
	}

}

static void explode_common_modelaction_endframef(entity_t * this, unsigned int imodel, char * actionname)
{
    if(this->parent)
    {
        if(ENT_PLAYER(this->parent)->bull == this)
        {
            ENT_PLAYER(this->parent)->bull = NULL;
            /* вернуть камеру игроку */
            this->parent->cam_entity = this->parent;
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

static entitymodel_t explode_small_models[] =
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

static entitymodel_t explode_big_models[] =
{
		{
				.modelname = "explode_big",
				.modelscale = 22.0f / 2.0f,
				.translation = { 0.0, 0.0 },
				.actions_num = 1,
				.actions = explode_big_modelactions
		}
};

static void explode_detonate(entity_t * this, explodetype_t type)
{
	const explodeinfo_t * explodeinfo = &explodeinfo_table[type];

	sound_play_start(NULL, 0, explodeinfo->soundIndex, 1);
	entity_model_play_start(this, 0, "explode");

	explode_common_destroy_walls(this, explodeinfo);
	//проверка попаданий в игрока
	entity_t * attacked;

	ENTITIES_FOREACH("player", attacked)
	{
		explode_touch_common(this, attacked, explodeinfo);
	}
	ENTITIES_FOREACH("enemy", attacked)
	{
		explode_touch_common(this, attacked, explodeinfo);
	}
	ENTITIES_FOREACH("boss", attacked)
	{
		explode_touch_common(this, attacked, explodeinfo);
	}

}

static ENTITY_FUNCTION_INIT(explode_artillery_entity_init)
{
	explode_detonate(this, EXPLODE_ARTILLERY);
}

static ENTITY_FUNCTION_INIT(explode_missile_entity_init)
{
	explode_detonate(this, EXPLODE_MISSILE);
}

static ENTITY_FUNCTION_INIT(explode_mine_entity_init)
{
	explode_detonate(this, EXPLODE_MINE);
}

static const entityinfo_t explode_artillery_reginfo = {
		.name = "explode_artillery",
		.edatasize = 0,
		.bodybox = 7.0f * 2.0f,
		ENTITYINFO_ENTMODELS(explode_small_models),
		.init = explode_artillery_entity_init,
		.done = ENTITY_FUNCTION_NONE,
		.handle = ENTITY_FUNCTION_NONE,
		.client_store = NULL,
		.client_restore = NULL,
};

static const entityinfo_t explode_missile_reginfo = {
		.name = "explode_missile",
		.edatasize = 0,
		.bodybox = 11.0f * 2.0f,
		ENTITYINFO_ENTMODELS(explode_big_models),
		.init = explode_missile_entity_init,
		.done = ENTITY_FUNCTION_NONE,
		.handle = ENTITY_FUNCTION_NONE,
		.client_store = NULL,
		.client_restore = NULL,
};

static const entityinfo_t explode_mine_reginfo = {
		.name = "explode_mine",
		.edatasize = 0,
		.bodybox = 11.0f * 2.0f,
		ENTITYINFO_ENTMODELS(explode_big_models),
		.init = explode_mine_entity_init,
		.done = ENTITY_FUNCTION_NONE,
		.handle = ENTITY_FUNCTION_NONE,
		.client_store = NULL,
		.client_restore = NULL,
};

void entity_explode_init(void)
{
	entity_register(&explode_artillery_reginfo);
	entity_register(&explode_missile_reginfo);
	entity_register(&explode_mine_reginfo);
}
