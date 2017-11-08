/*
 * server_private.c
 *
 *  Created on: 3 нояб. 2017 г.
 *      Author: mastersan
 */

#include "common/common_list2.h"

#include "net_pdu.h"
#include "server_private.h"
#include "server_reply.h"

#include "client_requests.h"

#include "g_gamesave.h"
#include "vars.h"

#include <stdlib.h>
#include <assert.h>

const char * server_gamestate_to_str(server_gamestate_t state)
{
    static const char *list[] =
    {
            "SERVER_GAMESTATE_1_NOGAME",
            "SERVER_GAMESTATE_2_INGAME",
            "SERVER_GAMESTATE_3_INTERMISSION",
    };
    return list[state];
}

void server_storages_free()
{
    server_player_vars_storage_t * storage;
    while(!LIST2_IS_EMPTY(server.storages))
    {
        storage = server.storages;
        LIST2_UNLINK(server.storages, storage);
        vars_delete(&storage->vars);
        Z_free(storage);
    }
}

server_player_vars_storage_t * server_storage_find(size_t clientId, size_t playerId)
{
    server_player_vars_storage_t * storage;
    LIST2_FOREACH(server.storages, storage)
    {
        if(
                storage->clientId == clientId &&
                storage->playerId == playerId)
            return storage;
    }
    return server_storage_create(clientId, playerId);
}

server_player_vars_storage_t * server_storage_create(size_t clientId, size_t playerId)
{
    server_player_vars_storage_t * storage = Z_malloc(sizeof(server_player_vars_storage_t));
    storage->vars = NULL;
    storage->clientId = clientId;
    storage->playerId = playerId;
    LIST2_PUSH(server.storages, storage);
    return storage;
}

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

void server_client_delete(server_client_t * client)
{
    size_t playerId = 0;
    server_player_t * player;
    while(!LIST2_IS_EMPTY(client->players))
    {
        player = client->players;
        LIST2_UNLINK(client->players, player);
        server_player_delete(player);
        playerId++;
    }
    Z_free(client);
}

int server_client_spawn(server_client_t * client, int players_num)
{
    client->players_num = players_num;
    int playerId = players_num - 1;
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
        size_t clientId = server_client_id_get(client);
        server_player_vars_storage_t * storage = server_storage_find(clientId, playerId);
        server_player_info_restore(player, storage);
        playerId--;
    }
    client->joined = true;
    return 0;
}

/**
 * @brif убирание клиентов из игры (не дисконект!), сохранение информации о клиентах
 */
void server_clients_unspawn(void)
{
    server_client_t * client;
    size_t clientId = 0;
    LIST2_FOREACHR(server.clients, client)
    {
        server_player_t * player;
        client->joined = false;
        size_t playerId = client->players_num - 1;
        while(!LIST2_IS_EMPTY(client->players))
        {
            player = client->players;
            LIST2_UNLINK(client->players, player);

            server_player_vars_storage_t * storage = server_storage_find(clientId, playerId);
            server_player_info_store(storage, player);
            player->entity = NULL;
            server_player_delete(player);

            playerId--;
        }
        clientId++;
    }
}

int server_clients_num_get(void)
{
    server_client_t * client;
    int num;
    LIST2_FOREACH_I(server.clients, client, num);
    return num;
}

size_t server_client_id_get(server_client_t * client)
{
    server_client_t * cl;
    ssize_t id = 0;
    LIST2_FOREACH(server.clients, cl)
    {
        if(cl == client)
            return id;
        id++;
    }
    assert(0 && "server_client_id_get(): client not found.");
    return -1;
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
vardata_t * server_storage_vardata_get(server_player_vars_storage_t * storage, const char * varname, vartype_t vartype)
{
    static const char * list[] =
    {
            "INTEGER",
            "FLOAT",
            "STRING",
    };
    var_t * var = var_find(storage->vars, varname);
    if(!var)
    {
        var = var_create(&storage->vars, varname, vartype);
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
void server_player_info_store(server_player_vars_storage_t * storage, server_player_t * player)
{
    if(player->entity->info->player_store)
        player->userstoredata = player->entity->info->player_store(
                player->entity->edata
        );

    entity_t * entity = player->entity;
    size_t vars_num = entity->info->vars_num;
    entityvarinfo_t * vars = entity->info->vars;

    size_t i;
    for(i = 0; i < vars_num; i++)
    {
        var_t * var;
        var = var_find(storage->vars, vars[i].name);
        if(!var)
            var = var_create(&storage->vars, vars[i].name, vars[i].type);
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
    int clients_num = server_clients_num_get();
    if(id < 0 || id >= clients_num)
        return NULL;
    server_client_t * client = server.clients;
    id = clients_num - 1 - id;
    int i;
    LIST2_LIST_TO_IENT(server.clients, client, i, id);
    return client;
}

int server_gamesave_load(int isave)
{
    /* игра уже создана */
    gamesave_load_context_t ctx;
    if(g_gamesave_load_open(isave, &ctx))
        return -1;

    int res = g_gamesave_load_read_header(&ctx);
    if(res)
    {
        game_console_send("server: Error: Could not load gamesave.");
        return -1;
    }

    map_clear();

    //прочитаем карту
    if(map_load(ctx.mapfilename))
    {
        game_console_send("server: Error: Could not load map \"%s\".", ctx.mapfilename);
        //game_abort();

        g_gamesave_load_close(&ctx);
        return -1;
    }

    for(size_t clientId = 0; clientId < ctx.clients_num; clientId++)
    {
        size_t players_num = ctx.clients_descr[clientId];
        for(size_t playerId = 0; playerId < players_num; playerId++)
        {
            server_player_vars_storage_t * storage = server_storage_find(clientId, playerId);
            g_gamesave_load_player(&ctx, storage);
        }
    }

    server.flags.localgame = ctx.flag_localgame;
    server.flags.allow_respawn = ctx.flag_allow_respawn;

    g_gamesave_load_close(&ctx);

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
void server_player_info_restore(server_player_t * player, server_player_vars_storage_t * storage)
{
    if(player->userstoredata)
    {
        player->entity->info->player_restore(
            player->entity,
            player->entity->edata,
            player->userstoredata
        );
        Z_free(player->userstoredata);
        player->userstoredata = NULL;
    }

    if(storage->vars)
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

        vars_dump(storage->vars, "==== vars in storage:");

        vars_foreach(storage->vars, var_restore, NULL);
        vars_dump(player->entity->vars, "==== Entity vars:");
    }

}


int server_pdu_parse(const net_addr_t * sender, const char * buf, size_t buf_len)
{
    size_t sv_req_queue_num;
    game_server_event_t event;
    event.sender = *sender;

    size_t ofs = 0;
    int16_t value16;

    PDU_POP_BUF(&value16, sizeof(value16));
    sv_req_queue_num = ntohs(value16);
    for(size_t i = 0; i < sv_req_queue_num; i++)
    {
        PDU_POP_BUF(&value16, sizeof(value16));
        game_client_request_type_t type = htons(value16);
        switch(type)
        {
            /** Непривилегированые запросы */
            case G_CLIENT_REQ_DISCOVERYSERVER:
                event.type = G_SERVER_EVENT_REMOTE_DISCOVERYSERVER;
                break;
            case G_CLIENT_REQ_CONNECT:
                event.type = G_SERVER_EVENT_REMOTE_CLIENT_CONNECT;
                break;
            case G_CLIENT_REQ_DISCONNECT:
                event.type = G_SERVER_EVENT_REMOTE_CLIENT_DISCONNECT;
                break;
            case G_CLIENT_REQ_SPAWN:
                event.type = G_SERVER_EVENT_REMOTE_CLIENT_SPAWN;
                PDU_POP_BUF(&value16, sizeof(value16));
                event.data.REMOTE_JOIN.players_num = ntohs(value16);
                break;
            case G_CLIENT_REQ_PLAYER_ACTION:
                event.type = G_SERVER_EVENT_REMOTE_CLIENT_PLAYER_ACTION;
                PDU_POP_BUF(&value16, sizeof(value16));
                event.data.REMOTE_PLAYER_ACTION.playerId = ntohs(value16);
                PDU_POP_BUF(event.data.REMOTE_PLAYER_ACTION.action, GAME_ACTION_SIZE);
                break;
                /** Привилегированные запросы */
            case G_CLIENT_REQ_GAME_ABORT:
                event.type = G_SERVER_EVENT_REMOTE_GAME_ABORT;
                break;
            case G_CLIENT_REQ_GAME_SETMAP:
                event.type = G_SERVER_EVENT_REMOTE_GAME_SETMAP;
                PDU_POP_BUF(event.data.REMOTE_GAME_SETMAP.mapname, MAP_FILENAME_SIZE);
                break;
            case G_CLIENT_REQ_READY:
                event.type = G_SERVER_EVENT_REMOTE_CLIENT_READY;
                break;
            case G_CLIENT_REQ_GAME_SAVE:
                event.type = G_SERVER_EVENT_REMOTE_GAME_SAVE;
                PDU_POP_BUF(&value16, sizeof(value16));
                event.data.REMOTE_GAME_SAVE.isave = ntohs(value16);
                break;
            case G_CLIENT_REQ_GAME_LOAD:
                event.type = G_SERVER_EVENT_REMOTE_GAME_LOAD;
                PDU_POP_BUF(&value16, sizeof(value16));
                event.data.REMOTE_GAME_LOAD.isave = ntohs(value16);
                break;
        }

        server_event_send(&event);
    }
    return 0;
}

int server_pdu_build(server_client_t * client, char * buf, size_t * buf_len, size_t buf_size)
{
    int16_t value16;
    size_t i;
    size_t ofs = 0;

    value16 = htons(client->tx_queue_num);
    PDU_PUSH_BUF(&value16, sizeof(value16));

    for(i = 0; i < client->tx_queue_num; i++)
    {
        game_server_reply_t *event = &client->tx_queue[i].req;
        value16 = htons(event->type);
        PDU_PUSH_BUF(&value16, sizeof(value16));
        switch(event->type)
        {
            case G_SERVER_REPLY_INFO:
                /*
            nvalue32 = htonl( (uint32_t) event->info.clients_num );
            memcpy(&buf[buflen], &nvalue32, sizeof(nvalue32));
            buflen += sizeof(nvalue32);
                 */
                break;
            case G_SERVER_REPLY_CONNECTION_ACCEPTED:
                break;
            case G_SERVER_REPLY_CONNECTION_CLOSE:
                break;
            case G_SERVER_REPLY_PLAYERS_ENTITY_SET:
            {
                int player_num = event->data.PLAYERS_ENTITY_SET.players_num;
                value16 = htons(player_num);
                PDU_PUSH_BUF(&value16, sizeof(value16));
                for(int i = 0; i < player_num; i++)
                {
                    PDU_PUSH_BUF(event->data.PLAYERS_ENTITY_SET.ent[i].entityname, GAME_SERVER_EVENT_ENTNAME_SIZE);
                    PDU_PUSH_BUF(&event->data.PLAYERS_ENTITY_SET.ent[i].entity, sizeof(event->data.PLAYERS_ENTITY_SET.ent[i].entity));
                }
                break;
            }
            case G_SERVER_REPLY_GAME_ENDMAP:
                value16 = htons(event->data.GAME_ENDMAP.win ? -1 : 0);
                PDU_PUSH_BUF(&value16, sizeof(value16));
                value16 = htons(event->data.GAME_ENDMAP.endgame ? -1 : 0);
                PDU_PUSH_BUF(&value16, sizeof(value16));
                break;
        }
    }
    client->tx_queue_num = 0;
    return 0;
}



void server_tx(void)
{
    static char buf[PDU_BUF_SIZE];
    int err;
    size_t buf_len;

    server_client_t * client;
    LIST2_FOREACH(server.clients, client)
    {
        err = server_pdu_build(client, buf, &buf_len, PDU_BUF_SIZE);
        if(err)
        {
            game_console_send("SERVER: client TX buffer overflow");
            return;
        }
        if(buf_len > 0)
        {
            net_send(&client->ns, buf, buf_len);
        }

    }
}
