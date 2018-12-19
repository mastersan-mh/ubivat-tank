/*
 * world.c
 *
 *  Created on: 23 нояб. 2017 г.
 *      Author: mastersan
 */

#include "world.h"

#include "entity_internal.h"
#include "game.h"
#include "game_progs_internal.h"
#include "Z_mem.h"
#include "client.h"
#include <stdlib.h>

static world_t world = {};

int world_create(const char * mapfilename)
{
    if(world.map)
    {
        game_cprint("Can not create world, it already created.");
        return -1;
    }
    game_cprint("Loading map \"%s\"", mapfilename);
    world.map = map_load(mapfilename);
    if(!world.map)
    {

        game_cprint("Error: Can not load world map \"%s\"", mapfilename);
        return -1;
    }
    world.mapfilename = Z_strdup(mapfilename);

    game_world_create();

    return 0;
}

void world_destroy(void)
{
    game_world_destroy();
    client_world_valid_set(false);
    Z_free(world.mapfilename);
    world.mapfilename = NULL;
    map_free(world.map);
    world.map = NULL;
}

bool world_valid(void)
{
    return world.map != NULL;
}

const char * world_mapfilename_get(void)
{
    return world.mapfilename;
}

map_t * world_map_get(void)
{
    return world.map;
}

void world_handle(void)
{
    if(!world_valid())
        return;
    entities_handle();
    game_world_handle();
}
