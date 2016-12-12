/*
 * mobjs.c
 *
 *  Created on: 29 нояб. 2016 г.
 *      Author: mastersan
 */

#include "game.h"
#include "mobjs.h"
#include "map.h"

char *mobjnames[__MOBJ_NUM] =
{
		"spawn_player",
		"spawn_enemy" ,
		"spawn_boss"  ,
		"item_scores" ,
		"item_health" ,
		"item_armor"  ,
		"item_ammo_missile",
		"item_ammo_mine",
		"message"     ,
		"player"      ,
		"enemy"       ,
		"boss"        ,
		"bull_artillery",
		"bull_missile",
		"bull_mine",
		"explode_artillery",
		"explode_missile",
		"explode_mine",
		"exit"
};

static const mobj_reginfo_t ** mobj_reginfos = NULL;
static size_t mobj_register_size = 0;
static size_t mobj_register_num = 0;

void mobj_register(const mobj_reginfo_t * info)
{
	const mobj_reginfo_t ** tmp;
	if(info == NULL)
	{
		game_console_send("mobj_register(): Invalid register data is NULL.");
		return;
	}

	if(info->name == NULL || strlen(info->name) == 0)
	{
		game_console_send("mobj_register(): Invalid register data, name is empty.");
		return;
	}

	if(info->datasize != 0 && info->mobjinit == NULL)
		game_console_send("mobj_register(): Possible, invalid register data in entity \"%s\", .mobjinit == NULL.", info->name);

	if(info->datasize == 0 && info->mobjinit != NULL)
		game_console_send("mobj_register(): Possible, invalid register data in entity \"%s\", .datasize == 0.", info->name);

	if(mobj_register_size < mobj_register_num + 1)
	{
		if(mobj_register_size == 0)
			mobj_register_size = 1;
		else
			mobj_register_size *= 2;
		tmp = Z_realloc(mobj_reginfos, sizeof(mobj_reginfo_t*) * mobj_register_size);
		if(!tmp)game_halt("mobj_register(): failed.");
		mobj_reginfos = tmp;
	}
	mobj_reginfos[mobj_register_num] = info;
	mobj_register_num++;
}

const mobj_reginfo_t * mobj_reginfo_get(const char * name)
{
	size_t i;
	for(i = 0; i < mobj_register_num; i++)
	{
		if(!strncmp(mobj_reginfos[i]->name, name, 64))
			return mobj_reginfos[i];
	}
	return NULL;
}

mobj_t * mobj_new(mobj_type_t mobj_type, vec_t x, vec_t y, direction_t dir, const mobj_t * parent, const void * args)
{

	const mobj_reginfo_t * mobjinfo = NULL;
	if((int)mobj_type >= 0)
	{
		char * mobjname = mobjnames[mobj_type];
		mobjinfo = mobj_reginfo_get(mobjname);
		if(!mobjinfo)
		{
			game_console_send("Cannot create unknown entity \"%s\".", mobjname);
			return NULL;
		}

	}

	mobj_t * mobj = Z_malloc(sizeof(mobj_t));

	mobj->erase = false;
	mobj->type = mobj_type;
	mobj->pos.x = x;
	mobj->pos.y = y;
	mobj->dir   = dir;
	mobj->show  = true;

	mobj->next = map.mobjs;
	map.mobjs  = mobj;

	if(mobjinfo)
	{
		mobj->info = mobjinfo;
		if(mobjinfo->datasize == 0)
			mobj->data = NULL;
		else
			mobj->data = Z_malloc(mobjinfo->datasize);
		if(mobjinfo->mobjinit)
			mobj->info->mobjinit(mobj, mobj->data, parent, args);
	}

	return mobj;
}

void mobjs_handle()
{
	mobj_t * mobj;
	for(mobj = map.mobjs; mobj; mobj = mobj->next)
	{
		if(mobj->erase) continue;
		if(!mobj->show) continue;

		if(mobj->info != NULL && mobj->info->handle != NULL)
		{
			mobj->info->handle(mobj);
			continue;
		}

		if(mobj->erase)
		{
			/* do something */
		}
	}
}

/*
 *
 */
static void mobj_erase_internal(mobj_t * mobj)
{
	if(mobj->info != NULL)
	{
		if(mobj->info->mobjdone != NULL)
			mobj->info->mobjdone(mobj, mobj->data);
		if(mobj->info->datasize)
			Z_free(mobj->data);
	}
}

void mobjs_erase_all()
{
	mobj_t * mobj;
	while(map.mobjs)
	{
		mobj = map.mobjs;
		map.mobjs = map.mobjs->next;
		mobj_erase_internal(mobj);
		Z_free(mobj);
	}
}


#include "player.h"
#include "explode.h"
#include "bull.h"
#include "items.h"
#include "_gr2D.h"

/*
 * рисование игрока
 */
void player_draw(camera_t * cam, mobj_t * player)
{
	player_t * pl = player->data;
	if(pl->items[ITEM_HEALTH] > 0)
	{
		//если игрок жив
		gr2D_setimage1(
			(cam->x+player->pos.x-(cam->pos.x-cam->sx/2))+c_p_MDL_pos,
			(cam->y-player->pos.y+(cam->pos.y+cam->sy/2))+c_p_MDL_pos,
			player->img,
			0,
			c_p_MDL_box*((player->dir * 4)+VEC_ROUND(pl->Fbase)),
			c_p_MDL_box,
			c_p_MDL_box
		);
		gr2D_setimage0(
			(cam->x+player->pos.x-(cam->pos.x-cam->sx/2))+c_p_MDL_pos,
			(cam->y-player->pos.y+(cam->pos.y+cam->sy/2))+c_p_MDL_pos,
			pl->Iflag
		);
	}
}

void exit_draw(camera_t * cam, mobj_t * mobj)
{
	gr2D_setimage0(
		(cam->x + mobj->pos.x - (cam->pos.x - cam->sx / 2)) + c_i_MDL_pos,
		(cam->y - mobj->pos.y + (cam->pos.y + cam->sy / 2)) + c_i_MDL_pos,
		mobj->img
	);
}

/*
 * рисование объектов на карте
 */
void mobjs_render(camera_t * cam)
{
	int cam_sx_half = cam->sx / 2;
	int cam_sy_half = cam->sy / 2;
	mobj_t * mobj;
	for(mobj = map.mobjs;mobj;mobj = mobj->next)
	{
		if(mobj->erase) continue;
		if(!mobj->show) continue;



		//int viewbox_half = mobj->img->img_sx;
		int viewbox_half = 16;

		if(
				( cam->pos.x  - cam_sx_half <= mobj->pos.x + viewbox_half ) &&
				( mobj->pos.x - viewbox_half <= cam->pos.x  + cam_sx_half ) &&
				( cam->pos.y  - cam_sy_half <= mobj->pos.y + viewbox_half ) &&
				( mobj->pos.y - viewbox_half <= cam->pos.y  + cam_sy_half )
		)
		{

			ent_models_render(cam, mobj->pos, mobj->info->models);

			if(mobj->img == NULL) continue;

			switch(mobj->type)
			{
			case MOBJ_SPAWN_PLAYER:
			case MOBJ_SPAWN_ENEMY:
			case MOBJ_SPAWN_BOSS:
				break;
			case MOBJ_ITEM_SCORES:
			case MOBJ_ITEM_HEALTH:
			case MOBJ_ITEM_ARMOR :
			case MOBJ_ITEM_AMMO_MISSILE:
			case MOBJ_ITEM_AMMO_MINE:
			case MOBJ_MESSAGE:
				break;
			case MOBJ_PLAYER :
			case MOBJ_ENEMY  :
				player_draw(cam, mobj);
				break;
			case MOBJ_BULL_ARTILLERY:
			case MOBJ_BULL_MISSILE:
			case MOBJ_BULL_MINE:
				bull_draw(cam, mobj);
				break;
			case MOBJ_EXPLODE_ARTILLERY:
			case MOBJ_EXPLODE_MISSILE:
			case MOBJ_EXPLODE_MINE:
				explode_draw(cam, mobj);
				break;
			case MOBJ_EXIT:
				exit_draw(cam, mobj);
				break;
			default: ;
			}
		}
	}
}

/*
 * добавление объекта
 */
/**
 * удаление всех объектов
 */
/*
 * удаление взрыва из списка, после удаления
 *        explode указывает на предыдущий эл-т если
 *        он существует, если нет - то на следующий
 */
/*
void explode_remove(mobj_t ** mobj)
{
	mobj_t * p;

	if (explList && *explode != NULL)
	{
		p = explList;
		if(explList== *explode) {                                    //если HEAD и explode совпадают
			explList = p->next;
			*explode          = explList;
			Z_free(p);
			p = NULL;
		}
		else
		{
			if(p->next)
			{                                         //если в списке не один эл-т
				while(p && p->next != *explode) p = p->next;                     //находим ссылку на нужный эл-т
				p->next = (*explode)->next;
				Z_free(*explode);                                                    //удаляем его
				*explode = p;                                                          //bull указ. на пред. эл-т
			}
		}
	}
}
*/
