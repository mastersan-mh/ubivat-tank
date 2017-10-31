/*
 * mobjs.c
 *
 *  Created on: 29 нояб. 2016 г.
 *      Author: mastersan
 */

#include "game.h"
#include "entity.h"
#include "model.h"
#include "common/common_list2.h"
#include "common/common_hash.h"
#include "vars.h"

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
static entity_registered_t * entityregs = NULL;
static size_t entityregs_size = 0;
static size_t entityregs_num = 0;

/*
 *  получить регистрационную информацию объекта
 */
static entity_registered_t * entityregisteredinfo_get(const char * name)
{
    size_t i;
    for(i = 0; i < entityregs_num; i++)
    {
        if(!strncmp(entityregs[i].info->name, name, ENTITY_NAME_SIZE))
            return &entityregs[i];
    }
    return NULL;
}

/*
 *  получить информацию по типу объекта
 */
const entityinfo_t * entityinfo_get(const char * name)
{
    const entity_registered_t * entity_registered = entityregisteredinfo_get(name);
    if(entity_registered == NULL)
        return NULL;
    return entity_registered->info;
}

void entity_register(const entityinfo_t * info)
{
    ssize_t i;
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

    if(entityregisteredinfo_get(info->name) != NULL)
    {
        game_console_send("Entity registration failed: duplicate name \"%s\"", info->name);
        return;
    }

    if(info->edatasize != 0 && info->init == NULL)
        game_console_send("Entity registration warning: entity \"%s\" invalid register data: .entityinit == NULL.", info->name);
    /*
	if(info->datasize == 0 && info->init != NULL)
		game_console_send("Entity registration warning: entity \"%s\" invalid register data: .datasize == 0.", info->name);
     */

    if(info->bodybox <= 0.0f)
        game_console_send("Entity registration warning: entity \"%s\" invalid register data: .bodybox <= 0.0f.", info->name);

    {
        /* проверка переменных */
        nodeId_t * nodeIds = calloc(info->vars_num, sizeof(nodeId_t));
        for(i = 0; i < info->vars_num; i++)
        {
            nodeIds[i] = HASH32(info->vars[i].name);
            ssize_t j;
            for(j = 0; j < i; j++)
            {
                if(nodeIds[i] == nodeIds[j])
                {
                    game_console_send("Entity registration failed: entity \"%s\" duplicate variable name \"%s\".", info->name, info->vars[i].name);
                    free(nodeIds);
                    return;
                }
            }
        }
        free(nodeIds);
    }

    if(entityregs_size < entityregs_num + 1)
    {
        if(entityregs_size == 0)
            entityregs_size = 1;
        else
            entityregs_size *= 2;
        tmp = Z_realloc(entityregs, sizeof(entity_registered_t) * entityregs_size);
        if(!tmp)game_halt("Entity registration failed: out of memory");
        entityregs = tmp;
    }
    entityregs[entityregs_num].info = info;
    entityregs[entityregs_num].entities = NULL;
    entityregs[entityregs_num].entities_erased = NULL;
    entityregs_num++;

    game_console_send("Entity registered: \"%s\".", info->name);

}

/**
 * @description установить модель на entity
 */
int entity_model_set(entity_t * entity, unsigned int imodel, const char * modelname)
{
    if(imodel >= entity->info->entmodels_num)
        return -1;
    const model_t * model = model_get(modelname);
    if(!model)
    {
        game_console_send("Error: Entity model set: No model \"%s\"", modelname);
    }
    entity->modelplayers[imodel].model = model;
    entity->modelplayers[imodel].frame = 0.0f;
    return 0;
}

/**
 * @description добавление объекта
 */
entity_t * entity_new(const char * name, vec_t x, vec_t y, direction_t dir, const entity_t * parent)
{
    size_t i;
    entity_registered_t * entityinfo_reg = entityregisteredinfo_get(name);
    if(!entityinfo_reg)
    {
        game_console_send("Error: Cannot create unknown entity \"%s\".", name);
        return NULL;
    }

    entity_t * entity = Z_malloc(sizeof(entity_t));

    entity->parent = (entity_t*)parent;
    entity->cam_entity = entity;
    entity->erase = false;
    VEC2_SET(entity->origin, x, y);
    entity->dir   = dir;
    entity->alive = true;
    entity->allow_handle = true;
    entity->allow_draw = true;
    entity->stat_traveled_distance = 0.0f;

    const entityinfo_t * entityinfo = entityinfo_reg->info;
    entity->info = entityinfo;

    {
        /* сформировать переменные объекта */
        entityvarinfo_t * evars = entityinfo->vars;
        for(i = 0; i < entityinfo->vars_num; i++)
        {
            var_create(&entity->vars, evars[i].name, evars[i].type);
        }
    }

    LIST2_PUSH(entityinfo_reg->entities, entity);

    if(entityinfo->entmodels_num > 0)
    {
        entity->modelplayers = Z_malloc(entityinfo->entmodels_num * sizeof(ent_modelplayer_t));

        for( i = 0; i < entityinfo->entmodels_num; i++)
        {
            entity_model_set(entity, i, entityinfo->entmodels[i].modelname);
        }
    }

    if(entityinfo->edatasize == 0)
        entity->edata = NULL;
    else
        entity->edata = Z_malloc(entityinfo->edatasize);
    if(entityinfo->init)
        entityinfo->init(entity, entity->edata, parent);

    if(entityinfo->spawn == NULL)
    {
        entity->spawned = true;
    }
    else
    {
        entity->spawned = false;
        entityinfo->spawn(entity, entity->edata);
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
            entity->info->done(entity, entity->edata);
        if(entity->info->edatasize)
            Z_free(entity->edata);
    }
    Z_free(entity->modelplayers);

    vars_delete(&entity->vars);
    Z_free(entity);
}

/**
 * @description удаление всех объектов
 */
void entities_erase(void)
{
    size_t i;
    for(i = 0; i < entityregs_num; i++)
    {
        entity_registered_t * entreg = &entityregs[i];

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
 * @brief получить данные переменной
 * @param vartype   тип переменной: -1 - тип выбрать автоматически
 */
vardata_t * entity_vardata_get(const entity_t * entity, const char * varname, vartype_t vartype)
{
    static const char * list[] =
    {
            "INTEGER",
            "FLOAT",
            "STRING",
    };
    var_t * node = var_find(entity->vars, varname);
    if(!node)
    {
        game_console_send("Error: Entity \"%s\" has no variable \"%s\"", entity->info->name, varname);
        return NULL;
    }
    vardata_t * vardata = node->data;
    if( (int)vartype >= 0 && vardata->type != vartype )
    {
        game_console_send("Warning: Entity \"%s\" variable \"%s\" has type %s, but used as %s.", entity->info->name, varname, list[vardata->type], list[vartype]);
    }
    return vardata;
}

/**
 * указать нового родителя взамен старого
 */
static void reparent(const entity_t * oldparent, const entity_t * newparent)
{
    size_t ientreg;
    for(ientreg = 0; ientreg < entityregs_num; ientreg++)
    {
        entity_registered_t * entreg = &entityregs[ientreg];
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

static bool is_touched(entity_t * this, entity_t * that)
{
    vec_t this_halfbox = this->info->bodybox * 0.5;
    vec_t that_halfbox = that->info->bodybox * 0.5;
    return
            ( this->origin[0] - this_halfbox <= that->origin[0] + that_halfbox ) &&
            ( that->origin[0] - that_halfbox <= this->origin[0] + this_halfbox ) &&
            ( this->origin[1] - this_halfbox <= that->origin[1] + that_halfbox ) &&
            ( that->origin[1] - that_halfbox <= this->origin[1] + this_halfbox )
            ;
}

/**
 * touchs
 */
static void P_entity_touchs(const entityinfo_t * info, entity_t * entity)
{
    if(
            !entity->allow_handle ||
            !entity->alive ||
            !entity->spawned
    )
        return;

    size_t i;
    entitytouch_t * entitytouchs = info->entitytouchs;
    for(i = 0; i < info->entitytouchs_num; i++)
    {
        entity_registered_t * entityreg = entityregisteredinfo_get(entitytouchs[i].entityname);


        const entityinfo_t * thatinfo = entityreg->info;
        if(!thatinfo)
        {
            game_console_send("Error: Entity \"%s\" can not touch unknown entities \"%s\".",
                info->name, entitytouchs[i].entityname);
            break;
        }

        if(entitytouchs[i].touch)
        {
            entity_t * that;
            for(that = entityreg->entities; that; that = that->next)
            {
                if(
                        that->allow_handle &&
                        that->alive &&
                        that->spawned &&
                        !that->erase &&
                        is_touched(entity, that)
                )
                    entitytouchs[i].touch(entity, that);
            }
        }
    }
}

/**
 * проигрывание кадров моделей
 */
static void P_entity_modelplay(const entityinfo_t * info, entity_t * entity)
{

    size_t ientmodel;
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
}

/*
 * передвижение игрока
 */
static void P_entity_move(const entityinfo_t * info, entity_t * entity)
{
    if(!(info->flags | ENTITYFLAG_SOLIDWALL))
        return;

    /*
	vec2_t * orig = &entity->origin;
     */
    /*
	vec2_t * orig_prev = &entity->origin_prev;
	vec_t halfbox = info->bodybox/2;
	vec_t dist;
     */
    /*
	direction_t dir;
	bool check_clip = false;
	if(check_clip)  FIXME: костыль
	{
		// * найдем препятствия *
		map_clip_find_near(orig, info->bodybox, dir, MAP_WALL_CLIP, info->bodybox, &dist);
		if(dist < dway + halfbox)
			dway = dist - halfbox;
	}
	switch(dir)
	{
	case DIR_UP   : orig->y += dway; break;
	case DIR_DOWN : orig->y -= dway; break;
	case DIR_LEFT : orig->x -= dway; break;
	case DIR_RIGHT: orig->x += dway; break;
	}
	//подсчитываем пройденный путь
	entity->stat_traveled_distance += VEC_ABS(dway);
     */
}

void entities_handle(void)
{
    size_t ientreg;
    for(ientreg = 0; ientreg < entityregs_num; ientreg++)
    {
        entity_registered_t * entreg = &entityregs[ientreg];

        const entityinfo_t * info = entreg->info;
        if(!info)
            continue;

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

            VEC2_COPY(entity->origin, entity->origin_prev);
            if(info->handle != NULL)
            {
                (*info->handle)(entity, entity->edata);
            }

            if(!entity->erase)
                P_entity_touchs(info, entity);

            if(!entity->erase)
                P_entity_move(info, entity);

            if(!entity->erase)
                P_entity_modelplay(info, entity);

            entity = entity->next;
        }
    }
}

/*
 * обработка событий объектов присоедениения клиента к игре.
 * Как только обработчик объекта возвращает объект (не NULL),
 * этот возвращённый объект становится "телом" клиента.
 */
entity_t * entries_client_join(void)
{
    size_t ientreg;
    for(ientreg = 0; ientreg < entityregs_num; ientreg++)
    {
        entity_registered_t * entreg = &entityregs[ientreg];
        const entityinfo_t * info = entreg->info;
        if(!info->client_join)
            continue;
        entity_t * entity;
        for(entity = entreg->entities; entity; entity = entity->next)
        {
            entity_t * client_entity = (*info->client_join)(entity);
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
    entitymodel_t * entmodel = &info->entmodels[imodel];
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
    entity_registered_t * entinfo = entityregisteredinfo_get(name);
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

    vec2_t * pos = &entity->origin;
    const struct entityinfo_s * info = entity->info;
    entitymodel_t * ent_models = info->entmodels;
    size_t models_num = info->entmodels_num;


    if(!ent_models) return;

    size_t i;

    for( i = 0; i < models_num; i++ )
    {
        entitymodel_t * ent_model = &ent_models[i];
        ent_modelplayer_t * modelplayer = &entity->modelplayers[i];
        if(!modelplayer)
        {
            game_console_send("Error: entity \"%s\", imodel %d, modelplayer == NULL.", info->name, (int)i);
            continue;
        }
        int frame = VEC_TRUNC(modelplayer->frame);
        direction_t dir = entity->dir;
        model_render(
            cam,
            *pos,
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
    for(i = 0; i < entityregs_num; i++)
    {
        entity_registered_t * entreg = &entityregs[i];

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
                    ( cam->origin[0]    - cam_sx_half  <= entity->origin[0] + viewbox_half ) &&
                    ( entity->origin[0] - viewbox_half <= cam->origin[0]    + cam_sx_half  ) &&
                    ( cam->origin[1]    - cam_sy_half  <= entity->origin[1] + viewbox_half ) &&
                    ( entity->origin[1] - viewbox_half <= cam->origin[1]    + cam_sy_half  )
            )
            {
                ent_models_render(cam, entity);
                ent_rendered++;
            }
        }
    }
    //game_console_send("ent_rendered = %d\n", ent_rendered);
}
