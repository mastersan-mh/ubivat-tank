/*
 * entity_helpers.c
 *
 *  Created on: 9 янв. 2017 г.
 *      Author: mastersan
 */

#include "helpers.h"
#include "progs.h"
#include "progs_main.h"

void coerce_value_int(INTEGER * val, INTEGER min, INTEGER max)
{
    if(*val > max) *val = max;
    else if(*val < min) *val = min;
}

entity_t * entity_init(const char * classname, entity_t * owner)
{
    size_t i;
    for(i = 0; i < g_entities_num; i++)
    {
        entity_t * entity = &g_entities[i];
        if(!entity->used)
        {
            entity->classname = classname;
            entity->used = true;
            entity->owner = owner;
            return entity;
        }
    }
    return NULL;
}

void entity_done(entity_t * entity)
{
    entity->classname = "noclass";
    entity->used = false;
}

void entity_cam_set(entity_t *entity, entity_t *cam_entity)
{
    entity->c.client->cam_target = (entity_common_t *)cam_entity;
}

void entity_cam_reset(entity_t *entity)
{
    entity->c.client->cam_target = (entity_common_t*)entity;
}

/* сравнить класс entity */
int entity_classname_cmp(const entity_t * entity, const char * classname)
{
    return strncmp(entity->classname, classname, ENTITY_CLASSNAME_SIZE );
}

#ifdef HAVE_STRLCPY
error We have strlcpy(), check the code before use it.
#else
/*
 * @brief As strncpy (copy no more than <n> bytes), but always set last byte (or n-1 byte) to zero in <dest>
 * @return total copied bytes in dest
 */
size_t strlcpy(char *__restrict dest, const char *__restrict src, size_t n)
{
    size_t srcsize = strnlen(src, n) + 1;
    n = MIN(srcsize, n);
    n--;
    memcpy(dest, src, n);
    dest[n] = '\0';
    return n + 1;
}
#endif

char * s_snprintf(char * str, size_t n, const char *format, ...)
{
    va_list argptr;
    va_start(argptr, format);
#ifdef HAVE_VSNPRINTF
    vsnprintf(str, n, format, argptr);
#else
    vsprintf(str, format, argptr);
#endif
    va_end(argptr);
    return str;
}

void entity_vars_set_all(
    entity_t * entity,
    const char * info
)
{
    if(!info)
        return;
    const char * ctx = info;
    const char * key_begin;
    const char * key_end;
    const char * value_begin;
    const char * value_end;
    size_t size;

    const var_descr_t * vars_descr;
    size_t vars_descr_num;
    gi->vars_descr_get(&vars_descr, &vars_descr_num);

    void * vars = entity;

    static char key[UT_KEYSIZE];
    static char value[UT_VALUESIZE];

    while(infovars_get_next_key(
        &ctx,
        &key_begin,
        &key_end,
        &value_begin,
        &value_end))
    {
        size = key_end - key_begin + 1;
        if(size > UT_KEYSIZE)
        {
            gi->cprint(
                "Error: The length of the key is %ld, which is greater or equal than its size, equal to %ld.",
                size, UT_KEYSIZE
            );
            continue;
        }
        strlcpy(key, key_begin, size);

        size = value_end - value_begin + 1;
        if(size > UT_VALUESIZE)
        {
            gi->cprint(
                "Warning: Key \"%s\", length of the value is %ld, which is greater or equal than its size, equal to %ld.",
                key, size, UT_KEYSIZE
            );
            gi->cprint("Info: Key \"%s\" set to the default value.", key);
            strcpy(value, "");
        }
        else
        {
            strlcpy(value, value_begin, MIN(size, UT_VALUESIZE));
        }

        var_set(vars, vars_descr, vars_descr_num, key, value);
    }
}

/**
 * получить любой объект из заданной группы
 */
entity_t * entity_get_random(const char * entityname)
{
    size_t count = 0;
    entity_t * entity;
    /* считаем количество */
    size_t i;
    ENTITIES_FOREACH(entity, i)
    {
        if(entity_classname_cmp(entity, entityname) == 0)
            count++;
    };
    if(count == 0)
        return NULL;
    count = xrand(count);

    /* выбираем случайным образом */
    ENTITIES_FOREACH(entity, i)
    {
        if(entity_classname_cmp(entity, entityname) == 0)
        {
            if(count == 0)
                return entity;
            count--;
        }
    };
    return NULL;
}

direction_t entity_direction_invert(direction_t dir)
{
    switch(dir)
    {
        case DIR_UP   : return DIR_DOWN;
        case DIR_DOWN : return DIR_UP;
        case DIR_LEFT : return DIR_RIGHT;
        case DIR_RIGHT: return DIR_LEFT;
    }
    return DIR_UP;
}
/*
 * передвижение игрока
 */
void entity_move(entity_t * entity, direction_t dir, vec_t speed, bool check_clip)
{
    map_t * map = world_map_get();

    vec_t dway = speed * gi->dtimed1000();
    FLOAT bodybox = entity->c.bodybox;
    FLOAT halfbox = bodybox/2;
    vec_t dist;

    if(check_clip) /* FIXME: костыль */
    {
        /* найдем препятствия */
        map_clip_find_near(map, entity->c.origin, bodybox, dir, MAP_WALL_CLIP, bodybox, &dist);
        if(dist < dway + halfbox)
            dway = dist - halfbox;
    }

    switch(dir)
    {
        case DIR_UP   : entity->c.origin[1] += dway; break;
        case DIR_DOWN : entity->c.origin[1] -= dway; break;
        case DIR_LEFT : entity->c.origin[0] -= dway; break;
        case DIR_RIGHT: entity->c.origin[0] += dway; break;
    }
    //подсчитываем пройденный путь
    entity->stat_traveled_distance += VEC_ABS(dway);
}

BOOL entities_in_contact(entity_t * ent1, entity_t * ent2)
{
    FLOAT ent1_halfbox = ent1->c.bodybox * 0.5;
    FLOAT ent2_halfbox = ent2->c.bodybox * 0.5;
    return
            ( ent1->c.origin[0] - ent1_halfbox <= ent2->c.origin[0] + ent2_halfbox ) &&
            ( ent2->c.origin[0] - ent2_halfbox <= ent1->c.origin[0] + ent1_halfbox ) &&
            ( ent1->c.origin[1] - ent1_halfbox <= ent2->c.origin[1] + ent2_halfbox ) &&
            ( ent2->c.origin[1] - ent2_halfbox <= ent1->c.origin[1] + ent1_halfbox )
            ;
}



/**
 * @description установить модель на entity
 */
int entity_model_set(
    entity_t * entity,
    unsigned int imodel,
    const char * modelname,
    FLOAT modelscale,
    FLOAT translation_x,
    FLOAT translation_y
)
{
/*
    if(imodel >= entity->info->models_num)
    {
        game_cprint("Error: Entity " ENTITY_PRINTF_FORMAT ": no model index #%d, could not set model", ENTITY_PRINTF_VALUE(entity), imodel);
        return -1;
    }

    entity_model_t * entity_model = &entity->models[imodel];
    Z_free(entity_model->name);
    entity_model->name = NULL;

    model_t * model = (model_t*)model_get(modelname);
    if(!model)
    {
        game_cprint("Error: Entity " ENTITY_PRINTF_FORMAT ", model index #%d: could not load model \"%s\".", imodel, ENTITY_PRINTF_VALUE(entity), modelname);
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
*/
    return 0;
}

int entity_model_sequence_set(entity_t * entity, unsigned int imodel, const entity_framessequence_t * fseq)
{
    /*
    const game_exports_entityinfo_t * info = entity->info;
    if(imodel >= info->models_num)
    {
        game_cprint("Error: Entity " ENTITY_PRINTF_FORMAT ": no model index #%d, could not set frames sequence.",
            ENTITY_PRINTF_VALUE(entity),
            imodel
        );
        return -1;
    }

    entity_model_t * model = &entity->models[imodel];
    model->player.fseq = fseq;
    if(fseq == NULL)
        model->player.play_frames_seq = NULL;
        */
    return 0;
}

/**
 * @description начать/возобновить проигрывание кадров модели
 */
void entity_model_play_start(entity_t * entity, unsigned int imodel)
{
/*
    const game_exports_entityinfo_t * info = entity->info;
    if(imodel >= info->models_num)
    {
        game_cprint("Error: Entity " ENTITY_PRINTF_FORMAT ": no model index #%d, could not play frames sequence.",
            ENTITY_PRINTF_VALUE(entity),
            imodel
        );
        return;
    }

    entity_model_t * model = &entity->models[imodel];

    const entity_framessequence_t * fseq = model->player.fseq;
    if(!fseq)
    {
        game_cprint("Error: Entity " ENTITY_PRINTF_FORMAT ", model index #%d: no frames sequence, could not start play.",
            ENTITY_PRINTF_VALUE(entity),
            imodel);
        return;
    }

    if(model->player.play_frames_seq == NULL)
    {
        // если действия нет или закончилось, начнём действие заново
        model->player.play_frames_seq = fseq;
        model->player.frame = fseq->firstframe;
    }
    else
    {
        model->player.play_frames_seq = fseq;
        unsigned int frame = model->player.frame;
        // coerce
        if( frame < fseq->firstframe || fseq->lastframe + 1 <= frame )
            model->player.frame = fseq->firstframe;
    }
*/
}

/**
 * @description приостановить проигрывание кадров модели
 */
void entity_model_play_pause(entity_t * entity, unsigned int imodel)
{
/*
    const game_exports_entityinfo_t * info = entity->info;
    if(imodel >= info->models_num)
    {
        game_cprint("Error: Entity " ENTITY_PRINTF_FORMAT ": no model index #%d, could not pause frames.",
            ENTITY_PRINTF_VALUE(entity),
            imodel
        );
        return;
    }
    entity->models[imodel].player.play_frames_seq = NULL;
    */
}

void entity_model_play_pause_all(entity_t * entity)
{
/*
    const game_exports_entityinfo_t * info = entity->info;
    int imodel;
    for(imodel = 0; imodel < info->models_num; imodel++ )
    {
        entity->models[imodel].player.play_frames_seq = NULL;
    }
    */
}

