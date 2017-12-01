/*
 * entity_internal.c
 *
 *  Created on: 12 нояб. 2017 г.
 *      Author: mastersan
 */
#include "entity_internal.h"
#include "entity.h"
#include "game.h"

#include <assert.h>

/* зарегестрированные объекты */
entity_registered_t * entityregs = NULL;

/* существующие объекты (все одного типа) */
entity_head_t entities = CIRCLEQ_HEAD_INITIALIZER(entities);
/* удалённые объекты (все одного типа) */
entity_head_t entities_erased = CIRCLEQ_HEAD_INITIALIZER(entities_erased);


size_t entityregs_size = 0;
size_t entityregs_num = 0;


static entity_id_t entityId_last = 0;

entity_t * entity_find_by_id(entity_id_t entityId)
{
    entity_t * entity;
    CIRCLEQ_FOREACH(entity, &entities, list)
    {
        if(entity->id == entityId)
            return entity;
    }
    return NULL;
}

/**
 *  получить регистрационную информацию объекта
 */
entity_registered_t * entityregisteredinfo_get(const char * name)
{
    size_t i;
    for(i = 0; i < entityregs_num; i++)
    {
        if(!strncmp(entityregs[i].info->name_, name, ENTITY_CLASSNAME_SIZE))
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

    size_t i;
    for( i = 0; i < info->models_num; i++)
    {
        Z_free(entity->models[i].name);
    }
    Z_free(entity->models);
    Z_free(entity->vars);
    Z_free(entity);
}

/**
 * указать нового родителя взамен старого
 */
static void reparent(const entity_t * oldparent, const entity_t * newparent)
{
    entity_t * entity;
    CIRCLEQ_FOREACH(entity, &entities, list)
    {
        if(entity->parent == oldparent)
            entity->parent = (entity_t*)newparent;
    }
}


static bool entities_in_contact(entity_t * ent1, entity_t * ent2)
{
    entity_vars_common_t * ent1_vars = ent1->vars;
    entity_vars_common_t * ent2_vars = ent2->vars;

    FLOAT this_halfbox = ent1->bodybox * 0.5;
    FLOAT that_halfbox = ent2->bodybox * 0.5;
    return
            ( ent1_vars->origin[0] - this_halfbox <= ent2_vars->origin[0] + that_halfbox ) &&
            ( ent2_vars->origin[0] - that_halfbox <= ent1_vars->origin[0] + this_halfbox ) &&
            ( ent1_vars->origin[1] - this_halfbox <= ent2_vars->origin[1] + that_halfbox ) &&
            ( ent2_vars->origin[1] - that_halfbox <= ent1_vars->origin[1] + this_halfbox )
            ;
}

/**
 * touchs
 */
static void P_entity_touchs(entity_t * self)
{
    entity_vars_common_t * self_vars = self->vars;
    if(
            self->erase ||
            self->freezed ||
            !self->spawned ||
            !self_vars->alive
    )
        return;

    entity_t * other;

    for( other = CIRCLEQ_NEXT(self, list); !CIRCLEQ_END(other, &entities); other = CIRCLEQ_NEXT(other, list) )
    {
        bool self_touch = (self->info->touch != NULL);
        bool other_touch = (other->info->touch != NULL);

        if( !(self_touch || other_touch) )
            continue;

        if( !entities_in_contact(self, other) )
            continue;

        entity_vars_common_t * other_vars = other->vars;
        if(
                other->erase ||
                other->freezed ||
                !other->spawned ||
                !other_vars->alive
        )
            continue;

        if(self_touch)
            self->info->touch((ENTITY)self, (ENTITY)other);
        if(other_touch)
            other->info->touch((ENTITY)other, (ENTITY)self);

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
static void P_entity_modelplay(entity_t * entity)
{
    const entityinfo_t * info = entity->info;
    size_t imodel;
    for(imodel = 0; imodel < info->models_num; imodel++)
    {
        entity_model_t * model = &entity->models[imodel];
        if(!(
                model->model != NULL &&
                model->model->frames != NULL &&
                model->model->fps > 0 &&
                model->player.play_frames_seq != NULL
        ))
            continue;

        const entity_framessequence_t * fseq = model->player.play_frames_seq;
        bool start;
        bool end;
        model_nextframe(
            &model->player.frame,
            model->model->fps,
            model->player.play_frames_seq->firstframe,
            model->player.play_frames_seq->lastframe,
            &start, &end
        );
        if(start && fseq != NULL && fseq->firstframef != NULL)
        {
            fseq->firstframef(
                (ENTITY)entity,
                imodel
            );
        }

        if(end && fseq != NULL && fseq->lastframef != NULL)
        {
            model->player.play_frames_seq = NULL;
            fseq->lastframef(
                (ENTITY)entity,
                imodel
            );
        }

    }
}

/**
 * передвижение игрока
 */
static void P_entity_move(entity_t * entity)
{
    if(!(entity->flags | ENTITYFLAG_SOLIDWALL))
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
    entity_t * entity = CIRCLEQ_FIRST(&entities);
    while(!CIRCLEQ_END(entity, &entities))
    {

        const entityinfo_t * info = entity->info;

        entity_vars_common_t * vars = entity->vars;
        if(entity->erase)
        {
            entity_t * erased = entity;
            /* укажем нового родителя, взамен удаляемого */
            reparent(erased, erased->parent);
            entity_model_play_pause_all((ENTITY)erased);
            entity = CIRCLEQ_NEXT(entity, list);
            CIRCLEQ_REMOVE(&entities, erased, list);
            CIRCLEQ_INSERT_TAIL(&entities_erased, erased, list);
            continue;
        }

        if(
                entity->freezed ||
                !entity->spawned
        )
        {
            entity = CIRCLEQ_NEXT(entity, list);
            continue;
        }

        VEC2_COPY(vars->origin_prev, vars->origin);
        if(info->handle != NULL)
        {
            info->handle((ENTITY)entity);
        }

        P_entity_touchs(entity);

        if(!entity->erase)
            P_entity_move(entity);

        if(!entity->erase)
            P_entity_modelplay(entity);

        entity = CIRCLEQ_NEXT(entity, list);
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

    entity_vars_common_t * common = entity->vars;
    vec2_t * pos = &common->origin;
    size_t models_num = entity->info->models_num;
    entity_model_t * models = entity->models;

    if(!models)
        return;

    size_t i;

    for( i = 0; i < models_num; i++ )
    {
        entity_model_t * model = &models[i];
        if(!model->model)
        {
            game_console_send("Error: Entity " ENTITY_PRINTF_FORMAT ", no imodel %d.", ENTITY_PRINTF_VALUE(entity), (int)i);
            continue;
        }
        int frame = VEC_TRUNC(model->player.frame);
        direction_t dir = common->dir;
        model_render(
            cam,
            *pos,
            model->model,
            model->scale,
            model->translation,
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

    entity_t * entity;
    CIRCLEQ_FOREACH(entity, &entities, list)
    {
        if(entity->erase) continue;
        if(entity->freezed) continue;
        if(!entity->spawned) continue;


        //int viewbox_half = entity->img->img_sx;
        int viewbox_half = 16;

        entity_vars_common_t * common = entity->vars;

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

    //game_console_send("ent_rendered = %d\n", ent_rendered);
}

/**
 * @description удаление всех объектов
 */
void entities_erase(void)
{
    entity_t * entity;
    while(!CIRCLEQ_EMPTY(&entities))
    {
        entity = CIRCLEQ_FIRST(&entities);
        CIRCLEQ_REMOVE(&entities, entity, list);
        entity_freemem(entity);
    }
    while(!CIRCLEQ_EMPTY(&entities_erased))
    {
        entity = CIRCLEQ_FIRST(&entities_erased);
        CIRCLEQ_REMOVE(&entities_erased, entity, list);
        entity_freemem(entity);
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

    const entityinfo_t * info = entityinfo_reg->info;

    entity_t * entity = Z_malloc(sizeof(entity_t));

    assert(entityId_last < ENTITY_ID_MAX && "Entities id are over!");
    entity->id = entityId_last;
    entityId_last++;
    strncpy(entity->classname, name, ENTITY_CLASSNAME_SIZE);
    entity->flags = 0;
    entity->bodybox = 0.0f;

    if(info->models_num == 0)
    {
        entity->models = NULL;
    }
    else
    {
        entity->models = Z_malloc(info->models_num * sizeof(entity_model_t));
        for( i = 0; i < info->models_num; i++)
        {
            entity->models[i].name = NULL;
            entity->models[i].player.frame = 0.0f;
            entity->models[i].player.fseq = NULL;
            entity->models[i].player.play_frames_seq = NULL;
        }
    }

    entity->info = info;

    entity->vars = Z_malloc(info->vars_size);

    entity->parent = (entity_t*)parent;
    entity->cam_entity = entity;
    entity->erase = false;
    entity->freezed = false;
    entity->allow_draw = true;

    CIRCLEQ_INSERT_TAIL(&entities, entity, list);


    entity->spawned = false;

    entity_vars_common_t * common = entity->vars;
    void * storage = entity_build_storage(name, vars_values, vars_values_num);
    entity_restore((ENTITY)entity, storage);
    if(info->init)
        info->init((ENTITY)entity, (ENTITY)parent);
    if(info->spawn)
        info->spawn((ENTITY)entity);

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
    entity_t * entity;
    CIRCLEQ_FOREACH(entity, &entities, list)
    {

        const entityinfo_t * info = entity->info;
        if(!info->player_spawn)
            continue;

        entity_t * ent = (entity_t*)info->player_spawn((ENTITY)entity, storage);
        return ent;
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
    entity_vars_common_t * common = entity->vars;
    common->alive = true;
}


