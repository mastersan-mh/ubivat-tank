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

/*
 * обработчик
 */
mobj_t * explode_new(vec_t x, vec_t y, mobj_explodetype_t explode_type, mobj_t * owner)
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
	mobj_t * mobj = mobj_new(MOBJ_EXPLODE, x, y, DIR_UP);

	mobj->explode.owner = owner;
	mobj->explode.type  = explode_type;
	mobj->explode.frame = -1;
	mobj->img = image_get(img_list[explode_type]);
	sound_play_start(sound_list[explode_type], 1);

	return mobj;
}




void explode_handle(mobj_t * mobj)
{
	mobj_t * explode = mobj;
	mobj_t * player;
	float r,sp_x,sp_y;
	bool self;
	int x,y;
	char wall;
	char wall_type;

	explodeinfo_t * explodeinfo = &explodeinfo_table[explode->explode.type];

	if(explode->explode.frame == -1)
	{
		explode->explode.frame = 0;
		//проверка попаданий в стены
		for(y = -explodeinfo->radius; y<= explodeinfo->radius; y++)
		{
			for(x = -explodeinfo->radius; x<= explodeinfo->radius; x++)
			{
				if(
						(0<=trunc((explode->pos.x+x)/8)) && (trunc((explode->pos.x+x)/8)<=MAP_SX) &&
						(0<=trunc((explode->pos.y+y)/8)) && (trunc((explode->pos.y+y)/8)<=MAP_SY)
				)
				{
					wall = map.map[(int)trunc((explode->pos.y+y)/8)][(int)trunc((explode->pos.x+x)/8)];
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
													explode->explode.type == EXPLODE_MISSILE ||
													explode->explode.type == EXPLODE_MINE
											)
									)
							)
							{
								map.map[(int)trunc((explode->pos.y+y)/8)][(int)trunc((explode->pos.x+x)/8)] = 0;
							}
						}
					}
				}
			}
		}
		//проверка попаданий в игрока
		player = map.mobjs;
		while(player)
		{
			if(player->type != MOBJ_PLAYER)
			{
				player = player->next;
				continue;
			}
			sp_x = player->pos.x-explode->pos.x;
			sp_y = player->pos.y-explode->pos.y;
			if(
					(abs(sp_x)<=c_p_MDL_box/2) &&
					(abs(sp_y)<=c_p_MDL_box/2)) r = 0;
			else
			{
				r = sqrt(sp_x*sp_x+sp_y*sp_y)-sqrt(sqrf(c_p_MDL_box/2)+sqrf(c_p_MDL_box/2))/2;
				//if(r <= explodeinfo->bullbox) r = 0;
			};
			if(0 < explodeinfo->radius && r <= explodeinfo->radius)
			{
				if(
						(explode->explode.owner == player) ||
						(
								(explode->explode.owner == game.P0 || explode->explode.owner == game.P1)&&
								(player == game.P0 || player == game.P1)
						)
				)
					self = true;          //взрывом задели себя или товарища по команде(не для монстров)
				else self = false;                                                  //взрывом задели другого игрока
				player_getdamage(player, explode, self, r);
			};
			player = player->next;
		};
	};
	if(c_explode_Famnt - 1 < explode->explode.frame) explode->explode.frame = c_explode_Famnt - 1;

	explode->explode.frame = explode->explode.frame + c_explode_FPS * dtimed1000;

	if(explode->explode.frame > c_explode_Famnt - 1) explode->erase = true;
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
	if(
			(cam->pos.x-cam->sx/2<=explode->pos.x+(mdlbox >> 1)) &&
			(explode->pos.x-(mdlbox >> 1)<=cam->pos.x+cam->sx/2) &&
			(cam->pos.y-cam->sy/2<=explode->pos.y+(mdlbox >> 1)) &&
			(explode->pos.y-(mdlbox >> 1)<=cam->pos.y+cam->sy/2)
			)
	{
		gr2D_setimage1(
			roundf(cam->x + explode->pos.x - (cam->pos.x - cam->sx / 2)) - (mdlbox / 2),
			roundf(cam->y - explode->pos.y + (cam->pos.y + cam->sy / 2)) - (mdlbox / 2),
			explode->img,
			0,
			mdlbox * trunc(explode->explode.frame),
			mdlbox,
			mdlbox
		);
	}
}
