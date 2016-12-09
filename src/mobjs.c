/*
 * mobjs.c
 *
 *  Created on: 29 нояб. 2016 г.
 *      Author: mastersan
 */

#include "game.h"
#include "mobjs.h"
#include "explode.h"
#include "bull.h"
#include "think.h"
#include "map.h"
#include "sound.h"

#include "_gr2D.h"

char *mobjnames[__MOBJ_NUM] =
{
		"MOBJ_SPAWN"  ,
		"MOBJ_ITEM"   ,
		"MOBJ_MESSAGE",
		"player"      ,
		"enemy"       ,
		"boss"        ,
		"bull_artillery",
		"bull_missile",
		"bull_mine",
		"explode_artillery",
		"explode_missile",
		"explode_mine",
		"MOBJ_EXIT"
};

static const mobj_reginfo_t ** mobj_reginfos = NULL;
static size_t mobj_register_size = 0;
static size_t mobj_register_num = 0;

void mobj_register(const mobj_reginfo_t * info)
{
	const mobj_reginfo_t ** tmp;

	if(mobj_register_size < mobj_register_num + 1)
	{
		if(mobj_register_size == 0)
			mobj_register_size = 1;
		else
			mobj_register_size *= 2;
		tmp = Z_realloc(mobj_reginfos, sizeof(mobj_reginfo_t*) * mobj_register_size);
		if(!tmp)game_halt("mobj_register(): failed");
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

void mobjs_handle()
{
	mobj_t * mobj;
	for(mobj = map.mobjs; mobj; mobj = mobj->next)
	{
		if(mobj->erase) continue;

		if(mobj->info != NULL && mobj->info->handle != NULL)
		{
			mobj->info->handle(mobj);
			continue;
		}

		switch(mobj->type)
		{
		case MOBJ_SPAWN: break;
		case MOBJ_ITEM: break;
		case MOBJ_MESSAGE: break;
		case MOBJ_EXIT: break;
		default: ;
		}
		if(mobj->erase)
		{

		}
	}
}

/*
 *
 */
void item_draw(camera_t * cam, mobj_t * mobj)
{
	if(!mobj->item.exist) return;
	gr2D_setimage0(
		VEC_ROUND(cam->x + mobj->pos.x - (cam->pos.x - cam->sx / 2)) + c_i_MDL_pos,
		VEC_ROUND(cam->y - mobj->pos.y + (cam->pos.y + cam->sy / 2)) + c_i_MDL_pos,
		mobj->img
	);
}


/*
 * рисование объектов на карте
 */
void mobjs_draw(camera_t * cam)
{
	mobj_t * mobj;
	for(mobj = map.mobjs;mobj;mobj = mobj->next)
	{
		if(mobj->erase) continue;

		if(
				mobj->img != NULL &&
				cam->pos.x - cam->sx / 2 <= mobj->pos.x + (c_i_MDL_box/2) &&
				mobj->pos.x - (c_i_MDL_box/2) <= cam->pos.x+cam->sx/2 &&
				cam->pos.y - cam->sy / 2 <= mobj->pos.y + (c_i_MDL_box/2) &&
				mobj->pos.y - (c_i_MDL_box/2) <= cam->pos.y + cam->sy/2
			)
		{

			switch(mobj->type)
			{
			case MOBJ_SPAWN:
				break;
			case MOBJ_ITEM:
				item_draw(cam, mobj);
				break;
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
				break;
			default: ;
			}
		}
	}
}

/*
 * добавление объекта
 */
mobj_t * mobj_new(mobj_type_t mobj_type, vec_t x, vec_t y, direction_t dir, mobj_t * const parent)
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

	mobj->next = map.mobjs;
	map.mobjs  = mobj;

	if(mobjinfo)
	{
		mobj->info = mobjinfo;
		if(mobj->info->mobjinit)
			mobj->data = mobj->info->mobjinit(mobj, parent);
	}

	return mobj;
}

void mobj_free_internal(mobj_t * mobj)
{
	if(mobj->info != NULL)
	{
		if(mobj->info->mobjdone == NULL)
		{
			Z_free(mobj->data);
		}
		else
		{
			mobj->info->mobjdone(mobj);
		}
	}

	switch(mobj->type)
	{
	case MOBJ_SPAWN  : break;
	case MOBJ_ITEM   : break;
	case MOBJ_MESSAGE: Z_free(mobj->mesage.message); break;
	case MOBJ_EXIT   : Z_free(mobj->exit.message); break;
	default: break;
	}
}

/**
 * удаление всех объектов
 */
void mobjs_erase_all()
{
	mobj_t * mobj;
	while(map.mobjs)
	{
		mobj = map.mobjs;
		map.mobjs = map.mobjs->next;
		mobj_free_internal(mobj);
		Z_free(mobj);
	}
}

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
