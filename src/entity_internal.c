/*
 * entity_internal.c
 *
 *  Created on: 12 нояб. 2017 г.
 *      Author: mastersan
 */
#include "entity_internal.h"
#include "entity.h"
#include "game.h"
#include "sound.h"

#include "game_progs.h" /* TODO: delete it; extern game_exports_t * ge; */

#include <assert.h>

/* существующие объекты (все одного типа) */
body_head_t bodies = CIRCLEQ_HEAD_INITIALIZER(bodies);

static size_t bodies_amount = 0;


static size_t body_last_id = 0;

/**
 * @description добавление объекта
 */
void body_link_entity(void * entity)
{
    if(!entity)
        return;
    body_t * body = Z_malloc(sizeof(body_t));

    assert(bodies_amount < BODY_ID_MAX && "Entities id are over!");

    bodies_amount++;
    body->id = body_last_id++;
    body->entity = entity;

    CIRCLEQ_INSERT_TAIL(&bodies, body, list);
}

/**
 * @description добавление объекта
 */
void body_unlink_entity(void * entity)
{
    if(!entity)
        return;

    body_t * body;
    body_t * found = NULL;

    CIRCLEQ_FOREACH(body, &bodies, list)
    {
        if(body->entity == entity)
        {
            found = body;
            break;
        }
    }

    if(!found)
        return;

    CIRCLEQ_REMOVE(&bodies, found, list);
    bodies_amount--;
    Z_free(found);
}

void entities_handle(void)
{
    body_t * body;
    CIRCLEQ_FOREACH(body, &bodies, list)
    {
/* resolve collisions */
    }
}


body_t * body_find_by_id(size_t ibody)
{
    size_t i = 0;
    body_t * body;
    CIRCLEQ_FOREACH(body, &bodies, list)
    {
        if(body->id == ibody)
            return body;
    }
    return NULL;
}

/**
 * @brief Очистка памяти entity
 */
static void body_freemem(body_t * body)
{
    sound_play_stop(body->entity, -1);

    /*
    size_t i;
    for( i = 0; i < info->models_num; i++)
    {
        Z_free(body->models[i].name);
    }
    Z_free(body->models);
    */
    Z_free(body);
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
 *
static void P_entity_modelplay(body_t * entity)
{
    const game_exports_entityinfo_t * info = entity->info;
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
                (entity_t *)entity,
                imodel
            );
        }

        if(end && fseq != NULL && fseq->lastframef != NULL)
        {
            model->player.play_frames_seq = NULL;
            fseq->lastframef(
                (entity_t *)entity,
                imodel
            );
        }

    }
}
*/

/**
 * передвижение игрока
 */
static void P_entity_move(body_t * entity)
{
/*
    if(!(entity->flags | ENTITYFLAG_SOLIDWALL))
        return;
*/
    /*
    vec2_t * orig = &entity->c.origin;
     */
    /*
    vec2_t * orig_prev = &entity->c.origin_prev;
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


static void ent_models_render(
    camera_t * cam,
    body_t * entity
)
{
/*
    static GLfloat angles[] =
    {
            0.0f,   // * N *
            180.0f, // * S *
            270.0f, // * W *
            90.0f,  // * E *
    };

    entity_common_t * common = entity->entity_;
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
            game_cprint("Error: Entity " ENTITY_PRINTF_FORMAT ": no model index #%d, could not render.", ENTITY_PRINTF_VALUE(entity), (int)i);
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
    */
}

/**
 * рисование объектов на карте
 */
void bodies_render(camera_t * cam)
{
    int cam_sx_half = cam->sx / 2;
    int cam_sy_half = cam->sy / 2;
    int ent_rendered = 0;

    body_t * entity;
    CIRCLEQ_FOREACH(entity, &bodies, list)
    {


        //int viewbox_half = entity->img->img_sx;
        int viewbox_half = 16;

        entity_common_t * common = entity->entity;

        if(
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

    //game_cprint("ent_rendered = %d\n", ent_rendered);
}

/**
 * @description удаление всех объектов
 */
void entities_erase(void)
{
    body_t * entity;
    while(!CIRCLEQ_EMPTY(&bodies))
    {
        entity = CIRCLEQ_FIRST(&bodies);
        CIRCLEQ_REMOVE(&bodies, entity, list);
        body_freemem(entity);
    }
}

