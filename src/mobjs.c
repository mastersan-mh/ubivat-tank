/*
 * mobjs.c
 *
 *  Created on: 29 нояб. 2016 г.
 *      Author: mastersan
 */

#include "game.h"
#include "mobjs.h"
#include "model.h"

#define LIST2_PUSH(enttop, ent) \
		do                      \
		{                       \
			(ent)->prev = NULL;     \
			(ent)->next = (enttop); \
			if( (enttop) != NULL ) (enttop)->prev = (ent); \
			(enttop) = (ent);       \
		}while(0)

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

/* действительные entities */
mobj_t * mobjs = NULL;

/* удалённые entities */
mobj_t * mobjs_erased = NULL;

static const mobj_reginfo_t ** mobj_reginfos = NULL;
static size_t mobj_register_size = 0;
static size_t mobj_register_num = 0;

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


	if(mobj_reginfo_get(info->name) != NULL)
	{
		game_console_send("Entity registration failed: duplicate name \"%s\"", info->name);
		return;
	}

	if(info->datasize != 0 && info->mobjinit == NULL)
		game_console_send("Entity registration warning: entity \"%s\" invalid register data: .mobjinit == NULL.", info->name);

	if(info->datasize == 0 && info->mobjinit != NULL)
		game_console_send("Entity registration warning: entity \"%s\" invalid register data: .datasize == 0.", info->name);

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

/**
 * @description установить модель на entity
 */
int mobj_model_set(mobj_t * mobj, unsigned int imodel, char * modelname)
{
	if(imodel >= mobj->info->entmodels_num) return -1;
	mobj->modelplayers[imodel].model = model_get(modelname);
	mobj->modelplayers[imodel].frame = 0;
	return 0;
}

/**
 * @description добавление объекта
 */
mobj_t * mobj_new(mobj_type_t mobj_type, vec_t x, vec_t y, direction_t dir, const mobj_t * parent, const void * args)
{
	int i;
	const mobj_reginfo_t * mobjinfo = NULL;

	if((int)mobj_type < 0)
	{
		game_console_send("Error: Cannot create unknown entity with mobj_type \"%d\".", mobj_type);
		return NULL;
	}

	char * mobjname = mobjnames[mobj_type];
	mobjinfo = mobj_reginfo_get(mobjname);
	if(!mobjinfo)
	{
		game_console_send("Error: Cannot create unknown entity \"%s\".", mobjname);
		return NULL;
	}


	mobj_t * mobj = Z_malloc(sizeof(mobj_t));

	mobj->erase = false;
	mobj->type = mobj_type;
	mobj->pos.x = x;
	mobj->pos.y = y;
	mobj->dir   = dir;
	mobj->show  = true;

	mobj->info = mobjinfo;

	LIST2_PUSH(mobjs, mobj);

	if(mobjinfo->entmodels_num > 0)
	{
		mobj->modelplayers = Z_malloc(mobjinfo->entmodels_num * sizeof(ent_modelplayer_t));

		for( i = 0; i < mobjinfo->entmodels_num; i++)
		{
			mobj_model_set(mobj, i, mobjinfo->entmodels[i].modelname);
		}
	}

	if(mobjinfo->datasize == 0)
		mobj->data = NULL;
	else
		mobj->data = Z_malloc(mobjinfo->datasize);
	if(mobjinfo->mobjinit)
		mobj->info->mobjinit(mobj, mobj->data, parent, args);

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

/**
 * @description удаление всех объектов
 */
void mobjs_erase()
{
	mobj_t * mobj;
	while(mobjs)
	{
		mobj = mobjs;
		mobjs = mobjs->next;
		mobj_freemem(mobj);
	}
	while(mobjs_erased)
	{
		mobj = mobjs_erased;
		mobjs_erased = mobjs_erased->next;
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
	int i;
	mobj_t * mobj;
	for(mobj = mobjs; mobj; )
	{

		if(mobj->erase)
		{

			if(mobjs == mobj)
				mobjs = mobjs->next;

			mobj_t * erased = mobj;
			mobj = mobj->next;
			if(erased->prev)
				erased->prev->next = erased->next;
			if(erased->next)
				erased->next->prev = erased->prev;

			mobj_model_play_pause_all(erased);

			LIST2_PUSH(mobjs_erased, erased);
			continue;
		}

		if(!mobj->show)
		{
			mobj = mobj->next;
			continue;
		}

		const mobj_reginfo_t * info = mobj->info;
		if(!info) continue;

		if(info->handle != NULL)
		{
			info->handle(mobj);
		}

		if(mobj->erase)
		{
			mobj = mobj->next;
			continue;
		}

		for(i = 0; i < info->entmodels_num; i++)
		{

			if(
					mobj->modelplayers[i].model != NULL &&
					mobj->modelplayers[i].model->frames > 0 &&
					mobj->modelplayers[i].model->fps > 0 &&
					mobj->modelplayers[i].action != NULL
			)
			{

				bool end = model_nextframe(
					&mobj->modelplayers[i].frame,
					mobj->modelplayers[i].model->fps,
					mobj->modelplayers[i].action->startframe,
					mobj->modelplayers[i].action->endframe
				);
				if(end)
				{
					const ent_modelaction_t * action = mobj->modelplayers[i].action;
					if(action != NULL && action->endframef != NULL)
					{
						mobj->modelplayers[i].action = NULL;
						action->endframef(
							mobj,
							i,
							action->name
						);
					}
				}

			}

		}
		mobj = mobj->next;

	}
}

static const ent_modelaction_t * mobj_reginfo_action_get(const mobj_reginfo_t * info, unsigned int imodel, char * actionname)
{
	if(info->entmodels == NULL)
		return NULL;
	entmodel_t * entmodel = &info->entmodels[imodel];
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

/**
 * @description начать/возобновить проигрывание кадров модели
 */
void mobj_model_play_start(mobj_t * mobj, unsigned int imodel, char * actionname)
{
	const mobj_reginfo_t * info = mobj->info;
	if(imodel >= info->entmodels_num)
	{
		game_console_send("Error: Entity \"%s\": imodel %d not found, could not play frames.",
			info->name,
			imodel
		);
		return;
	}
	const ent_modelaction_t * action = mobj_reginfo_action_get(info, imodel, actionname);
	if(!action)
	{
		game_console_send("Error: Entity \"%s\", imodel %d: Action \"%s\" not found, could not play frames.",
			info->name,
			imodel,
			actionname
		);
		return;
	}

	ent_modelplayer_t * modelplayer = &mobj->modelplayers[imodel];
	if(modelplayer->action == NULL)
	{
		/* если действия нет или закончилось, начнём действие заново */
		modelplayer->action = action;
		modelplayer->frame = action->startframe;
	}
	else
	{
		modelplayer->action = action;
		float frame = modelplayer->frame;
		if( frame < action->startframe || action->endframe + 1 <= frame )
			modelplayer->frame = action->startframe;
	}

}

/**
 * @description приостановить проигрывание кадров модели
 */
void mobj_model_play_pause(mobj_t * mobj, unsigned int imodel)
{
	const mobj_reginfo_t * info = mobj->info;
	if(imodel >= info->entmodels_num)
	{
		game_console_send("Error: Entity \"%s\": imodel %d not found, could not pause frames.",
			info->name,
			imodel
		);
		return;
	}
	mobj->modelplayers[imodel].action = NULL;
}

void mobj_model_play_pause_all(mobj_t * mobj)
{
	const mobj_reginfo_t * info = mobj->info;
	int imodel;
	for(imodel = 0; imodel < info->entmodels_num; imodel++ )
	{
		mobj->modelplayers[imodel].action = NULL;
	}
}


/**
 * получить следующий entity, соответствующий enttype
 */
mobj_t * entity_getnext(mobj_t * mobj, const char * enttype)
{
	if(mobj == NULL)
	{
		if(enttype == NULL)
			return mobjs;
	}
	/* TODO */
	return NULL;
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
	entmodel_t * ent_models = info->entmodels;
	size_t models_num = info->entmodels_num;


	if(!ent_models) return;

	int i;

	for( i = 0; i < models_num; i++ )
	{
		entmodel_t * ent_model = &ent_models[i];
		ent_modelplayer_t * modelplayer = &mobj->modelplayers[i];

		int frame = VEC_TRUNC(modelplayer->frame);
		direction_t dir = mobj->dir;
		model_render(
			cam,
			pos,
			modelplayer->model,
			ent_model->modelscale,
			ent_model->translation,
			angles[dir],
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
	for(mobj = mobjs; mobj; mobj = mobj->next)
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
		}
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
		if(explList== *explode) { //если HEAD и explode совпадают
			explList = p->next;
			*explode          = explList;
			Z_free(p);
			p = NULL;
		}
		else
		{
			if(p->next)
			{ // если в списке не один эл-т
				while(p && p->next != *explode) p = p->next; // находим ссылку на нужный эл-т
				p->next = (*explode)->next;
				Z_free(*explode); // удаляем его
				*explode = p;     // bull указ. на пред. эл-т
			}
		}
	}
}
*/
