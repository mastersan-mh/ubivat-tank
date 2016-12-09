/*
 * bull.c
 *
 *  Created on: 30 нояб. 2016 г.
 *      Author: mastersan
 */

#include "game.h"
#include "weap.h"
#include "mobjs.h"
#include "bull.h"
#include "explode.h"
#include "player.h"
#include "_gr2D.h"
#include "sound.h"

bullinfo_t bullinfo_table[__BULL_NUM] =
{
		{ 15,   7,  -1,  75, 2, IMG_BULL_ARTILLERY },
		{100,  50,  -1,  80, 8, IMG_BULL_MISSILE   },
		{200, 100, 100, -80, 8, IMG_BULL_MINE      }
};

static void * bull_artillery_mobj_init(mobj_t * this, mobj_t * parent);
static void bull_artillery_mobj_done(mobj_t * this);
static void bull_artillery_handle(mobj_t * this);

static const mobj_reginfo_t bull_artillery_reginfo = {
		.name = "bull_artillery",
		.mobjinit = bull_artillery_mobj_init,
		.mobjdone = bull_artillery_mobj_done,
		.handle   = bull_artillery_handle,
		.client_store = NULL,
		.client_restore = NULL
};

static void * bull_missile_mobj_init(mobj_t * this, mobj_t * parent);
static void bull_missile_mobj_done(mobj_t * this);
static void bull_missile_handle(mobj_t * this);

static const mobj_reginfo_t bull_missile_reginfo = {
		.name = "bull_missile",
		.mobjinit = bull_missile_mobj_init,
		.mobjdone = bull_missile_mobj_done,
		.handle   = bull_missile_handle,
		.client_store = NULL,
		.client_restore = NULL
};

static void * bull_mine_mobj_init(mobj_t * this, mobj_t * parent);
static void bull_mine_mobj_done(mobj_t * this);
static void bull_mine_handle(mobj_t * this);

static const mobj_reginfo_t bull_mine_reginfo = {
		.name = "bull_mine",
		.mobjinit = bull_mine_mobj_init,
		.mobjdone = bull_mine_mobj_done,
		.handle   = bull_mine_handle,
		.client_store = NULL,
		.client_restore = NULL
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


static void * bull_common_mobj_init(mobj_t * this, mobj_t * parent, bulltype_t bulltype)
{
	bull_t * bull = Z_malloc(sizeof(bull_t));
	bull->owner = parent;
	bull->type = bulltype;
	bull->delta_s = 0;
	bull->frame = 0;
	bullinfo_t * bullinfo = &bullinfo_table[bulltype];
	this->img = image_get(bullinfo->icon);

	if(bulltype == BULL_MISSILE) ((player_t *)parent->data)->bull = this;

	return bull;
}

static void bull_common_mobj_done(mobj_t * this)
{
	Z_free(this->data);
}

/*
 * поведение пули
 */
void bull_common_handle(mobj_t * this)
{
	bool Ul,Ur,Dl,Dr,Lu,Ld,Ru,Rd;
	float s;
	bool flag;
	mobj_t * player;

	bull_t * bull = this->data;

	bullinfo_t * bullinfo = &bullinfo_table[bull->type];

	s = bullinfo->speed * dtimed1000;

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
		mobj_new(
			mobj_type,
			this->pos.x,
			this->pos.y,
			this->dir,
			bull->owner
		);
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
		mobj_new(
			mobj_type,
			this->pos.x,
			this->pos.y,
			this->dir,
			bull->owner
		);
		this->erase = true;
		return;
	}

	//пуля не попала в стену
	player = map.mobjs;
	flag = 0;
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
		mobj_new(
			mobj_type,
			this->pos.x,
			this->pos.y,
			this->dir,
			bull->owner
		);
		this->erase = true;
		return;
	}

	//в игрока не попали, продолжение полета
	int mdlbox = this->img->img_sx;
	//количество кадров
	int Fmax = this->img->img_sy / (mdlbox * 4);
	bull->frame = bull->frame + c_bull_FPS * dtimed1000;
	if(bull->frame < 0 || Fmax <= bull->frame) bull->frame = 0;
}


/**
 * artillery
 */
void * bull_artillery_mobj_init(mobj_t * this, mobj_t * parent)
{
	return bull_common_mobj_init(this, parent, BULL_ARTILLERY);
}

void bull_artillery_mobj_done(mobj_t * this)
{
	bull_common_mobj_done(this->data);
}

void bull_artillery_handle(mobj_t * this)
{
	bull_common_handle(this);
}

/**
 * missile
 */
void * bull_missile_mobj_init(mobj_t * this, mobj_t * parent)
{
	return bull_common_mobj_init(this, parent, BULL_MISSILE);
}

void bull_missile_mobj_done(mobj_t * this)
{
	bull_common_mobj_done(this->data);
}

void bull_missile_handle(mobj_t * this)
{
	bull_common_handle(this);
}

/**
 * mine
 */
void * bull_mine_mobj_init(mobj_t * this, mobj_t * parent)
{
	return bull_common_mobj_init(this, parent, BULL_MINE);
}

void bull_mine_mobj_done(mobj_t * this)
{
	bull_common_mobj_done(this->data);
}

void bull_mine_handle(mobj_t * this)
{
	bull_common_handle(this);
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

	gr2D_setimage1(
		VEC_ROUND(cam->x+bull->pos.x-(cam->pos.x-cam->sx/2))-(mdlbox / 2),
		VEC_ROUND(cam->y-bull->pos.y+(cam->pos.y+cam->sy/2))-(mdlbox / 2),
		bull->img,
		0,
		mdlbox*(bull->dir * Fmax + trunc( ((bull_t*)bull->data)->frame) ),
		mdlbox,
		mdlbox
	);

}

