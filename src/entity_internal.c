/*
 * entity_internal.c
 *
 *  Created on: 12 нояб. 2017 г.
 *      Author: mastersan
 */
#include "entity_internal.h"
#include "entity.h"
#include "game.h"

#include "common/common_list2.h"

/* зарегестрированные объекты */
entity_registered_t * entityregs = NULL;
size_t entityregs_size = 0;
size_t entityregs_num = 0;

/**
 *  получить регистрационную информацию объекта
 */
entity_registered_t * entityregisteredinfo_get(const char * name)
{
    size_t i;
    for(i = 0; i < entityregs_num; i++)
    {
        if(!strncmp(entityregs[i].info->name, name, ENTITY_NAME_SIZE))
            return &entityregs[i];
    }
    return NULL;
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
            info->done((ENTITY)entity);
    }

    vars_free(entity->vars, info->vars_descr, info->vars_descr_num);

    Z_free(entity->modelplayers);
    Z_free(entity->vars);
    Z_free(entity);
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

static bool is_touched(entity_t * this, entity_t * that)
{
    entity_common_t * thisc = this->vars;
    entity_common_t * thatc = that->vars;

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
    entity_common_t * common = entity->vars;
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
                entity_common_t * thatc = entity->vars;
                if(
                        !that->erase &&
                        !that->freezed &&
                        that->spawned &&
                        thatc->alive &&
                        is_touched(entity, that)
                )
                    entitytouchs[i].touch((ENTITY)entity, (ENTITY)that);
            }
        }
    }
}

/**
 * @return end-of-frames?
 */
static void model_nextframe(float * frame, unsigned int fps, unsigned int startframe, unsigned int endframe,
    bool * start, bool * end
)
{
    *start = false;
    *end = false;
    if(fps == 0)
    {
        *start = true;
        return;
    }

    float sf = startframe;
    float ef = endframe;
    if(*frame < sf) *frame = sf;
    if(*frame == sf)
        *start = true;

    *frame += fps * dtimed1000;
    if(*frame >= ef + 1) // последний кадр длится столько же, сколько и остальные кадры
    {
        *frame = ef;
        *end = true;
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

        const ent_modelaction_t * action = modelplayer->action;
        bool start;
        bool end;
        model_nextframe(
            &modelplayer->frame,
            modelplayer->model->fps,
            modelplayer->action->startframe,
            modelplayer->action->endframe,
            &start, &end
        );
        if(start && action != NULL && action->startframef != NULL)
        {
            action->startframef(
                (ENTITY)entity,
                ientmodel,
                action->name
            );
        }

        if(end && action != NULL && action->endframef != NULL)
        {
            modelplayer->action = NULL;
            action->endframef(
                (ENTITY)entity,
                ientmodel,
                action->name
            );
        }

    }
}

/**
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

const ent_modelaction_t * entity_reginfo_action_get(const entityinfo_t * info, unsigned int imodel, char * actionname)
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
            entity_common_t * vars = entity->vars;
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

                entity_model_play_pause_all((ENTITY)erased);

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

            VEC2_COPY(vars->origin_prev, vars->origin);
            if(info->handle != NULL)
            {
                info->handle((ENTITY)entity);
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

    entity_common_t * common = entity->vars;
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

/**
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

            entity_common_t * common = entity->vars;

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
entity_t * entity_new_(const char * name, entity_t * parent, const var_value_t * vars_values, size_t vars_values_num)
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

    entity->vars = Z_malloc(entityinfo->vars_size);

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
            entity_model_set((ENTITY)entity, i, entityinfo->entmodels[i].modelname);
        }
    }

    entity->spawned = false;

    entity_common_t * common = entity->vars;
    void * storage = entity_build_storage(name, vars_values, vars_values_num);
    entity_restore((ENTITY)entity, storage);
    if(entityinfo->init)
        entityinfo->init((ENTITY)entity, (ENTITY)parent);
    if(entityinfo->spawn)
        entityinfo->spawn((ENTITY)entity);

    entity->spawned = true;
    common->alive = true;

    Z_free(storage);

    return entity;
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
            entity_t * ent = info->player_spawn((ENTITY)entity, storage);
            return ent;
        }
    }
    return NULL;
}

/**
 * @params vars     Буфер хранилища переменных
 */
void entity_respawn(entity_t * entity, const void * vars)
{
    if(entity->spawned)
        return;
    entity_restore((ENTITY)entity, vars);
    const entityinfo_t * info = entity->info;
    if(info->spawn)
        info->spawn((ENTITY)entity);
    entity->spawned = true;
    entity_common_t * common = entity->vars;
    common->alive = true;
}


