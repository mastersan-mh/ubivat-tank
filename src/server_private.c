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
    };
    return list[state];
}


void server_gamesave_clients_info_allocate(size_t clients_num)
{
    if(server.gamesave_clients_info != NULL)
        Z_free(server.gamesave_clients_info);
    server.gamesave_clients_info_num = clients_num;
    size_t size = sizeof(server_gamesave_client_info_t) * clients_num;
    server.gamesave_clients_info = Z_malloc(size);
    memset(server.gamesave_clients_info, 0, size);
}

void server_gamesave_clients_info_clean(void)
{
    Z_free(server.gamesave_clients_info);
    server.gamesave_clients_info_num = 0;
}

static inline void server_gamesave_client_info_set(size_t clientId, size_t players_num)
{
    server.gamesave_clients_info[clientId].players_num = players_num;
}

size_t server_gamesave_client_info_get(size_t clientId)
{
    if(clientId >= server.gamesave_clients_info_num)
        return 0;
    return server.gamesave_clients_info[clientId].players_num;
}

/**
 * @brief отметить инфлормацию как использованную
 */
void server_gamesave_client_info_mark(size_t clientId)
{
    if(clientId >= server.gamesave_clients_info_num)
        return;
    server_gamesave_client_info_t * info = &server.gamesave_clients_info[clientId];
    if(info->players_num > 0)
        info->players_num--;
}



void server_storages_free()
{
    server_player_vars_storage_t * storage;
    while(!LIST2_IS_EMPTY(server.storages))
    {
        storage = server.storages;
        LIST2_UNLINK(server.storages, storage);
        storage->info = NULL;
        Z_free(storage->vars);
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
        if( !memcmp(&client->net_addr.addr, &addr->addr, sizeof(struct sockaddr)) )
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
    int player_spawned = 0;
    for(int i = 0; i < players_num; i++)
    {
        server_player_t * player = server_player_create();
        LIST2_PUSH(client->players, player);

        size_t clientId = server_client_id_get(client);
        server_player_vars_storage_t * storage = server_storage_find(clientId, playerId);

        entity_t * entity = entity_player_spawn_random(storage ? storage->vars : NULL);
        if(!entity)
        {
            game_console_send("Error: No entity to spawn client.");
            return -1;
        }
        player->entity = entity;
        playerId--;
        player_spawned++;
    }
    client->joined = (player_spawned != 0);
    return client->joined ? 0 : -1;
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

/**
 * получить id клиента
 */
size_t server_client_id_get(const server_client_t * client)
{
    int clients_num = server_clients_num_get();
    server_client_t * cl;
    LIST2_FOREACH(server.clients, cl)
    {
        clients_num--;
        if(cl == client)
            return clients_num;
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
 * @brief сохранение информации о entity игрока в хранилище игрока
 */
void server_player_info_store(server_player_vars_storage_t * storage, server_player_t * player)
{
    entity_t * entity = player->entity;
    size_t info_vars_num = entity->info->vars_descr_num;
    const var_descr_t * info_vars = entity->info->vars_descr;
    if(!storage->vars)
        storage->vars = Z_malloc(entity->info->vars_size);
    storage->info = player->entity->info;
    char * vars = storage->vars;
    for(size_t i = 0; i < info_vars_num; i++)
    {
        intptr_t ofs = info_vars[i].ofs;
        memcpy(vars + ofs, entity->vars + ofs, info_vars[i].size);
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

        g_gamesave_load_close(&ctx);
        return -1;
    }

    server_gamesave_clients_info_allocate(ctx.clients_num);

    for(size_t clientId = 0; clientId < ctx.clients_num; clientId++)
    {
        size_t players_num = ctx.clients_descr[clientId];
        for(size_t playerId = 0; playerId < players_num; playerId++)
        {
            server_player_vars_storage_t * storage = server_storage_find(clientId, playerId);
            g_gamesave_load_player(&ctx, storage);
            server_gamesave_client_info_set(clientId, players_num);
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
    client->net_addr = *net_addr;
    client->players_num = GAME_CLIENT_PLAYERSNUM_ASSIGN_CLIENT;
    client->players = NULL;
    client->tx_queue_num = 0;
    LIST2_PUSH(server.clients, client);
    return client;
}




static const entity_action_t * server_entity_action_find_(
    const char * action_str,
    const entity_action_t *actions,
    size_t actions_num)
{

    for(size_t i = 0; i < actions_num; i++)
    {
        const entity_action_t * action = &actions[i];
        if(ACTIONS_EQ(action->action, action_str))
        {
            return action;
        }
    }
    return NULL;
}


const entity_action_t * server_entity_action_find(const entity_t * ent, const char * action_str)
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
    return player;
}

void server_player_delete(server_player_t * player)
{
    Z_free(player);
}

int server_pdu_parse(const net_addr_t * sender, const char * buf, size_t buf_len)
{
    size_t sv_req_queue_num;
    server_event_type_t evtype;
    server_event_data_t evdata;

    size_t ofs = 0;
    int16_t value16;

    PDU_POP_BUF(&value16, sizeof(value16));
    sv_req_queue_num = ntohs(value16);
    for(size_t i = 0; i < sv_req_queue_num; i++)
    {
        PDU_POP_BUF(&value16, sizeof(value16));
        client_request_type_t rtype = htons(value16);
        switch(rtype)
        {
            /** Непривилегированые запросы */
            case G_CLIENT_REQ_DISCOVERYSERVER:
                evtype = G_SERVER_EVENT_REMOTE_DISCOVERYSERVER;
                break;
            case G_CLIENT_REQ_CONNECT:
                evtype = G_SERVER_EVENT_REMOTE_CLIENT_CONNECT;
                break;
            case G_CLIENT_REQ_DISCONNECT:
                evtype = G_SERVER_EVENT_REMOTE_CLIENT_DISCONNECT;
                break;
            case G_CLIENT_REQ_SPAWN:
                evtype = G_SERVER_EVENT_REMOTE_CLIENT_SPAWN;
                PDU_POP_BUF(&value16, sizeof(value16));
                evdata.REMOTE_JOIN.players_num = ntohs(value16);
                break;
            case G_CLIENT_REQ_PLAYER_ACTION:
                evtype = G_SERVER_EVENT_REMOTE_CLIENT_PLAYER_ACTION;
                PDU_POP_BUF(&value16, sizeof(value16));
                evdata.REMOTE_PLAYER_ACTION.playerId = ntohs(value16);
                PDU_POP_BUF(evdata.REMOTE_PLAYER_ACTION.action, GAME_ACTION_SIZE);
                break;
                /** Привилегированные запросы */
            case G_CLIENT_REQ_GAME_ABORT:
                evtype = G_SERVER_EVENT_REMOTE_GAME_ABORT;
                break;
            case G_CLIENT_REQ_GAME_SETMAP:
                evtype = G_SERVER_EVENT_REMOTE_GAME_SETMAP;
                PDU_POP_BUF(evdata.REMOTE_GAME_SETMAP.mapname, MAP_FILENAME_SIZE);
                break;
            case G_CLIENT_REQ_READY:
                evtype = G_SERVER_EVENT_REMOTE_CLIENT_READY;
                break;
            case G_CLIENT_REQ_GAME_SAVE:
                evtype = G_SERVER_EVENT_REMOTE_GAME_SAVE;
                PDU_POP_BUF(&value16, sizeof(value16));
                evdata.REMOTE_GAME_SAVE.isave = ntohs(value16);
                break;
            case G_CLIENT_REQ_GAME_LOAD:
                evtype = G_SERVER_EVENT_REMOTE_GAME_LOAD;
                PDU_POP_BUF(&value16, sizeof(value16));
                evdata.REMOTE_GAME_LOAD.isave = ntohs(value16);
                break;
        }

        server_event_send(sender, evtype, &evdata);
    }
    return 0;
}

int server_pdu_client_build(server_client_t * client, char * buf, size_t * buf_len, size_t buf_size)
{
    int16_t value16;
    uint32_t valueu32;
    size_t i;
    size_t ofs = 0;

    if(client->tx_queue_num == 0)
    {
        *buf_len = 0;
        return 0;
    }
    value16 = htons(client->tx_queue_num);
    PDU_PUSH_BUF(&value16, sizeof(value16));

    for(i = 0; i < client->tx_queue_num; i++)
    {
        server_reply_t * reply = &client->tx_queue[i].reply;
        value16 = htons(reply->type);
        PDU_PUSH_BUF(&value16, sizeof(value16));
        switch(reply->type)
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
                int player_num = reply->data.PLAYERS_ENTITY_SET.players_num;
                value16 = htons(player_num);
                PDU_PUSH_BUF(&value16, sizeof(value16));
                for(int i = 0; i < player_num; i++)
                {
                    PDU_PUSH_BUF(reply->data.PLAYERS_ENTITY_SET.ent[i].entityname, GAME_SERVER_EVENT_ENTNAME_SIZE);
                    uint32_t entityId = reply->data.PLAYERS_ENTITY_SET.ent[i].entityId;
                    valueu32 = htonl(entityId);
                    PDU_PUSH_BUF(&valueu32, sizeof(valueu32));
                }
                break;
            }
            case G_SERVER_REPLY_GAME_ENDMAP:
                value16 = htons(reply->data.GAME_ENDMAP.win ? -1 : 0);
                PDU_PUSH_BUF(&value16, sizeof(value16));
                value16 = htons(reply->data.GAME_ENDMAP.endgame ? -1 : 0);
                PDU_PUSH_BUF(&value16, sizeof(value16));
                break;
        }
    }
    client->tx_queue_num = 0;
    return 0;
}

int server_pdu_build(const server_reply_t * reply, char * buf, size_t * buf_len, size_t buf_size)
{
    int16_t value16;
    uint32_t valueu32;

    size_t ofs = 0;

    int tx_queue_num = 1;
    value16 = htons(tx_queue_num);
    PDU_PUSH_BUF(&value16, sizeof(value16));

    value16 = htons(reply->type);
    PDU_PUSH_BUF(&value16, sizeof(value16));
    switch(reply->type)
    {
        case G_SERVER_REPLY_INFO:
            value16 = htons(reply->data.INFO.clients_num);
            PDU_PUSH_BUF(&value16, sizeof(value16));
            break;
        case G_SERVER_REPLY_CONNECTION_ACCEPTED:
            break;
        case G_SERVER_REPLY_CONNECTION_CLOSE:
            break;
        case G_SERVER_REPLY_PLAYERS_ENTITY_SET:
        {
            int player_num = reply->data.PLAYERS_ENTITY_SET.players_num;
            value16 = htons(player_num);
            PDU_PUSH_BUF(&value16, sizeof(value16));
            for(int i = 0; i < player_num; i++)
            {
                PDU_PUSH_BUF(reply->data.PLAYERS_ENTITY_SET.ent[i].entityname, GAME_SERVER_EVENT_ENTNAME_SIZE);
                uint32_t entityId = reply->data.PLAYERS_ENTITY_SET.ent[i].entityId;
                valueu32 = htonl(entityId);
                PDU_PUSH_BUF(&valueu32, sizeof(valueu32));
            }
            break;
        }
        case G_SERVER_REPLY_GAME_ENDMAP:
            value16 = htons(reply->data.GAME_ENDMAP.win ? -1 : 0);
            PDU_PUSH_BUF(&value16, sizeof(value16));
            value16 = htons(reply->data.GAME_ENDMAP.endgame ? -1 : 0);
            PDU_PUSH_BUF(&value16, sizeof(value16));
            break;
    }

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
        err = server_pdu_client_build(client, buf, &buf_len, PDU_BUF_SIZE);
        if(err)
        {
            game_console_send("SERVER: client TX buffer overflow");
            return;
        }
        if(buf_len > 0)
        {
            net_send(server.sock, buf, buf_len, &client->net_addr);
        }

    }


    while(!CIRCLEQ_EMPTY(&server.txs))
    {
        server_tx_t * tx = CIRCLEQ_FIRST(&server.txs);
        CIRCLEQ_REMOVE(&server.txs, tx, queue);

        server_pdu_build(&tx->reply, buf, &buf_len, PDU_BUF_SIZE);
        if(err)
        {
            game_console_send("SERVER: TX buffer overflow");
            return;
        }
        if(buf_len > 0)
        {
            net_send(server.sock, buf, buf_len, &tx->net_addr);
        }
        Z_free(tx);
    }


}
