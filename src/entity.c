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
int entity_model_set(ENTITY entity, unsigned int imodel, const char * modelname)
{
    entity_t * ent = (entity_t*)entity;
    if(imodel >= ent->info->entmodels_num)
        return -1;
    const model_t * model = model_get(modelname);
    if(!model)
    {
        game_console_send("Error: Entity model set: No model \"%s\"", modelname);
    }
    ent->modelplayers[imodel].model = model;
    ent->modelplayers[imodel].frame = 0.0f;
    return 0;
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

#ifdef VARS_DUMP
    char title[128];
    sprintf(title, "==== Entity \"%s\" vars:", ent->info->name);
    vars_dump(ent->vars, ent->info->vars_descr, ent->info->vars_descr_num, title);
#endif
}

/* entity является объектом типа entity_name */
BOOL entity_is(const ENTITY entity, const char * entity_name)
{
        return strncmp(
            ((entity_t*)entity)->info->name,
            entity_name, ENTITY_NAME_SIZE ) == 0;
}

const char * entity_info_name(const ENTITY entity)
{
    return ((entity_t *)entity)->info->name;
}

VECTOR1 entity_info_bodybox(const ENTITY entity)
{
    return ((entity_t *)entity)->info->bodybox;
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




/**
 * @description начать/возобновить проигрывание кадров модели
 */
void entity_model_play_start(ENTITY entity, unsigned int modelId, const char * seqname)
{
    entity_t * ent = (entity_t*)entity;
    const entityinfo_t * info = ent->info;
    if(modelId >= info->entmodels_num)
    {
        game_console_send("Error: Entity \"%s\": modelId %d not found, could not play frames sequence.",
            info->name,
            modelId
        );
        return;
    }
    const entity_framessequence_t * framesseq = entity_reginfo_framessequence_get(info, modelId, seqname);
    if(!framesseq)
    {
        game_console_send("Error: Entity \"%s\", modelId %d, frames sequence \"%s\" not found, could not play frames sequence.",
            info->name, modelId, seqname);
        return;
    }

    entity_modelplayer_t * modelplayer = &ent->modelplayers[modelId];
    if(modelplayer->framesseq == NULL)
    {
        /* если действия нет или закончилось, начнём действие заново */
        modelplayer->framesseq = framesseq;
        modelplayer->frame = framesseq->firstframe;
    }
    else
    {
        modelplayer->framesseq = framesseq;
        float frame = modelplayer->frame;
        if( frame < framesseq->firstframe || framesseq->lastframe + 1 <= frame )
            modelplayer->frame = framesseq->firstframe;
    }

}

/**
 * @description приостановить проигрывание кадров модели
 */
void entity_model_play_pause(ENTITY entity, unsigned int imodel)
{
    entity_t * ent = (entity_t *)entity;
    const entityinfo_t * info = ent->info;
    if(imodel >= info->entmodels_num)
    {
        game_console_send("Error: Entity \"%s\": imodel %d not found, could not pause frames.",
            info->name,
            imodel
        );
        return;
    }
    ent->modelplayers[imodel].framesseq = NULL;
}

void entity_model_play_pause_all(ENTITY entity)
{
    entity_t * ent = (entity_t *)entity;
    const entityinfo_t * info = ent->info;
    int imodel;
    for(imodel = 0; imodel < info->entmodels_num; imodel++ )
    {
        ent->modelplayers[imodel].framesseq = NULL;
    }
}


/**
 * получить первый entity, соответствующий name
 */
ENTITY entity_first(const char * name)
{
    entity_registered_t * entinfo = entityregisteredinfo_get(name);
    if(entinfo == NULL)
    {
        game_console_send("Error: Unknown entity \"%s\".", name);
        return NULL;
    }
    return (ENTITY)entinfo->entities;
}

/**
 * получить первый entity, соответствующий name
 */
ENTITY entity_next(ENTITY entity)
{
    return (ENTITY)((entity_t*)entity)->next;
}

