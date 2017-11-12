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

var_descr_t entity_common_vars[] =
{
        ENTITY_COMMON_VARS,
};

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
    static var_descr_t entity_common_vars[] =
    {
            ENTITY_COMMON_VARS
    };

    ssize_t i;
    entity_registered_t * tmp;
    if(info == NULL)
    {
        game_console_send("Entity registration failed: Register data is NULL.");
        return;
    }

    if(info->name == NULL || strnlen(info->name, ENTITY_NAME_SIZE) == 0)
    {
        game_console_send("Entity registration failed: entity name is empty.");
        return;
    }

    if(entityregisteredinfo_get(info->name) != NULL)
    {
        game_console_send("Entity \"%s\" registration failed: duplicate name \"%s\"", info->name);
        return;
    }

    if(info->vars_size == 0)
    {
        game_console_send("Entity \"%s\" registration failed: Invalid descriptor: .vars_size == 0.", info->name);
        return;
    }
    if(info->vars_descr_num == 0)
    {
        game_console_send("Entity \"%s\" registration failed: Invalid descriptor: .vars_descr_num == 0.", info->name);
        return;
    }
    if(info->vars_descr == NULL)
    {
        game_console_send("Entity \"%s\" registration failed: Invalid descriptor: .vars_descr == NULL.", info->name);
        return;
    }

bool vars_descr_eq(const var_descr_t * vds1, const var_descr_t * vds2, size_t size)
{
    for(size_t i = 0; i < size; i++)
    {
        const var_descr_t * vd1 = &vds1[i];
        const var_descr_t * vd2 = &vds2[i];
        if(
                vd1->type != vd2->type ||
                vd1->ofs  != vd2->ofs  ||
                vd1->size != vd2->size ||
                strncmp(vd1->name, vd2->name, VARNAME_SIZE) != 0
        )
            return false;

    }
    return true;
}

    if(!vars_descr_eq(info->vars_descr, entity_common_vars , ARRAYSIZE(entity_common_vars)))
    {
        game_console_send("Entity \"%s\" registration failed: Invalid entity common part.", info->name);
        return;
    }

    do {
        /* проверка переменных */
        uint32_t * varnames_hashs = calloc(info->vars_descr_num, sizeof(uint32_t));
        for(i = 0; i < info->vars_descr_num; i++)
        {
            varnames_hashs[i] = HASH32(info->vars_descr[i].name);
            ssize_t j;
            for(j = 0; j < i; j++)
            {
                if(varnames_hashs[i] == varnames_hashs[j])
                {
                    game_console_send("Entity \"%s\" registration failed: Duplicate variable name \"%s\".", info->name, info->vars_descr[i].name);
                    free(varnames_hashs);
                    return;
                }
            }
        }
        free(varnames_hashs);
    } while(0);

    if(info->bodybox <= 0.0f)
        game_console_send("Entity \"%s\" registration warning: Invalid descriptor: .bodybox <= 0.0f.", info->name);

    if(entityregs_size < entityregs_num + 1)
    {
        if(entityregs_size == 0)
            entityregs_size = 1;
        else
            entityregs_size *= 2;
        tmp = Z_realloc(entityregs, sizeof(entity_registered_t) * entityregs_size);
        if(!tmp)game_halt("Entity \"%s\" registration failed: Out of memory", info->name);
        entityregs = tmp;
    }
    entityregs[entityregs_num].info = info;
    entityregs[entityregs_num].entities = NULL;
    entityregs[entityregs_num].entities_erased = NULL;
    entityregs_num++;

    game_console_send("Entity \"%s\" registered.", info->name);

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
 * @brief восстановление информации о entity игрока из хранилища игрока в entity
 * @brief (при переходе на следующий уровень и при чтении gamesave)
 */
void entity_restore(entity_t * entity, const void * vars)
{
    if(!vars)
        return;
    size_t info_vars_num = entity->info->vars_descr_num;
    var_descr_t * info_vars = entity->info->vars_descr;

    for(size_t i = 0; i < info_vars_num; i++)
    {
        intptr_t ofs = info_vars[i].ofs;
        memcpy(entity->common + ofs, vars + ofs, info_vars[i].size);
    }

    char title[128];
    sprintf(title, "==== Entity \"%s\" vars:", entity->info->name);
    vars_dump(entity->common, entity->info->vars_descr, entity->info->vars_descr_num, title);

}
/**
 * @params vars     Буфер хранилища переменных
 */
void entity_respawn(entity_t * entity, const void * vars)
{
    if(entity->spawned)
        return;
    entity_restore(entity, vars);
    const entityinfo_t * info = entity->info;
    entity_common_t * common = entity->common;
    if(info->spawn)
        info->spawn(entity, common);
    entity->spawned = true;
    common->alive = true;
}

void entity_erase(void * entity)
{
    ((entity_t*)entity)->erase = true;
}

bool entity_is_spawned(void * entity)
{
    return ((entity_t*)entity)->spawned;
}

void entity_unspawn(void * entity)
{
    ((entity_t*)entity)->spawned = false;
}

void entity_show(void * entity)
{
    ((entity_t*)entity)->allow_draw = true;
}

void entity_hide(void * entity)
{
    ((entity_t*)entity)->allow_draw = false;
}

void * entity_parent(void * entity)
{
    return ((entity_t*)entity)->parent;
}

void * entity_vars(void * entity)
{
    return ((entity_t*)entity)->common;
}

void entity_cam_set(void * entity, void * cam_entity)
{
    ((entity_t*)entity)->cam_entity = cam_entity;
}

void entity_cam_reset(void * entity)
{
    ((entity_t*)entity)->cam_entity = entity;
}


void * entity_build_storage(const char * name, const var_value_t * vars_values, size_t vars_values_num)
{
    entity_registered_t * entityinfo_reg = entityregisteredinfo_get(name);
    if(!entityinfo_reg)
    {
        game_console_send("Error: Build storage unknown entity \"%s\".", name);
        return NULL;
    }

    if(vars_values_num == 0)
        return NULL;

    const entityinfo_t * info = entityinfo_reg->info;

    size_t bufsize = var_buffersize_calculate(info->vars_descr, info->vars_descr_num);
    void * buf = Z_malloc(bufsize);
    for(size_t i = 0; i < vars_values_num; i++)
    {
        const var_value_t * var_value = &vars_values[i];

        const var_descr_t * vd = var_find(info->vars_descr, info->vars_descr_num, var_value->name);
        if(!vd)
        {
            game_console_send("Warning: While entity \"%s\" creation, variable \"%s\" not found.", name, var_value->name);
            continue;
        }
        if(var_value->type != vd->type)
        {
            game_console_send("Warning: While entity \"%s\" creation, variable \"%s\" has other type.", name, var_value->name);
            continue;
        }

        memcpy(buf + vd->ofs, &var_value->blob, vd->size);

    }
    return buf;
}

/**
 * @description добавление объекта
 */
entity_t * entity_new(const char * name, entity_t * parent, const var_value_t * vars_values, size_t vars_values_num)
{
    size_t i;
    entity_registered_t * entityinfo_reg = entityregisteredinfo_get(name);
    if(!entityinfo_reg)
    {
        game_console_send("Error: Cannot create unknown entity \"%s\".", name);
        return NULL;
    }

    const entityinfo_t * entityinfo = entityinfo_reg->info;

    entity_t * entity = Z_malloc(sizeof(entity_t));
    entity->info = entityinfo;

    entity->common = Z_malloc(entityinfo->vars_size);

    entity->parent = (entity_t*)parent;
    entity->cam_entity = entity;
    entity->erase = false;
    entity->freezed = false;
    entity->allow_draw = true;

    LIST2_PUSH(entityinfo_reg->entities, entity);

    if(entityinfo->entmodels_num > 0)
    {
        entity->modelplayers = Z_malloc(entityinfo->entmodels_num * sizeof(ent_modelplayer_t));

        for( i = 0; i < entityinfo->entmodels_num; i++)
        {
            entity_model_set(entity, i, entityinfo->entmodels[i].modelname);
        }
    }

    entity->spawned = false;

    entity_common_t * common = entity->common;
    void * storage = entity_build_storage(name, vars_values, vars_values_num);
    entity_restore(entity, storage);
    if(entityinfo->init)
        entityinfo->init(entity, common, parent);
    if(entityinfo->spawn)
        entityinfo->spawn(entity, common);

    entity->spawned = true;
    common->alive = true;

    Z_free(storage);

    return entity;
}

/**
 * @brief Очистка памяти entity
 */
static void entity_freemem(entity_t * entity)
{
    const entityinfo_t * info = entity->info;
    if(info != NULL)
    {
        if(info->done != NULL)
            info->done(entity, entity->common);
    }

    vars_free(entity->common, info->vars_descr, info->vars_descr_num);

    Z_free(entity->modelplayers);
    Z_free(entity->common);
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
    entity_common_t * thisc = this->common;
    entity_common_t * thatc = that->common;

    vec_t this_halfbox = this->info->bodybox * 0.5;
    vec_t that_halfbox = that->info->bodybox * 0.5;
    return
            ( thisc->origin[0] - this_halfbox <= thatc->origin[0] + that_halfbox ) &&
            ( thatc->origin[0] - that_halfbox <= thisc->origin[0] + this_halfbox ) &&
            ( thisc->origin[1] - this_halfbox <= thatc->origin[1] + that_halfbox ) &&
            ( thatc->origin[1] - that_halfbox <= thisc->origin[1] + this_halfbox )
            ;
}

/**
 * touchs
 */
static void P_entity_touchs(const entityinfo_t * info, entity_t * entity)
{
    entity_common_t * common = entity->common;
    if(
            entity->freezed ||
            !entity->spawned ||
            !common->alive
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
                entity_common_t * thatc = entity->common;
                if(
                        !that->erase &&
                        !that->freezed &&
                        that->spawned &&
                        thatc->alive &&
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
        ent_modelplayer_t * modelplayer = &entity->modelplayers[ientmodel];
        if(!(
                modelplayer->model != NULL &&
                modelplayer->model->frames != NULL &&
                modelplayer->model->fps > 0 &&
                modelplayer->action != NULL
        ))
            continue;

        bool end = model_nextframe(
            &modelplayer->frame,
            modelplayer->model->fps,
            modelplayer->action->startframe,
            modelplayer->action->endframe
        );
        if(!end)
            return;

        const ent_modelaction_t * action = modelplayer->action;
        if(action != NULL && action->endframef != NULL)
        {
            modelplayer->action = NULL;
            action->endframef(
                entity,
                ientmodel,
                action->name
            );
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
            entity_common_t * common = entity->common;
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
                    entity->freezed ||
                    !entity->spawned
            )
            {
                entity = entity->next;
                continue;
            }

            VEC2_COPY(common->origin, common->origin_prev);
            if(info->handle != NULL)
            {
                info->handle(entity, common);
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

/**
 * @brief
 * @descrition Обработка событий объектов присоедениения клиента к игре.
 * Как только обработчик объекта возвращает объект (не NULL),
 * этот возвращённый объект становится "телом" клиента.
 * @param vars  Буфер, в котором хранятся значения переменных entity, которые необходимо восстановить
 */
entity_t * entity_player_spawn_random(void * storage)
{
    size_t ientreg;
    for(ientreg = 0; ientreg < entityregs_num; ientreg++)
    {
        entity_registered_t * entreg = &entityregs[ientreg];
        const entityinfo_t * info = entreg->info;
        if(!info->player_spawn)
            continue;
        entity_t * entity;
        for(entity = entreg->entities; entity; entity = entity->next)
        {
            entity_t * entity = info->player_spawn(entity, storage);
            return entity;
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

    entity_common_t * common = entity->common;
    vec2_t * pos = &common->origin;
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
        direction_t dir = common->dir;
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
            if(entity->freezed) continue;
            if(!entity->spawned) continue;


            //int viewbox_half = entity->img->img_sx;
            int viewbox_half = 16;

            entity_common_t * common = entity->common;

            if(
                    entity->allow_draw &&
                    ( cam->origin[0]    - cam_sx_half  <= common->origin[0] + viewbox_half ) &&
                    ( common->origin[0] - viewbox_half <= cam->origin[0]    + cam_sx_half  ) &&
                    ( cam->origin[1]    - cam_sy_half  <= common->origin[1] + viewbox_half ) &&
                    ( common->origin[1] - viewbox_half <= cam->origin[1]    + cam_sy_half  )
            )
            {
                ent_models_render(cam, entity);
                ent_rendered++;
            }
        }
    }
    //game_console_send("ent_rendered = %d\n", ent_rendered);
}
