/*
 * server_private.c
 *
 *  Created on: 3 нояб. 2017 г.
 *      Author: mastersan
 */

#include "common/common_list2.h"

#include "server_private.h"
#include "server_reply.h"
#include "g_gamesave.h"

#include <stdlib.h>


server_client_t * server_client_find_by_addr(const net_addr_t * addr)
{
    server_client_t * client;
    LIST2_FOREACH(server.clients, client)
    {
        if( !memcmp(&client->ns.addr_.addr, &addr->addr, sizeof(struct sockaddr)) )
            return client;
    }
    return NULL;
}


void server_client_players_num_set(server_client_t * client, int players_num)
{
    client->players_num = players_num;
}

void server_client_disconnect(server_client_t * client)
{
    game_server_event_t event;
    event.type = G_SERVER_EVENT_CONNECTION_CLOSE;
    server_reply_send(client, &event);
}

void server_client_delete(server_client_t * client)
{
    server_player_t * player;
    while(!LIST2_IS_EMPTY(client->players))
    {
        player = client->players;
        LIST2_UNLINK(client->players, player);
        server_player_delete(player);
    }
    Z_free(client);
}

int server_client_join(server_client_t * client, int players_num)
{
    client->players_num = players_num;
    for(int i = 0; i < players_num; i++)
    {
        server_player_t * player = server_player_create();
        LIST2_PUSH(client->players, player);

        entity_t * entity = entries_client_join();
        if(!entity)
        {
            game_console_send("Error: No entity to spawn client.");
            return -1;
        }
        player->entity = entity;
        server_client_player_info_store(player);
    }
    client->joined = true;
    return 0;
}

int server_client_num_get(void)
{
    server_client_t * client;
    int num;
    LIST2_FOREACH_I(server.clients, client, num);
    return num;
}

int server_client_players_num_get(const server_client_t * client)
{
    server_player_t * player;
    int num;
    LIST2_FOREACH_I(client->players, player, num);
    return num;
}

/**
 * brief получить данные переменной
 */
vardata_t * server_client_vardata_get(server_player_t * player, const char * varname, vartype_t vartype)
{
    static const char * list[] =
    {
            "INTEGER",
            "FLOAT",
            "STRING",
    };
    var_t * var = var_find(player->vars, varname);
    if(!var)
    {
        var = var_create(&player->vars, varname, vartype);
    }
    vardata_t * vardata = var->data;
    if( (int)vartype >= 0 && vardata->type != vartype )
    {
        game_console_send("Warning: Host client variable \"%s\" has type %s, but used as %s.", varname, list[vardata->type], list[vartype]);
    }
    return vardata;
}

/**
 * @brief сохранение информации о entity игрока в хранилище игрока
 */
void server_client_player_info_store(server_player_t * player)
{
    if(player->entity->info->client_store)
        player->userstoredata = player->entity->info->client_store(
                player->entity->edata
        );

    entity_t * entity = player->entity;
    size_t vars_num = entity->info->vars_num;
    entityvarinfo_t * vars = entity->info->vars;

    size_t i;
    for(i = 0; i < vars_num; i++)
    {
        var_t * var = var_create(&player->vars, vars[i].name, vars[i].type);
        vardata_t * clientvardata = (vardata_t*)var->data;
        vardata_t * entityvardata = entity_vardata_get(entity, vars[i].name, -1);
        strncpy(clientvardata->name, vars[i].name, VARNAME_SIZE);
        clientvardata->type  = entityvardata->type;
        clientvardata->value = entityvardata->value;
    }

}

server_player_t * server_client_player_get_by_id(const server_client_t * client, int playerId)
{
    int players_num = server_client_players_num_get(client);
    if(playerId < 0 || playerId >= players_num)
        return NULL;
    server_player_t * player = client->players;
    playerId = players_num - 1 - playerId;
    int i;
    LIST2_LIST_TO_IENT(client->players, player, i, playerId);
    return player;
}

server_client_t * server_client_get(int id)
{
    int clients_num = server_client_num_get();
    if(id < 0 || id >= clients_num)
        return NULL;
    server_client_t * client = server.clients;
    id = clients_num - 1 - id;
    int i;
    LIST2_LIST_TO_IENT(server.clients, client, i, id);
    return client;
}

/**
 * @brif убирание клиентов из игры (не дисконект!), сохранение информации о клиентах
 */
void server_unjoin_clients(void)
{
    server_client_t * client;
    LIST2_FOREACHR(server.clients, client)
    {
        server_player_t * player;
        LIST2_FOREACHR(client->players, player)
        {
            server_client_player_info_store(player);
            player->entity = NULL;
        }
    }
}


int server_gamesave_load(int isave)
{
    /* игра уже создана */
    gamesave_load_context_t ctx;
    if(g_gamesave_load_open(isave, &ctx))
        return -1;

    map_clear();

    //прочитаем карту
    if(map_load(ctx.mapfilename))
    {
        game_console_send("Error: Could not load map \"%s\".", ctx.mapfilename);
        //game_abort();

        g_gamesave_load_close(&ctx);
        return -1;
    }

    server.flags.localgame = ctx.flag_localgame;
    server.flags.allow_respawn = ctx.flag_allow_respawn;

    g_gamesave_load_close(&ctx);

    server.gamestate.allow_state_gamesave = false;
/* TODO:
    foreach_loaded_client()
    {
    server.loaded_client[i] = ctx.loadedclient[i];

    exiting_client_get(i).settings = server.loaded_client[i].settings;
//       server_client_players_num_set(client, players_num);

    }
    */
    return 0;
}

server_client_t * server_client_create(int sock, const net_addr_t * net_addr, bool main)
{
    server_client_t * client = Z_malloc(sizeof(server_client_t));
    if(!client)
        game_halt("server_client_create(): Can not alloc memory, failed");

    client->joined = false;
    client->main = main;
    client->ns.sock = sock;
    client->ns.addr_ = *net_addr;
    client->players_num = GAME_CLIENT_PLAYERSNUM_ASSIGN_CLIENT;
    client->players = NULL;
    client->tx_queue_num = 0;
    LIST2_PUSH(server.clients, client);
    return client;
}




static const entityaction_t * server_entity_action_find_(
    const char * action_str,
    const entityaction_t *actions,
    size_t actions_num)
{

    for(size_t i = 0; i < actions_num; i++)
    {
        const entityaction_t * action = &actions[i];
        if(ACTIONS_EQ(action->action, action_str))
        {
            return action;
        }
    }
    return NULL;
}


const entityaction_t * server_entity_action_find(const entity_t * ent, const char * action_str)
{
    const entityinfo_t * info = ent->info;
    return server_entity_action_find_(action_str, info->actions, info->actions_num);
}

void server_clients_delete(void)
{
    server_client_t * client;
    while(!LIST2_IS_EMPTY(server.clients))
    {
        client = server.clients;
        LIST2_UNLINK(server.clients, client);
        server_client_delete(client);
    }
}


server_player_t * server_player_create()
{
    server_player_t * player = Z_malloc(sizeof(server_player_t));
    if(!player)
        game_halt("server_player_create(): Can not alloc memory, failed");
    player->entity = NULL;
    player->userstoredata = NULL;
    return player;
}

void server_player_delete(server_player_t * player)
{
    Z_free(player->userstoredata);
    Z_free(player);
}



/**
 * @brief восстановление информации о entity игрока из хранилища игрока в entity
 * @brief (при переходе на следующий уровень и при чтении gamesave)
 */
void server_client_player_info_restore(server_player_t * player)
{
    if(player->userstoredata)
    {
        player->entity->info->client_restore(
            player->entity,
            player->entity->edata,
            player->userstoredata
        );
        Z_free(player->userstoredata);
        player->userstoredata = NULL;
    }

    if(player->vars)
    {
        entity_t * entity = player->entity;

        void var_restore(vardata_t * clientvardata, void * args)
        {
            vardata_t * entityvardata = entity_vardata_get(entity, clientvardata->name, -1);
            if(!entityvardata)
                game_console_send("Error: Can not restore client entity info, entity has no variable \"%s\".", clientvardata->name);
            else
            {
                if(entityvardata->type != clientvardata->type)
                {
                    game_console_send("Error: Can not restore client entity info, variable \"%s\" has different types.", clientvardata->name);
                }
                else
                {
                    strncpy(entityvardata->name, clientvardata->name, VARNAME_SIZE);
                    entityvardata->type  = clientvardata->type;
                    entityvardata->value = clientvardata->value;
                }
            }
        }

        vars_dump(player->vars, "==== Client vars:");

        vars_foreach(player->vars, var_restore, NULL);
        vars_dump(player->entity->vars, "==== Entity vars:");

        vars_delete(&player->vars);
    }

}
