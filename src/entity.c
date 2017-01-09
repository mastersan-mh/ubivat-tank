/*
 * mobjs.c
 *
 *  Created on: 29 нояб. 2016 г.
 *      Author: mastersan
 */

#include "game.h"
#include "entity.h"
#include "model.h"
#include "common_list2.h"

typedef struct
{
	/* информация о объектах в списках */
	const entityinfo_t * info;
	/* существующие объекты (все одного типа) */
	entity_t * entities;
	/* удалённые объекты (все одного типа) */
	entity_t * entities_erased;
}entity_registered_t;

/* зарегестрированные объекты */
static entity_registered_t * entityinfo_regs = NULL;
static size_t entityinfo_regs_size = 0;
static size_t entityinfo_regs_num = 0;

entity_registered_t * entityinfo_get(const char * name)
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
	entity_registered_t * tmp;
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

	if(info->datasize != 0 && info->init == NULL)
		game_console_send("Entity registration warning: entity \"%s\" invalid register data: .entityinit == NULL.", info->name);

	if(info->datasize == 0 && info->init != NULL)
		game_console_send("Entity registration warning: entity \"%s\" invalid register data: .datasize == 0.", info->name);

	if(entityinfo_regs_size < entityinfo_regs_num + 1)
	{
		if(entityinfo_regs_size == 0)
			entityinfo_regs_size = 1;
		else
			entityinfo_regs_size *= 2;
		tmp = Z_realloc(entityinfo_regs, sizeof(entity_registered_t) * entityinfo_regs_size);
		if(!tmp)game_halt("Entity registration failed: out of memory");
		entityinfo_regs = tmp;
	}
	entityinfo_regs[entityinfo_regs_num].info = info;
	entityinfo_regs[entityinfo_regs_num].entities = NULL;
	entityinfo_regs[entityinfo_regs_num].entities_erased = NULL;
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
	entity_registered_t * entityinfo_reg = entityinfo_get(name);
	if(!entityinfo_reg)
	{
		game_console_send("Error: Cannot create unknown entity \"%s\".", name);
		return NULL;
	}

	entity_t * entity = Z_malloc(sizeof(entity_t));

	entity->parent = (entity_t*)parent;
	entity->erase = false;
	entity->pos.x = x;
	entity->pos.y = y;
	entity->dir   = dir;
	entity->allow_handle = true;
	entity->allow_draw = true;

	const entityinfo_t * entityinfo = entityinfo_reg->info;
	entity->info = entityinfo;

	LIST2_PUSH(entityinfo_reg->entities, entity);

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
	if(entityinfo->init)
		entityinfo->init(entity, entity->data, parent, args);

	if(entityinfo->spawn == NULL)
	{
		entity->spawned = true;
	}
	else
	{
		entity->spawned = false;
		entityinfo->spawn(entity, entity->data);
		entity->spawned = true;
	}

	return entity;
}

/*
 *
 */
static void entity_freemem(entity_t * entity)
{
	if(entity->info != NULL)
	{
		if(entity->info->done != NULL)
			entity->info->done(entity, entity->data);
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
		entity_registered_t * entreg = &entityinfo_regs[i];

		entity_t * entity;

		while(entreg->entities)
		{
			entity = entreg->entities;
			entreg->entities = entreg->entities->next;
			entity_freemem(entity);
		}
		while(entreg->entities_erased)
		{
			entity = entreg->entities_erased;
			entreg->entities_erased = entreg->entities_erased->next;
			entity_freemem(entity);
		}

	}
}

/**
 * указать нового родителя взамен старого
 */
static void reparent(const entity_t * oldparent, const entity_t * newparent)
{
	size_t ientreg;
	for(ientreg = 0; ientreg < entityinfo_regs_num; ientreg++)
	{
		entity_registered_t * entreg = &entityinfo_regs[ientreg];
		entity_t * entity;
		for(entity = entreg->entities; entity; entity = entity->next)
		{
			if(entity->parent == oldparent)
				entity->parent = (entity_t*)newparent;
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
		entity_registered_t * entreg = &entityinfo_regs[ientreg];
		entity_t * entity = entreg->entities;
		while(entity)
		{

			if(entity->erase)
			{

				if(entreg->entities == entity)
					entreg->entities = entreg->entities->next;

				entity_t * erased = entity;
				/* укажем нового родителя, взамен удаляемого */
				reparent(erased, erased->parent);
				entity = entity->next;
				if(erased->prev)
					erased->prev->next = erased->next;
				if(erased->next)
					erased->next->prev = erased->prev;

				entity_model_play_pause_all(erased);

				LIST2_PUSH(entreg->entities_erased, erased);
				continue;
			}

			if(
					!entity->allow_handle ||
					!entity->spawned
			)
			{
				entity = entity->next;
				continue;
			}

			const entityinfo_t * info = entity->info;
			if(!info) continue;

			if(info->handle != NULL)
			{
				info->handle(entity, entity->data);
			}

			if(entity->erase)
			{
				entity = entity->next;
				continue;
			}
			int ientmodel;
			for(ientmodel = 0; ientmodel < info->entmodels_num; ientmodel++)
			{

				if(
						entity->modelplayers[ientmodel].model != NULL &&
						entity->modelplayers[ientmodel].model->frames > 0 &&
						entity->modelplayers[ientmodel].model->fps > 0 &&
						entity->modelplayers[ientmodel].action != NULL
				)
				{

					bool end = model_nextframe(
						&entity->modelplayers[ientmodel].frame,
						entity->modelplayers[ientmodel].model->fps,
						entity->modelplayers[ientmodel].action->startframe,
						entity->modelplayers[ientmodel].action->endframe
					);
					if(end)
					{
						const ent_modelaction_t * action = entity->modelplayers[ientmodel].action;
						if(action != NULL && action->endframef != NULL)
						{
							entity->modelplayers[ientmodel].action = NULL;
							action->endframef(
								entity,
								ientmodel,
								action->name
							);
						}
					}

				}

			}
			entity = entity->next;

		}
	}
}

/*
 * обработка событий объектов появления клиента на карте.
 * Как только обработчик объекта возвращает объект (не NULL),
 * этот возвращённый объект становится "телом" клиента.
 */
entity_t * entries_client_spawn()
{
	size_t ientreg;
	for(ientreg = 0; ientreg < entityinfo_regs_num; ientreg++)
	{
		entity_registered_t * entreg = &entityinfo_regs[ientreg];
		const entityinfo_t * info = entreg->info;
		if(!info->client_spawn)
			continue;
		entity_t * entity;
		for(entity = entreg->entities; entity; entity = entity->next)
		{
			entity_t * client_entity = info->client_spawn(entity);
			if(client_entity != NULL)
				return client_entity;
		}
	}
	return NULL;
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
	entity_registered_t * entinfo = entityinfo_get(name);
	if(entinfo == NULL)
	{
		game_console_send("Error: Unknown entity \"%s\".", name);
		return NULL;
	}
	return entinfo->entities;
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
		entity_registered_t * entreg = &entityinfo_regs[i];

		entity_t * entity;
		for(entity = entreg->entities; entity; entity = entity->next)
		{
			if(entity->erase) continue;
			if(!entity->allow_handle) continue;
			if(!entity->spawned) continue;


			//int viewbox_half = entity->img->img_sx;
			int viewbox_half = 16;


			if(
					entity->allow_draw &&
					( cam->pos.x  - cam_sx_half  <= entity->pos.x + viewbox_half ) &&
					( entity->pos.x - viewbox_half <= cam->pos.x  + cam_sx_half  ) &&
					( cam->pos.y  - cam_sy_half  <= entity->pos.y + viewbox_half ) &&
					( entity->pos.y - viewbox_half <= cam->pos.y  + cam_sy_half  )
			)
			{
				ent_models_render(cam, entity);
				ent_rendered++;
			}
		}
	}
	//game_console_send("ent_rendered = %d\n", ent_rendered);
}
