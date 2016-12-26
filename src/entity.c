/*
 * mobjs.c
 *
 *  Created on: 29 нояб. 2016 г.
 *      Author: mastersan
 */

#include "game.h"
#include "entity.h"
#include "model.h"

#define LIST2_PUSH(enttop, ent) \
		do                      \
		{                       \
			(ent)->prev = NULL;     \
			(ent)->next = (enttop); \
			if( (enttop) != NULL ) (enttop)->prev = (ent); \
			(enttop) = (ent);       \
		}while(0)

/*
char *mobjnames[__MOBJ_NUM] =
{
		"spawn_player",
		"spawn_enemy" ,
		"spawn_boss"  ,
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
*/

typedef struct
{
	const entityinfo_t * info;
	/* существующие объекты */
	entity_t * entlinks;
	/* удалённые объекты */
	entity_t * entlinks_erased;
}entityinfo_reg_t;

/* зарегестрированные объекты */
static entityinfo_reg_t * entityinfo_regs = NULL;
static size_t entityinfo_regs_size = 0;
static size_t entityinfo_regs_num = 0;

entityinfo_reg_t * entityinfo_get(const char * name)
{
	size_t i;
	for(i = 0; i < entityinfo_regs_num; i++)
	{
		if(!strncmp(entityinfo_regs[i].info->name, name, 64))
			return &entityinfo_regs[i];
	}
	return NULL;
}

void entity_register(const entityinfo_t * info)
{
	entityinfo_reg_t * tmp;
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


	if(entityinfo_get(info->name) != NULL)
	{
		game_console_send("Entity registration failed: duplicate name \"%s\"", info->name);
		return;
	}

	if(info->datasize != 0 && info->entityinit == NULL)
		game_console_send("Entity registration warning: entity \"%s\" invalid register data: .entityinit == NULL.", info->name);

	if(info->datasize == 0 && info->entityinit != NULL)
		game_console_send("Entity registration warning: entity \"%s\" invalid register data: .datasize == 0.", info->name);

	if(entityinfo_regs_size < entityinfo_regs_num + 1)
	{
		if(entityinfo_regs_size == 0)
			entityinfo_regs_size = 1;
		else
			entityinfo_regs_size *= 2;
		tmp = Z_realloc(entityinfo_regs, sizeof(entityinfo_reg_t) * entityinfo_regs_size);
		if(!tmp)game_halt("Entity registration failed: out of memory");
		entityinfo_regs = tmp;
	}
	entityinfo_regs[entityinfo_regs_num].info = info;
	entityinfo_regs[entityinfo_regs_num].entlinks = NULL;
	entityinfo_regs[entityinfo_regs_num].entlinks_erased = NULL;
	entityinfo_regs_num++;
}

/**
 * @description установить модель на entity
 */
int entity_model_set(entity_t * entity, unsigned int imodel, char * modelname)
{
	if(imodel >= entity->info->entmodels_num) return -1;
	entity->modelplayers[imodel].model = model_get(modelname);
	entity->modelplayers[imodel].frame = 0;
	return 0;
}

/**
 * @description добавление объекта
 */
entity_t * entity_new(const char * name, vec_t x, vec_t y, direction_t dir, const entity_t * parent, const void * args)
{
	int i;
	entityinfo_reg_t * entityinfo_reg = entityinfo_get(name);
	if(!entityinfo_reg)
	{
		game_console_send("Error: Cannot create unknown entity \"%s\".", name);
		return NULL;
	}

	entity_t * entity = Z_malloc(sizeof(entity_t));

	entity->erase = false;
	entity->pos.x = x;
	entity->pos.y = y;
	entity->dir   = dir;
	entity->allow_handle = true;
	entity->allow_draw = true;

	const entityinfo_t * entityinfo = entityinfo_reg->info;
	entity->info = entityinfo;

	LIST2_PUSH(entityinfo_reg->entlinks, entity);

	if(entityinfo->entmodels_num > 0)
	{
		entity->modelplayers = Z_malloc(entityinfo->entmodels_num * sizeof(ent_modelplayer_t));

		for( i = 0; i < entityinfo->entmodels_num; i++)
		{
			entity_model_set(entity, i, entityinfo->entmodels[i].modelname);
		}
	}

	if(entityinfo->datasize == 0)
		entity->data = NULL;
	else
		entity->data = Z_malloc(entityinfo->datasize);
	if(entityinfo->entityinit)
		entity->info->entityinit(entity, entity->data, parent, args);

	return entity;
}

/*
 *
 */
static void entity_freemem(entity_t * entity)
{
	if(entity->info != NULL)
	{
		if(entity->info->entitydone != NULL)
			entity->info->entitydone(entity, entity->data);
		if(entity->info->datasize)
			Z_free(entity->data);
	}
	Z_free(entity->modelplayers);
	Z_free(entity);
}

/**
 * @description удаление всех объектов
 */
void entities_erase()
{
	size_t i;
	for(i = 0; i < entityinfo_regs_num; i++)
	{
		entityinfo_reg_t * entreg = &entityinfo_regs[i];

		entity_t * entlink;

		while(entreg->entlinks)
		{
			entlink = entreg->entlinks;
			entreg->entlinks = entreg->entlinks->next;
			entity_freemem(entlink);
		}
		while(entreg->entlinks_erased)
		{
			entlink = entreg->entlinks_erased;
			entreg->entlinks_erased = entreg->entlinks_erased->next;
			entity_freemem(entlink);
		}

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

void entities_handle()
{
	size_t ientreg;
	for(ientreg = 0; ientreg < entityinfo_regs_num; ientreg++)
	{
		entityinfo_reg_t * entreg = &entityinfo_regs[ientreg];

		entity_t * entlink = entreg->entlinks;
		while(entlink)
		{

			if(entlink->erase)
			{

				if(entreg->entlinks == entlink)
					entreg->entlinks = entreg->entlinks->next;

				entity_t * erased = entlink;
				entlink = entlink->next;
				if(erased->prev)
					erased->prev->next = erased->next;
				if(erased->next)
					erased->next->prev = erased->prev;

				entity_model_play_pause_all(erased);

				LIST2_PUSH(entreg->entlinks_erased, erased);
				continue;
			}

			if(!entlink->allow_handle)
			{
				entlink = entlink->next;
				continue;
			}

			const entityinfo_t * info = entlink->info;
			if(!info) continue;

			if(info->handle != NULL)
			{
				info->handle(entlink);
			}

			if(entlink->erase)
			{
				entlink = entlink->next;
				continue;
			}
			int ientmodel;
			for(ientmodel = 0; ientmodel < info->entmodels_num; ientmodel++)
			{

				if(
						entlink->modelplayers[ientmodel].model != NULL &&
						entlink->modelplayers[ientmodel].model->frames > 0 &&
						entlink->modelplayers[ientmodel].model->fps > 0 &&
						entlink->modelplayers[ientmodel].action != NULL
				)
				{

					bool end = model_nextframe(
						&entlink->modelplayers[ientmodel].frame,
						entlink->modelplayers[ientmodel].model->fps,
						entlink->modelplayers[ientmodel].action->startframe,
						entlink->modelplayers[ientmodel].action->endframe
					);
					if(end)
					{
						const ent_modelaction_t * action = entlink->modelplayers[ientmodel].action;
						if(action != NULL && action->endframef != NULL)
						{
							entlink->modelplayers[ientmodel].action = NULL;
							action->endframef(
								entlink,
								ientmodel,
								action->name
							);
						}
					}

				}

			}
			entlink = entlink->next;

		}
	}
}

static const ent_modelaction_t * entity_reginfo_action_get(const entityinfo_t * info, unsigned int imodel, char * actionname)
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
void entity_model_play_start(entity_t * entity, unsigned int imodel, char * actionname)
{
	const entityinfo_t * info = entity->info;
	if(imodel >= info->entmodels_num)
	{
		game_console_send("Error: Entity \"%s\": imodel %d not found, could not play frames.",
			info->name,
			imodel
		);
		return;
	}
	const ent_modelaction_t * action = entity_reginfo_action_get(info, imodel, actionname);
	if(!action)
	{
		game_console_send("Error: Entity \"%s\", imodel %d: Action \"%s\" not found, could not play frames.",
			info->name,
			imodel,
			actionname
		);
		return;
	}

	ent_modelplayer_t * modelplayer = &entity->modelplayers[imodel];
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
void entity_model_play_pause(entity_t * entity, unsigned int imodel)
{
	const entityinfo_t * info = entity->info;
	if(imodel >= info->entmodels_num)
	{
		game_console_send("Error: Entity \"%s\": imodel %d not found, could not pause frames.",
			info->name,
			imodel
		);
		return;
	}
	entity->modelplayers[imodel].action = NULL;
}

void entity_model_play_pause_all(entity_t * entity)
{
	const entityinfo_t * info = entity->info;
	int imodel;
	for(imodel = 0; imodel < info->entmodels_num; imodel++ )
	{
		entity->modelplayers[imodel].action = NULL;
	}
}


/**
 * получить первый entity, соответствующий name
 */
entity_t * entity_getfirst(const char * name)
{
	entityinfo_reg_t * entinfo = entityinfo_get(name);
	if(entinfo == NULL)
	{
		game_console_send("Error: Unknown entity \"%s\".", name);
		return NULL;
	}
	return entinfo->entlinks;
}

static void ent_models_render(
	camera_t * cam,
	entity_t * entity
)
{
	static GLfloat angles[] =
	{
			0.0f,   /* N */
			180.0f, /* S */
			270.0f, /* W */
			90.0f,  /* E */
	};

	vec2_t pos = entity->pos;
	const struct entityinfo_s * info = entity->info;
	entmodel_t * ent_models = info->entmodels;
	size_t models_num = info->entmodels_num;


	if(!ent_models) return;

	int i;

	for( i = 0; i < models_num; i++ )
	{
		entmodel_t * ent_model = &ent_models[i];
		ent_modelplayer_t * modelplayer = &entity->modelplayers[i];

		int frame = VEC_TRUNC(modelplayer->frame);
		direction_t dir = entity->dir;
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
void entities_render(camera_t * cam)
{
	int cam_sx_half = cam->sx / 2;
	int cam_sy_half = cam->sy / 2;
	int ent_rendered = 0;

	size_t i;
	for(i = 0; i < entityinfo_regs_num; i++)
	{
		entityinfo_reg_t * entreg = &entityinfo_regs[i];

		entity_t * entlink;
		for(entlink = entreg->entlinks; entlink; entlink = entlink->next)
		{
			if(entlink->erase) continue;
			if(!entlink->allow_handle) continue;

			//int viewbox_half = entity->img->img_sx;
			int viewbox_half = 16;


			if(
					entlink->allow_draw &&
					( cam->pos.x  - cam_sx_half  <= entlink->pos.x + viewbox_half ) &&
					( entlink->pos.x - viewbox_half <= cam->pos.x  + cam_sx_half  ) &&
					( cam->pos.y  - cam_sy_half  <= entlink->pos.y + viewbox_half ) &&
					( entlink->pos.y - viewbox_half <= cam->pos.y  + cam_sy_half  )
			)
			{
				ent_models_render(cam, entlink);
				ent_rendered++;
			}
		}
	}
	//game_console_send("ent_rendered = %d\n", ent_rendered);
}