/*
 * entity_helpers.c
 *
 *  Created on: 9 янв. 2017 г.
 *      Author: mastersan
 */

#include "helpers.h"
#include "progs.h"

void coerce_value_int(INTEGER * val, INTEGER min, INTEGER max)
{
    if(*val > max) *val = max;
    else if(*val < min) *val = min;
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
    ENTITY entity,
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

    void * vars = entity_vars(entity);
    const var_descr_t * vars_descr;
    size_t vars_descr_num;
    entity_vars_descr_get(entity, &vars_descr, &vars_descr_num);

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
            game_console_send(
                "Error: The length of the key is %ld, which is greater or equal than its size, equal to %ld.",
                size, UT_KEYSIZE
            );
            continue;
        }
        strlcpy(key, key_begin, size);

        size = value_end - value_begin + 1;
        if(size > UT_VALUESIZE)
        {
            game_console_send(
                "Warning: Key \"%s\", length of the value is %ld, which is greater or equal than its size, equal to %ld.",
                key, size, UT_KEYSIZE
            );
            game_console_send("Info: Key \"%s\" set to the default value.", key);
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
ENTITY entity_get_random(const char * entityname)
{
    size_t count = 0;
    ENTITY entity;
    /* считаем количество */
    ENTITIES_FOREACH_CLASSNAME(entity, entityname)
    {
        count++;
    };
    if(count == 0)
        return NULL;
    count = xrand(count);

    /* выбираем случайным образом */
    ENTITIES_FOREACH_CLASSNAME(entity, entityname)
    {
        if(count == 0)
            return entity;
        count--;
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
void entity_move(ENTITY entity, direction_t dir, vec_t speed, bool check_clip)
{
    map_t * map = world_map_get();

    entity_t * ent = (entity_t *)entity;
    entity_vars_common_t * common = ent->vars;
    vec_t dway = speed * dtimed1000;
    FLOAT bodybox = ent->bodybox;
    FLOAT halfbox = bodybox/2;
    vec_t dist;

    if(check_clip) /* FIXME: костыль */
    {
        /* найдем препятствия */
        map_clip_find_near(map, common->origin, bodybox, dir, MAP_WALL_CLIP, bodybox, &dist);
        if(dist < dway + halfbox)
            dway = dist - halfbox;
    }

    switch(dir)
    {
        case DIR_UP   : common->origin[1] += dway; break;
        case DIR_DOWN : common->origin[1] -= dway; break;
        case DIR_LEFT : common->origin[0] -= dway; break;
        case DIR_RIGHT: common->origin[0] += dway; break;
    }
    //подсчитываем пройденный путь
    common->stat_traveled_distance += VEC_ABS(dway);
}
