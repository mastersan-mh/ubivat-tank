/*
 * entity_helpers.c
 *
 *  Created on: 9 янв. 2017 г.
 *      Author: mastersan
 */

#include "types.h"
#include "system.h"
#include "entity_helpers.h"
#include "game.h"
#include "entity_internal.h"
#include "world.h"

/**
 * получить любой объект из заданной группы
 */
ENTITY entity_get_random(const char * entityname)
{
    size_t count = 0;
    ENTITY entity;
    /* считаем количество */
    ENTITIES_FOREACH_NAME(entityname, entity)
    {
        count++;
    };
    if(count == 0)
        return NULL;
    count = xrand(count);

    /* выбираем случайным образом */
    ENTITIES_FOREACH_NAME(entityname, entity)
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
void entity_move(ENTITY this, direction_t dir, vec_t speed, bool check_clip)
{
    map_t * map = world_map_get();

    entity_t * ent = (entity_t *)this;
    entity_vars_common_t * common = ent->vars;
    vec_t dway = speed * dtimed1000;
    vec_t bodybox = ent->info->bodybox;
    vec_t halfbox = bodybox/2;
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
