/*
 * mobjs.c
 *
 *  Created on: 29 нояб. 2016 г.
 *      Author: mastersan
 */

#include "gamedefs.h"
#include "entity_internal.h"
#include "game.h"
#include "entity.h"
#include "model.h"
#include "common/common_hash.h"
#include "vars.h"

var_descr_t entity_common_vars[] =
{
        ENTITY_VARS_COMMON,
};

/**
 * @brief восстановление информации о entity игрока из хранилища игрока в entity
 * @brief (при переходе на следующий уровень и при чтении gamesave)
 */
void entity_restore(ENTITY entity, const void * vars)
{
    if(!vars)
        return;
    entity_t * ent = (entity_t *)entity;
    size_t info_vars_num = ent->info->vars_descr_num;
    const var_descr_t * info_vars = ent->info->vars_descr;

    for(size_t i = 0; i < info_vars_num; i++)
    {
        intptr_t ofs = info_vars[i].ofs;
        memcpy(ent->vars + ofs, vars + ofs, info_vars[i].size);
    }

#ifdef VARS_DUMP_ALLOW
    char title[128];
    sprintf(title, "==== Entity \"%s\" vars:", ent->info->classname);
    VARS_DUMP(ent->vars, ent->info->vars_descr, ent->info->vars_descr_num, title);
#endif
}

void entity_vars_descr_get(ENTITY entity, const var_descr_t ** vars_descr, size_t * vars_descr_num)
{
    entity_t * ent = (entity_t *)entity;
    *vars_descr = ent->info->vars_descr;
    *vars_descr_num = ent->info->vars_descr_num;
}

/* сравнить класс entity */
int entity_classname_cmp(const ENTITY entity, const char * classname)
{
    return strncmp(((entity_t*)entity)->classname, classname, ENTITY_CLASSNAME_SIZE );
}

const char * entity_classname_get(const ENTITY entity)
{
    return ((entity_t *)entity)->classname;
}

void entity_erase(ENTITY entity)
{
    ((entity_t*)entity)->erase = true;
}

bool entity_is_spawned(ENTITY entity)
{
    return ((entity_t*)entity)->spawned;
}

void entity_unspawn(ENTITY entity)
{
    ((entity_t*)entity)->spawned = false;
}

void entity_show(ENTITY entity)
{
    ((entity_t*)entity)->allow_draw = true;
}

void entity_hide(ENTITY entity)
{
    ((entity_t*)entity)->allow_draw = false;
}

ENTITY entity_parent(ENTITY entity)
{
    if(!entity) return NULL;
    return (ENTITY)((entity_t*)entity)->parent;
}

void * entity_vars(ENTITY entity)
{
    return ((entity_t*)entity)->vars;
}

void entity_cam_set(ENTITY entity, ENTITY cam_entity)
{
    ((entity_t*)entity)->cam_entity = (entity_t*)cam_entity;
}

void entity_cam_reset(ENTITY entity)
{
    ((entity_t*)entity)->cam_entity = (entity_t*)entity;
}

ENTITY entity_spawn(const char * classname, ENTITY parent)
{
    return (ENTITY)entity_new_(classname, (entity_t*)parent, NULL, 0);
}

void entity_flags_set(ENTITY entity, int flags)
{
    ((entity_t*) entity)->flags = flags;
}

void entity_bodybox_set(ENTITY entity, FLOAT bodybox)
{
    if(bodybox < 0.0f)
        bodybox = 0.0f;
    ((entity_t*) entity)->bodybox = bodybox;
}

FLOAT entity_bodybox_get(const ENTITY entity)
{
    return ((entity_t *)entity)->bodybox;
}

/**
 * @description установить модель на entity
 */
int entity_model_set(
    ENTITY entity,
    unsigned int imodel,
    const char * modelname,
    FLOAT modelscale,
    FLOAT translation_x,
    FLOAT translation_y
)
{
    entity_t * ent = (entity_t*)entity;
    if(imodel >= ent->info->models_num)
    {
        game_console_send("Error: Entity " ENTITY_PRINTF_FORMAT ": no model index #%d, could not set model", ENTITY_PRINTF_VALUE(ent), imodel);
        return -1;
    }

    entity_model_t * entity_model = &ent->models[imodel];
    Z_free(entity_model->name);
    entity_model->name = NULL;

    model_t * model = (model_t*)model_get(modelname);
    if(!model)
    {
        game_console_send("Error: Entity " ENTITY_PRINTF_FORMAT ", model index #%d: could not load model \"%s\".", imodel, ENTITY_PRINTF_VALUE(ent), modelname);
    }
    else
    {
        entity_model->name = Z_strdup(modelname);
    }

    entity_model->model = model;
    entity_model->scale = modelscale;
    VEC2_SET(entity_model->translation, translation_x, translation_y);
    entity_model->player.fseq = NULL;
    entity_model->player.play_frames_seq = NULL;
    entity_model->player.frame = 0.0f;
    return 0;
}

int entity_model_sequence_set(ENTITY entity, unsigned int imodel, const entity_framessequence_t * fseq)
{
    entity_t * ent = (entity_t*)entity;

    const game_exports_entityinfo_t * info = ent->info;
    if(imodel >= info->models_num)
    {
        game_console_send("Error: Entity " ENTITY_PRINTF_FORMAT ": no model index #%d, could not set frames sequence.",
            ENTITY_PRINTF_VALUE(ent),
            imodel
        );
        return -1;
    }

    entity_model_t * model = &ent->models[imodel];
    model->player.fseq = fseq;
    if(fseq == NULL)
        model->player.play_frames_seq = NULL;
    return 0;
}

void entity_done_set(ENTITY entity, void (*done)(ENTITY self))
{
    entity_t * ent = (entity_t *)entity;
    ent->done = done;
}

void entity_thinker_set(ENTITY entity, void (*think)(ENTITY self))
{
    entity_t * ent = (entity_t *)entity;
    ent->think = think;
}

void entity_toucher_set(ENTITY entity, void (*touch)(ENTITY self, ENTITY other))
{
    entity_t * ent = (entity_t *)entity;
    ent->touch = touch;
}

/**
 * @description начать/возобновить проигрывание кадров модели
 */
void entity_model_play_start(ENTITY entity, unsigned int imodel)
{
    entity_t * ent = (entity_t*)entity;
    const game_exports_entityinfo_t * info = ent->info;
    if(imodel >= info->models_num)
    {
        game_console_send("Error: Entity " ENTITY_PRINTF_FORMAT ": no model index #%d, could not play frames sequence.",
            ENTITY_PRINTF_VALUE(ent),
            imodel
        );
        return;
    }

    entity_model_t * model = &ent->models[imodel];

    const entity_framessequence_t * fseq = model->player.fseq;
    if(!fseq)
    {
        game_console_send("Error: Entity " ENTITY_PRINTF_FORMAT ", model index #%d: no frames sequence, could not start play.",
            ENTITY_PRINTF_VALUE(ent),
            imodel);
        return;
    }

    if(model->player.play_frames_seq == NULL)
    {
        /* если действия нет или закончилось, начнём действие заново */
        model->player.play_frames_seq = fseq;
        model->player.frame = fseq->firstframe;
    }
    else
    {
        model->player.play_frames_seq = fseq;
        unsigned int frame = model->player.frame;
        /* coerce */
        if( frame < fseq->firstframe || fseq->lastframe + 1 <= frame )
            model->player.frame = fseq->firstframe;
    }

}

/**
 * @description приостановить проигрывание кадров модели
 */
void entity_model_play_pause(ENTITY entity, unsigned int imodel)
{
    entity_t * ent = (entity_t *)entity;
    const game_exports_entityinfo_t * info = ent->info;
    if(imodel >= info->models_num)
    {
        game_console_send("Error: Entity " ENTITY_PRINTF_FORMAT ": no model index #%d, could not pause frames.",
            ENTITY_PRINTF_VALUE(ent),
            imodel
        );
        return;
    }
    ent->models[imodel].player.play_frames_seq = NULL;
}

void entity_model_play_pause_all(ENTITY entity)
{
    entity_t * ent = (entity_t *)entity;
    const game_exports_entityinfo_t * info = ent->info;
    int imodel;
    for(imodel = 0; imodel < info->models_num; imodel++ )
    {
        ent->models[imodel].player.play_frames_seq = NULL;
    }
}


/**
 * получить первый entity, соответствующий name
 */
ENTITY entity_first(void)
{
    return (ENTITY)CIRCLEQ_FIRST(&entities);
}

/**
 * получить первый entity, соответствующий name
 */
ENTITY entity_next(ENTITY entity)
{
    if(CIRCLEQ_END((entity_t*)entity, &entities))
        return CIRCLEQ_NULL(&entities);
    return (ENTITY)CIRCLEQ_NEXT((entity_t*)entity, list);
}

/**
 * получить первый entity, соответствующий name
 */
ENTITY entity_first_classname(const char * classname)
{
    entity_t * entity;
    CIRCLEQ_FOREACH(entity, &entities, list)
    {
        if(entity_classname_cmp((ENTITY)entity, classname) == 0)
            return (ENTITY)entity;
    }

    return CIRCLEQ_NULL(&entities);
}

/**
 * получить первый entity, соответствующий name
 */
ENTITY entity_next_classname(ENTITY entity, const char * classname)
{
    if(CIRCLEQ_END((entity_t*)entity, &entities))
        return (ENTITY)CIRCLEQ_NULL(&entities);
    entity_t * ent = (entity_t*)entity;
    for(
            ent = CIRCLEQ_NEXT(ent, list);
            !CIRCLEQ_END(ent, &entities) && entity_classname_cmp((ENTITY)ent, classname) != 0;
            ent = CIRCLEQ_NEXT(ent, list)
    );
    return (ENTITY)ent;
}

bool entity_end(ENTITY entity)
{
    return CIRCLEQ_END(entity, &entities);
}
