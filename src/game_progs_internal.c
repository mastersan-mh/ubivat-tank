/*
 * game_progs_internal.c
 *
 *  Created on: 1 дек. 2017 г.
 *      Author: mastersan
 */


#include "game_progs.h"
#include "game_progs_internal.h"
#include "game.h"
#include "vars.h"
#include "entity_internal.h"
#include "progs/progs_main.h"
#include "common/common_hash.h"

#include "sv_game.h"

#define GAME_PROGS_PROTECT() \
    do { \
        if(!ge) game_halt("game exports is NULL!"); \
    } while (0)

static game_inits_t init = {};
static game_imports_t gimport = {};
static game_exports_t * ge = NULL;

static long game_dtime(void)
{
    return dtime;
}

static double game_dtimed1000(void)
{
    return dtimed1000;
}

static void game_vars_descr_get(const var_descr_t ** vars_descr, size_t * vars_descr_num)
{
    *vars_descr = ge->vars_descr;
    *vars_descr_num = ge->vars_descr_num;
}

int game_progs_load(void)
{
    gimport.eprint = game_halt;
    gimport.cprint = game_cprint;
    gimport.centerprint = game_cprint;

    gimport.link = body_link_entity;
    gimport.unlink = body_unlink_entity;

    gimport.vars_descr_get = game_vars_descr_get;

    gimport.dtime = game_dtime;
    gimport.dtimed1000 = game_dtimed1000;
    gimport.sv_game_win = sv_game_win;
    gimport.sv_game_flag_localgame = sv_game_flag_localgame;
    gimport.sv_game_is_first_map = sv_game_is_first_map;

    ge = progs_init(&gimport);

    static var_descr_t entity_common_vars[] =
    {
            ENTITY_VARS_COMMON
    };

    if(!ge)
    {
        game_halt("game context is NULL!");
    }

    if(ge->vars_size == 0)
    {
        game_cprint("Game exports registration failed: Invalid descriptor: .vars_size == 0.");
        return -1;
    }
    if(ge->vars_descr_num == 0)
    {
        game_cprint("Game exports registration failed: Invalid descriptor: .vars_descr_num == 0.");
        return -1;
    }
    if(ge->vars_descr == NULL)
    {
        game_cprint("Game exports registration failed: Invalid descriptor: .vars_descr == NULL.");
        return -1;
    }

    if(!vars_descr_eq(ge->vars_descr, entity_common_vars , ARRAYSIZE(entity_common_vars)))
    {
        game_cprint("Game exports registration failed: Invalid entity common part.");
        return -1;
    }

    /* проверка переменных */
    size_t i;
    uint32_t * varnames_hashs = calloc(ge->vars_descr_num, sizeof(uint32_t));
    for(i = 0; i < ge->vars_descr_num; i++)
    {
        varnames_hashs[i] = HASH32(ge->vars_descr[i].name);
        ssize_t j;
        for(j = 0; j < i; j++)
        {
            if(varnames_hashs[i] == varnames_hashs[j])
            {
                game_cprint("Game exports registration failed: Duplicate variable name \"%s\".", ge->vars_descr[i].name);
                free(varnames_hashs);
                return -1;
            }
        }
    }
    free(varnames_hashs);

    return 0;
}

const entity_action_t * gamep_player_action_find(const char * action_str)
{
    GAME_PROGS_PROTECT();
    for(size_t i = 0; i < ge->actions_num; i++)
    {
        const entity_action_t * action = &ge->actions[i];
        if(ACTIONS_EQ(action->action, action_str))
        {
            return action;
        }
    }
    return NULL;

}

void game_progs_init(void)
{
    GAME_PROGS_PROTECT();
    if(!ge->init)
        return;

    init.clients_max = PLAYERS_MAX;
    init.entities_max = ENTITIES_MAX;
    ge->init(&init);
}

void game_progs_done(void)
{
    GAME_PROGS_PROTECT();
    if(!ge->done)
        return;
    ge->done();
}

size_t game_progs_clients_max_get(void)
{
    return init.clients_max;
}

size_t game_progs_entities_max_get(void)
{
    return init.entities_max;
}

void game_progs_world_create(void)
{
    GAME_PROGS_PROTECT();
    if(!ge->world_create)
        return;
    ge->world_create(ENTITIES_MAX);
}

void game_progs_world_destroy(void)
{
    GAME_PROGS_PROTECT();
    if(!ge->world_destroy)
        return;
    ge->world_destroy();
}

void game_progs_world_handle(void)
{
    GAME_PROGS_PROTECT();
    if(!ge->world_handle)
        return;
    ge->world_handle();
}

void game_progs_entity_on_read(const char *classname, const char * info)
{
    GAME_PROGS_PROTECT();
    if(!ge->entity_on_read)
        return;
    ge->entity_on_read(classname, info);
}

bool game_progs_client_connect(const char * clientinfo)
{
    GAME_PROGS_PROTECT();
    if(!ge->client_player_connect)
        return true;
    return ge->client_player_connect(clientinfo);
}

entity_common_t * game_progs_client_player_spawn(const char * spawninfo)
{
    GAME_PROGS_PROTECT();
    if(!ge->client_player_spawn)
        return NULL;
    return (entity_common_t*)ge->client_player_spawn(spawninfo);
}

entity_common_t * game_entity_find(size_t igclient)
{
    body_t * body;
    CIRCLEQ_FOREACH(body, &bodies, list)
    {
        entity_common_t * entity = body->entity;
        if(entity->client->iclient == igclient)
            return entity;
    }
    return NULL;
}

