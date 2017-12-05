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

static game_exports_t * ge = NULL;

#define GAME_PROGS_PROTECT() \
    do { \
        if(!ge) game_halt("game exports is NULL!"); \
    } while (0)


static void P_game_entity_register(const game_exports_entityinfo_t * info)
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

    if(info->classname == NULL || strnlen(info->classname, ENTITY_CLASSNAME_SIZE) == 0)
    {
        game_console_send("Entity registration failed: entity name is empty.");
        return;
    }

    if(entityregisteredinfo_get(info->classname) != NULL)
    {
        game_console_send("Entity \"%s\" registration failed: duplicate name \"%s\"", info->classname);
        return;
    }

    if(info->vars_size == 0)
    {
        game_console_send("Entity \"%s\" registration failed: Invalid descriptor: .vars_size == 0.", info->classname);
        return;
    }
    if(info->vars_descr_num == 0)
    {
        game_console_send("Entity \"%s\" registration failed: Invalid descriptor: .vars_descr_num == 0.", info->classname);
        return;
    }
    if(info->vars_descr == NULL)
    {
        game_console_send("Entity \"%s\" registration failed: Invalid descriptor: .vars_descr == NULL.", info->classname);
        return;
    }

    if(!vars_descr_eq(info->vars_descr, entity_common_vars , ARRAYSIZE(entity_common_vars)))
    {
        game_console_send("Entity \"%s\" registration failed: Invalid entity common part.", info->classname);
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
                    game_console_send("Entity \"%s\" registration failed: Duplicate variable name \"%s\".", info->classname, info->vars_descr[i].name);
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
        if(!tmp)game_halt("Entity \"%s\" registration failed: Out of memory", info->classname);
        entityregs = tmp;
    }
    entityregs[entityregs_num].info = info;
    entityregs_num++;

    game_console_send("Entity \"%s\" registered.", info->classname);

}


void game_progs_init()
{
     ge = progs_init();
     if(!ge)
     {
         game_halt("game context is NULL!");
     }

     size_t i;
     for(i = 0; i < ge->infos_num; i++)
     P_game_entity_register(&ge->infos[i]);

}

const entity_action_t * game_player_action_find(const char * action_str)
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

bool game_client_player_connect(const char * clientinfo)
{
    GAME_PROGS_PROTECT();
    if(!ge->client_player_connect)
        return true;
    return ge->client_player_connect(clientinfo);
}

entity_t * game_client_player_spawn(const char * spawninfo)
{
    GAME_PROGS_PROTECT();
    if(!ge->client_player_spawn)
        return NULL;
    return (entity_t*)ge->client_player_spawn(spawninfo);
}

void game_entity_on_read(const char *classname, const char * info)
{
    GAME_PROGS_PROTECT();
    if(!ge->entity_on_read)
        return;
    ge->entity_on_read(classname, info);
}
