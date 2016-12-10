/*
 * explode.c
 *
 *  Created on: 29 нояб. 2016 г.
 *      Author: mastersan
 */

#include "mobjs.h"
#include "explode.h"
#include "game.h"
#include "player.h"
#include "_gr2D.h"
#include "sound.h"

//оружия
explodeinfo_t explodeinfo_table[__EXPLODE_NUM] =
{
		{  15,   7,  7},
		{ 100,  50, 11},
		{ 200, 100, 11}
};

static MOBJ_FUNCTION_INIT(explode_artillery_mobj_init);
static void explode_artillery_handle(mobj_t * this);

static const mobj_reginfo_t explode_artillery_reginfo = {
		.name = "explode_artillery",
		.datasize = sizeof(explode_t),
		.mobjinit = explode_artillery_mobj_init,
		.mobjdone = MOBJ_FUNCTION_DONE_DEFAULT,
		.handle   = explode_artillery_handle,
		.client_store = NULL,
		.client_restore = NULL
};

static MOBJ_FUNCTION_INIT(explode_missile_mobj_init);
static void explode_missile_handle(mobj_t * this);

static const mobj_reginfo_t explode_missile_reginfo = {
		.name = "explode_missile",
		.datasize = sizeof(explode_t),
		.mobjinit = explode_missile_mobj_init,
		.mobjdone = MOBJ_FUNCTION_DONE_DEFAULT,
		.handle   = explode_missile_handle,
		.client_store = NULL,
		.client_restore = NULL
};

static MOBJ_FUNCTION_INIT(explode_mine_mobj_init);
static void explode_mine_handle(mobj_t * this);

static const mobj_reginfo_t explode_mine_reginfo = {
		.name = "explode_mine",
		.datasize = sizeof(explode_t),
		.mobjinit = explode_mine_mobj_init,
		.mobjdone = MOBJ_FUNCTION_DONE_DEFAULT,
		.handle   = explode_mine_handle,
		.client_store = NULL,
		.client_restore = NULL
};

void mobj_explode_init()
{
	mobj_register(&explode_artillery_reginfo);
	mobj_register(&explode_missile_reginfo);
	mobj_register(&explode_mine_reginfo);
}


static void explode_common_init(mobj_t * this, explode_t * explode, const mobj_t * parent, explodetype_t type)
{
	static image_index_t img_list[] =
	{
			IMG_EXPLODE_SMALL,
			IMG_EXPLODE_BIG,
			IMG_EXPLODE_BIG
	};
	static sound_index_t sound_list[] =
	{
			SOUND_EXPLODE_ARTILLERY,
			SOUND_EXPLODE_MISSILE,
			SOUND_EXPLODE_GRENADE
	};

	explode->owner = (mobj_t *)parent;
	explode->type  = type;
	explode->frame = -1;
	this->img = image_get(img_list[type]);
	sound_play_start(sound_list[type], 1);
}

static void explode_common_handle(mobj_t * this);

MOBJ_FUNCTION_INIT(explode_artillery_mobj_init)
{
	explode_common_init(this, thisdata, parent, EXPLODE_ARTILLERY);
}

void explode_artillery_handle(mobj_t * this)
{
	explode_common_handle(this);
}

MOBJ_FUNCTION_INIT(explode_missile_mobj_init)
{
	explode_common_init(this, thisdata,  parent, EXPLODE_MISSILE);
}

void explode_missile_handle(mobj_t * this)
{
	explode_common_handle(this);
}

MOBJ_FUNCTION_INIT(explode_mine_mobj_init)
{
	explode_common_init(this,  thisdata, parent, EXPLODE_MINE);
}

void explode_mine_handle(mobj_t * this)
{
	explode_common_handle(this);
}





void explode_common_handle(mobj_t * this)
{
	explode_t * explode = this->data;

	mobj_t * player;
	vec_t r;
	vec_t dx, dy;
	bool self;
	int ix,iy;
	char wall;
	char wall_type;

	explodeinfo_t * explodeinfo = &explodeinfo_table[explode->type];

	if(explode->frame == -1)
	{
		explode->frame = 0;
		//проверка попаданий в стены
		for(iy = -explodeinfo->radius; iy <= explodeinfo->radius; iy++)
		{
			for(ix = -explodeinfo->radius; ix <= explodeinfo->radius; ix++)
			{
				vec_t x = VEC_TRUNC((this->pos.x+ix)/8);
				vec_t y = VEC_TRUNC((this->pos.y+iy)/8);

				if(
						0 <= x && x <= MAP_SX &&
						0 <= y && y <= MAP_SY
				)
				{
					int x8 = x;
					int y8 = y;
					wall = map.map[y8][x8];
					if(wall & c_m_f_clip)
					{                   //присутствует зажим
						wall_type = wall & 0x0F;
						if((wall_type | c_m_w_w0 | c_m_water) != (c_m_w_w0 | c_m_water))
						{//если не броня0 и не вода
							if(
									(wall_type == c_m_w_brick) ||//если кирпич
									(
											/* если броня1 */
											(wall_type == c_m_w_w1) &&
											(
													explode->type == EXPLODE_MISSILE ||
													explode->type == EXPLODE_MINE
											)
									)
							)
							{
								map.map[y8][x8] = 0;
							}
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
	if(c_explode_Famnt - 1 < explode->frame) explode->frame = c_explode_Famnt - 1;

	explode->frame = explode->frame + c_explode_FPS * dtimed1000;

	if(explode->frame > c_explode_Famnt - 1)
		this->erase = true;
}


/*
 * добавление взрыва
 */
/*
 * отрисовка
 */
void explode_draw(camera_t * cam, mobj_t * explode)
{
	int mdlbox;

	mdlbox = explode->img->img_sx;
	gr2D_setimage1(
		VEC_ROUND(cam->x + explode->pos.x - (cam->pos.x - cam->sx / 2)) - (mdlbox / 2),
		VEC_ROUND(cam->y - explode->pos.y + (cam->pos.y + cam->sy / 2)) - (mdlbox / 2),
		explode->img,
		0,
		mdlbox * VEC_TRUNC(EXPLODE(explode)->frame),
		mdlbox,
		mdlbox
	);
}
