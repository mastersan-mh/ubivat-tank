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


static const mobj_register_t ** mobj_registers = NULL;
static size_t mobj_register_size = 0;
static size_t mobj_register_num = 0;

void mobjinfo_register(const mobj_register_t * info)
{
	const mobj_register_t ** tmp;

	if(mobj_register_size < mobj_register_num + 1)
	{
		if(mobj_register_size == 0) mobj_register_size = 1;
		else mobj_register_size *= 2;
		tmp = Z_realloc(mobj_registers, mobj_register_size);
		if(!tmp)game_halt("mobj_register(): failed");
		mobj_registers = tmp;
	}
	mobj_registers[mobj_register_num] = info;
	mobj_register_num++;
}

const mobj_register_t * mobjinjfo_get(const char * name)
{
	size_t i;
	for(i = 0; i < mobj_register_num; i++)
	{
		if(!strncmp(mobj_registers[mobj_register_num]->name, name, 64))return mobj_registers[mobj_register_num];
	}
	return NULL;
}

mobj_bulltype_t mobj_weapon_type_to_bull_type(weapontype_t type)
{
	switch(type)
	{
	case WEAP_ARTILLERY: return EXPLODE_ARTILLERY;
	case WEAP_MISSILE  : return EXPLODE_MISSILE;
	case WEAP_MINE     : return EXPLODE_MINE;
	default: ;
	}
	return EXPLODE_ARTILLERY;
}

mobj_explodetype_t mobj_bull_type_to_explode_type(mobj_bulltype_t bull_type)
{
	switch(bull_type)
	{
	case BULL_ARTILLERY: return EXPLODE_ARTILLERY;
	case BULL_MISSILE  : return EXPLODE_MISSILE;
	case BULL_MINE     : return EXPLODE_MINE;
	default: ;
	}
	return EXPLODE_ARTILLERY;
}

void mobjs_handle()
{
	mobj_t * mobj;
	for(mobj = map.mobjs;mobj;mobj = mobj->next)
	{
		if(mobj->erase) continue;
		switch(mobj->type)
		{
		case MOBJ_SPAWN: break;
		case MOBJ_ITEM: break;
		case MOBJ_MESSAGE: break;
		case MOBJ_PLAYER :
		case MOBJ_ENEMY  :

			switch(mobj->player->charact.status)
			{
			case c_p_BOSS:
			case c_p_ENEMY:
				think_enemy(mobj);
				break;
			case c_p_P0:
				think_human(0, mobj);
				break;
			case c_p_P1:
				think_human(1, mobj);
				break;
			}
			player_handle(mobj);

			break;
		case MOBJ_BULL:
			bull_handle(mobj);
			break;
		case MOBJ_EXPLODE:
			explode_handle(mobj);
			break;
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
		round(cam->x + mobj->pos.x - (cam->pos.x - cam->sx / 2)) + c_i_MDL_pos,
		round(cam->y - mobj->pos.y + (cam->pos.y + cam->sy / 2)) + c_i_MDL_pos,
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
				cam->pos.y-cam->sy / 2 <= mobj->pos.y + (c_i_MDL_box/2) &&
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
			case MOBJ_BULL:
				bull_draw(cam, mobj);
				break;
			case MOBJ_EXPLODE:
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
mobj_t * mobj_new(mobj_type_t mobj_type, vec_t x, vec_t y, direction_t dir)
{
	mobj_t * mobj = Z_malloc(sizeof(mobj_t));

	mobj->erase = false;
	mobj->type = mobj_type;
	mobj->pos.x = x;
	mobj->pos.y = y;
	mobj->dir   = dir;

	mobj->next = map.mobjs;
	map.mobjs = mobj;
	return mobj;
}

void mobj_free_internal(mobj_t * mobj)
{
	switch(mobj->type)
	{
	case MOBJ_SPAWN  : break;
	case MOBJ_ITEM   : break;
	case MOBJ_MESSAGE: Z_free(mobj->mesage.message); break;
	case MOBJ_PLAYER :
	case MOBJ_ENEMY  :
		sound_play_stop(mobj->player->soundId_move);
		ctrl_AI_done(&(mobj->player->brain));
		Z_free(mobj->player);
		break;
	case MOBJ_BULL   : break;
	case MOBJ_EXPLODE: break;
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
