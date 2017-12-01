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
            ENTITY_VARS_COMMON
    };

    ssize_t i;
    entity_registered_t * tmp;
    if(info == NULL)
    {
        game_console_send("Entity registration failed: Register data is NULL.");
        return;
    }

    if(info->name_ == NULL || strnlen(info->name_, ENTITY_CLASSNAME_SIZE) == 0)
    {
        game_console_send("Entity registration failed: entity name is empty.");
        return;
    }

    if(entityregisteredinfo_get(info->name_) != NULL)
    {
        game_console_send("Entity \"%s\" registration failed: duplicate name \"%s\"", info->name_);
        return;
    }

    if(info->vars_size == 0)
    {
        game_console_send("Entity \"%s\" registration failed: Invalid descriptor: .vars_size == 0.", info->name_);
        return;
    }
    if(info->vars_descr_num == 0)
    {
        game_console_send("Entity \"%s\" registration failed: Invalid descriptor: .vars_descr_num == 0.", info->name_);
        return;
    }
    if(info->vars_descr == NULL)
    {
        game_console_send("Entity \"%s\" registration failed: Invalid descriptor: .vars_descr == NULL.", info->name_);
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
        game_console_send("Entity \"%s\" registration failed: Invalid entity common part.", info->name_);
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
                    game_console_send("Entity \"%s\" registration failed: Duplicate variable name \"%s\".", info->name_, info->vars_descr[i].name);
                    free(varnames_hashs);
                    return;
                }
            }
        }
        free(varnames_hashs);
    } while(0);

    if(entityregs_size < entityregs_num + 1)
    {
        if(entityregs_size == 0)
            entityregs_size = 1;
        else
            entityregs_size *= 2;
        tmp = Z_realloc(entityregs, sizeof(entity_registered_t) * entityregs_size);
        if(!tmp)game_halt("Entity \"%s\" registration failed: Out of memory", info->name_);
        entityregs = tmp;
    }
    entityregs[entityregs_num].info = info;
    entityregs_num++;

    game_console_send("Entity \"%s\" registered.", info->name_);

}

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

ENTITY entity_new(const char * name, ENTITY parent)
{
    return (ENTITY)entity_new_(name, (entity_t*)parent, NULL, 0);
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

VECTOR1 entity_bodybox_get(const ENTITY entity)
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
    VECTOR1 translation_x,
    VECTOR1 translation_y
)
{
    entity_t * ent = (entity_t*)entity;
    if(imodel >= ent->info->models_num)
    {
        game_console_send("Error: Entity " ENTITY_PRINTF_FORMAT " can't set model index %d", ENTITY_PRINTF_VALUE(ent), imodel);
        return -1;
    }

    entity_model_t * entity_model = &ent->models[imodel];
    Z_free(entity_model->name);
    entity_model->name = NULL;

    model_t * model = (model_t*)model_get(modelname);
    if(!model)
    {
        game_console_send("Error: Entity " ENTITY_PRINTF_FORMAT " can't set model \"%s\", can't load model.", ENTITY_PRINTF_VALUE(ent), modelname);
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

    const entityinfo_t * info = ent->info;
    if(imodel >= info->models_num)
    {
        game_console_send("Error: Entity " ENTITY_PRINTF_FORMAT ": model index %d not found, could not set frames sequence.",
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

/**
 * @description начать/возобновить проигрывание кадров модели
 */
void entity_model_play_start(ENTITY entity, unsigned int imodel)
{
    entity_t * ent = (entity_t*)entity;
    const entityinfo_t * info = ent->info;
    if(imodel >= info->models_num)
    {
        game_console_send("Error: Entity " ENTITY_PRINTF_FORMAT ": model index %d not found, could not play frames sequence.",
            ENTITY_PRINTF_VALUE(ent),
            imodel
        );
        return;
    }

    entity_model_t * model = &ent->models[imodel];

    const entity_framessequence_t * fseq = model->player.fseq;
    if(!fseq)
    {
        game_console_send("Error: Entity " ENTITY_PRINTF_FORMAT ", model index %d, no frames sequence, could not start play.",
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
    const entityinfo_t * info = ent->info;
    if(imodel >= info->models_num)
    {
        game_console_send("Error: Entity " ENTITY_PRINTF_FORMAT ": imodel %d not found, could not pause frames.",
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
    const entityinfo_t * info = ent->info;
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
