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
		game_console_send("Entity registration failed: Register data is NULL.");
		return;
	}

	if(info->name == NULL || strlen(info->name) == 0)
	{
		game_console_send("Entity registration failed: entity name is empty.");
		return;
	}

	game_console_send("Entity registration: \"%s\".", info->name);

	size_t i;
	for( i = 0; i < mobj_register_num; i++)
	{
		if(!strcmp(info->name, mobj_reginfos[i]->name))
		{
			game_console_send("Entity registration failed: duplicate entity name \"%s\"", info->name);
			return;
		}
	}

	if(info->datasize != 0 && info->mobjinit == NULL)
		game_console_send("Entity registration warning: possible, invalid register data in entity \"%s\", .mobjinit == NULL.", info->name);

	if(info->datasize == 0 && info->mobjinit != NULL)
		game_console_send("Entity registration warning: possible, invalid register data in entity \"%s\", .datasize == 0.", info->name);

	if(mobj_register_size < mobj_register_num + 1)
	{
		if(mobj_register_size == 0)
			mobj_register_size = 1;
		else
			mobj_register_size *= 2;
		tmp = Z_realloc(mobj_reginfos, sizeof(mobj_reginfo_t*) * mobj_register_size);
		if(!tmp)game_halt("Entity registration failed: out of memory");
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

		if(mobjinfo->models_num > 0)
			mobj->modelplayers = Z_malloc(mobjinfo->models_num * sizeof(ent_modelplayer_t));

		if(mobjinfo->datasize == 0)
			mobj->data = NULL;
		else
			mobj->data = Z_malloc(mobjinfo->datasize);
		if(mobjinfo->mobjinit)
			mobj->info->mobjinit(mobj, mobj->data, parent, args);

	}

	return mobj;
}

/*
 *
 */
static void mobj_freemem(mobj_t * mobj)
{
	if(mobj->info != NULL)
	{
		if(mobj->info->mobjdone != NULL)
			mobj->info->mobjdone(mobj, mobj->data);
		if(mobj->info->datasize)
			Z_free(mobj->data);
	}
	Z_free(mobj->modelplayers);
	Z_free(mobj);
}

void mobjs_erase()
{
	mobj_t * mobj;
	while(map.mobjs)
	{
		mobj = map.mobjs;
		map.mobjs = map.mobjs->next;
		mobj_freemem(mobj);
	}
}


/*
 * @return end-of-frames?
 */
static bool model_nextframe(float * frame, unsigned int fps, unsigned int startframe, unsigned int endframe)
{
	if(fps == 0) return true;

	float sf = startframe;
	float ef = endframe;
	if(*frame < sf) *frame = sf;

	*frame += fps * dtimed1000;
	if(*frame >= ef + 1) // последний кадр длится столько же, сколько и остальные кадры
	{
		*frame = ef;
		return true;
	}
	return false;
}

void mobjs_handle()
{
	mobj_t * mobj;
	for(mobj = map.mobjs; mobj; mobj = mobj->next)
	{
		if(mobj->erase) continue;
		if(!mobj->show) continue;

		const mobj_reginfo_t * info = mobj->info;
		if(!info) continue;

		if(info->handle != NULL)
		{
			info->handle(mobj);
		}

		int i;
		for(i = 0; i < info->models_num; i++)
		{

			if(
					mobj->modelplayers[i].action != NULL &&
					info->models[i].model->frames > 0 &&
					info->models[i].model->fps > 0
			)
			{

				bool end = model_nextframe(
					&mobj->modelplayers[i].frame,
					info->models[i].model->fps,
					mobj->modelplayers[i].action->startframe,
					mobj->modelplayers[i].action->endframe
				);
				if(end)
				{
					if(mobj->modelplayers[i].action->endframef != NULL)
						mobj->modelplayers[i].action->endframef(
							mobj,
							mobj->modelplayers[i].action->name
						);
					mobj->modelplayers[i].action = NULL;
				}

			}

		}

		if(mobj->erase)
		{
			/* do something */
		}

	}
}

static ent_modelaction_t * mobj_reginfo_action_get(const mobj_reginfo_t * info, unsigned int imodel, char * actionname)
{
	if(info->models == NULL)
		return NULL;
	ent_model_t * entmodel = &info->models[imodel];
	if(entmodel == NULL)
		return NULL;
	int i;
	for(i = 0; i < entmodel->actions_num; i++)
	{
		if(!strcmp(entmodel->actions[i].name, actionname))
			return &entmodel->actions[i];
	}
	return NULL;
}


void mobj_model_start_play(mobj_t * mobj, unsigned int imodel, char * actionname)
{
	const mobj_reginfo_t * info = mobj->info;
	ent_modelaction_t * action = mobj_reginfo_action_get(info, imodel, actionname);
	if(!action)
	{
		game_console_send("Error: Entity \"%s\", imodel %d: Action \"%s\" not found, could not play frames.",
			info->name,
			imodel,
			actionname
			);
		return;
	}
	mobj->modelplayers[imodel].action = action;
	mobj->modelplayers[imodel].frame = action->startframe;
}


#include "player.h"
#include "explode.h"
#include "bull.h"
#include "items.h"
#include "_gr2D.h"

/*
 * рисование игрока
 */
static void player_draw(camera_t * cam, mobj_t * player)
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

static void exit_draw(camera_t * cam, mobj_t * mobj)
{
	gr2D_setimage0(
		(cam->x + mobj->pos.x - (cam->pos.x - cam->sx / 2)) + c_i_MDL_pos,
		(cam->y - mobj->pos.y + (cam->pos.y + cam->sy / 2)) + c_i_MDL_pos,
		mobj->img
	);
}

static void ent_models_render(
	camera_t * cam,
	mobj_t * mobj
)
{
	static GLfloat angles[] =
	{
			0.0f,   /* N */
			180.0f, /* S */
			270.0f, /* W */
			90.0f,  /* E */
	};

	vec2_t pos = mobj->pos;
	const struct mobj_register_s * info = mobj->info;
	ent_model_t * ent_models = info->models;
	size_t models_num = info->models_num;


	if(!ent_models) return;

	int i;

	for( i = 0; i < models_num; i++ )
	{
		ent_model_t * ent_model = &ent_models[i];
		ent_modelplayer_t * modelplayer = &mobj->modelplayers[i];

		int frame = VEC_TRUNC(modelplayer->frame);
		model_render(
			cam,
			pos,
			ent_model->model,
			ent_model->modelscale,
			ent_model->translation,
			angles[DIR_RIGHT],
			frame
		);
	}
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
			ent_models_render(cam, mobj);

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
