/*
 * bull.c
 *
 *  Created on: 30 нояб. 2016 г.
 *      Author: mastersan
 */

#include "mobjs.h"
#include "bull.h"
#include "explode.h"
#include "game.h"
#include "player.h"
#include "_gr2D.h"
#include "sound.h"

bullinfo_t bullinfo_table[__BULL_NUM] =
{
		{ 15,   7,  -1,  75, 2, IMG_BULL_ARTILLERY },
		{100,  50,  -1,  80, 8, IMG_BULL_MISSILE   },
		{200, 100, 100, -80, 8, IMG_BULL_MINE      }
};


/*
 * проверка на попадание в игрока
 */
static int checkdamage(mobj_t * player, mobj_t * bull)
{
	bullinfo_t * bullinfo = &bullinfo_table[bull->bull.type];

	if(
			bull->bull.owner != player && //попали не в себя
			0 < player->player->items[ITEM_HEALTH] &&
			(player->pos.x - c_p_MDL_box / 2 <= bull->pos.x + bullinfo->bodybox / 2)&&
			(bull->pos.x - bullinfo->bodybox / 2 <= player->pos.x + c_p_MDL_box / 2)&&
			(player->pos.y - c_p_MDL_box / 2 <= bull->pos.y + bullinfo->bodybox / 2)&&
			(bull->pos.y - bullinfo->bodybox / 2 <= player->pos.y + c_p_MDL_box / 2)
	) return true;
	return false;
};

/**
 *
 */
mobj_t * bull_new(vec_t x, vec_t y, mobj_bulltype_t bulltype, direction_t dir, mobj_t * owner)
{

	mobj_t * mobj = mobj_new(MOBJ_BULL, x, y, dir);

	mobj->bull.owner = owner;
	mobj->bull.type = bulltype;
	mobj->bull.delta_s = 0;
	mobj->bull.frame = 0;
	bullinfo_t * bullinfo = &bullinfo_table[bulltype];
	mobj->img = image_get(bullinfo->icon);

	return mobj;
}

/*
 * рисование пули
 */
void bull_draw(camera_t * cam, mobj_t * bull)
{
	//размер кадра X,Y
	int mdlbox = bull->img->img_sx;
	//количество кадров
	int Fmax = bull->img->img_sy / (mdlbox * 4);
	if
	(
			(cam->pos.x-cam->sx/2<=bull->pos.x+(mdlbox >> 1)) && (bull->pos.x-(mdlbox >> 1)<=cam->pos.x+cam->sx/2) &&
			(cam->pos.y-cam->sy/2<=bull->pos.y+(mdlbox >> 1)) && (bull->pos.y-(mdlbox >> 1)<=cam->pos.y+cam->sy/2)
	)
	{
		gr2D_setimage1(
				roundf(cam->x+bull->pos.x-(cam->pos.x-cam->sx/2))-(mdlbox >> 1),
				roundf(cam->y-bull->pos.y+(cam->pos.y+cam->sy/2))-(mdlbox >> 1),
				bull->img,
				0,
				mdlbox*(bull->dir * Fmax + trunc(bull->bull.frame)),
				mdlbox,
				mdlbox
		);
	};
};

/*
 * поведение пули
 */
void bull_handle(mobj_t * mobj)
{
	bool Ul,Ur,Dl,Dr,Lu,Ld,Ru,Rd;
	float s;
	bool flag;
	mobj_t * player;

	bullinfo_t * bullinfo = &bullinfo_table[mobj->bull.type];

	s = bullinfo->speed * dtimed1000;

	switch(mobj->dir)
	{
	case DIR_UP   : mobj->pos.y = mobj->pos.y + s; break;
	case DIR_DOWN : mobj->pos.y = mobj->pos.y - s; break;
	case DIR_LEFT : mobj->pos.x = mobj->pos.x - s; break;
	case DIR_RIGHT: mobj->pos.x = mobj->pos.x + s; break;
	};
	mobj->bull.delta_s = mobj->bull.delta_s+abs(s);                                //подсчитываем пройденный путь
	if(
			bullinfo->range > -1 && mobj->bull.delta_s > bullinfo->range
	)
	{//предельное расстояние пройдено
		explode_new(mobj->pos.x, mobj->pos.y, mobj_bull_type_to_explode_type(mobj->bull.type), mobj->bull.owner);
		mobj->erase = true;
		return;
	}


	//предел на расстояние не превышен
	map_clip_find(                                                     //найдем препятствия
		&mobj->pos,
		bullinfo->bodybox,
		c_m_w_w0 | c_m_w_w1 | c_m_w_brick,
		&Ul,&Ur,&Dl,&Dr,&Lu,&Ld,&Ru,&Rd
	);
	if((Ul || Ur || Dl || Dr || Lu || Ld || Ru || Rd) != 0)
	{     //пуля попала в стену
		explode_new(mobj->pos.x, mobj->pos.y, mobj_bull_type_to_explode_type(mobj->bull.type), mobj->bull.owner);
		mobj->erase = true;
		return;
	}

	//пуля не попала в стену
	player = map.mobjs;
	flag = 0;
	while(player && !flag)
	{                               //проверим на попадание в игрока
		if(player->type == MOBJ_PLAYER)
		{
			flag = checkdamage(player, mobj);
		}
		player = player->next;
	};
	if(flag)
	{                                              //попадание в игрока
		explode_new(mobj->pos.x, mobj->pos.y, mobj_bull_type_to_explode_type(mobj->bull.type), mobj->bull.owner);
		mobj->erase = true;
		return;
	}

	//в игрока не попали, продолжение полета
	int mdlbox = mobj->img->img_sx;
	//количество кадров
	int Fmax = mobj->img->img_sy / (mdlbox * 4);
	mobj->bull.frame = mobj->bull.frame + c_bull_FPS * dtimed1000;
	if(mobj->bull.frame < 0 || Fmax<=mobj->bull.frame) mobj->bull.frame = 0;
}
