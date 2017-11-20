/*
 * client_request.c
 *
 *  Created on: 1 нояб. 2017 г.
 *      Author: mastersan
 */

#include "client_private.h"
#include "client_requests.h"
#include <assert.h>

/**
 * @param dest_addr     NULL - broadcast
 */
static void client_req_send(const net_addr_t * dest_addr, const client_request_t * req)
{
    if(client.tx_queue_num >= CLIENT_REQ_QUEUE_SIZE)
    {
        game_console_send("CLIENT: REQ queue overflow.");
        return;
    }

    client_req_queue_t * tx = &client.tx_queue[client.tx_queue_num];

    if(dest_addr)
        tx->dest_addr = *dest_addr;
    else
    {
        net_addr_set(&tx->dest_addr, client.dest_port, INADDR_BROADCAST);
    }
    tx->req = *req;
    client.tx_queue_num++;
}

extern void client_req_send_discoveryserver(void)
{
    client_request_t req;
    req.type = G_CLIENT_REQ_DISCOVERYSERVER;
    client_req_send(NULL, &req);
}

/**
 * @brief подключение игрока к игре
 */
void client_req_send_connect(void)
{
    client_request_t req;
    req.type = G_CLIENT_REQ_CONNECT;
    net_addr_t net_addr;
    net_addr_set(&net_addr, client.dest_port, client.dest_addr);
    client_req_send(&net_addr, &req);
}
void client_req_send_disconnect(void)
{
    client_request_t req;
    req.type = G_CLIENT_REQ_DISCONNECT;
    net_addr_t net_addr;
    net_addr_set(&net_addr, client.dest_port, client.dest_addr);
    client_req_send(&net_addr, &req);
}

void client_req_send_spawn(int players_num)
{
    client_request_t req;
    req.type = G_CLIENT_REQ_SPAWN;
    req.data.SPAWN.players_num = players_num;
    net_addr_t net_addr;
    net_addr_set(&net_addr, client.dest_port, client.dest_addr);
    client_req_send(&net_addr, &req);
}

void client_req_send_player_action(int playerId, const char * action_name)
{
    client_player_t * player = client_player_get(playerId);
    if(!player)
    {
        /* клавиша игрока, которого нет */
        return;
    }
    client_request_t req;
    req.type = G_CLIENT_REQ_PLAYER_ACTION;
    req.data.PLAYER_ACTION.playerId = playerId;
    memcpy(req.data.PLAYER_ACTION.action, action_name, GAME_ACTION_SIZE);
    net_addr_t net_addr;
    net_addr_set(&net_addr, client.dest_port, client.dest_addr);
    client_req_send(&net_addr, &req);
}

void client_req_send_game_abort(void)
{
    client_request_t req;
    req.type = G_CLIENT_REQ_GAME_ABORT;
    net_addr_t net_addr;
    net_addr_set(&net_addr, client.dest_port, client.dest_addr);
    client_req_send(&net_addr, &req);
}

void client_req_send_game_setmap(const char * mapname)
{
    client_request_t req;
    req.type = G_CLIENT_REQ_GAME_SETMAP;
    strncpy(req.data.GAME_SETMAP.mapname, mapname, MAP_FILENAME_SIZE);
    net_addr_t net_addr;
    net_addr_set(&net_addr, client.dest_port, client.dest_addr);
    client_req_send(&net_addr, &req);
}

void client_req_send_ready(void)
{
    client_request_t req;
    req.type = G_CLIENT_REQ_READY;
    net_addr_t net_addr;
    net_addr_set(&net_addr, client.dest_port, client.dest_addr);
    client_req_send(&net_addr, &req);
}

void client_req_send_game_save(int isave)
{
    client_request_t req;
    req.type = G_CLIENT_REQ_GAME_SAVE;
    req.data.GAME_SAVE.isave = isave;
    net_addr_t net_addr;
    net_addr_set(&net_addr, client.dest_port, client.dest_addr);
    client_req_send(&net_addr, &req);
}

void client_req_send_game_load(int isave)
{
    client_request_t req;
    req.type = G_CLIENT_REQ_GAME_LOAD;
    req.data.GAME_LOAD.isave = isave;
    net_addr_t net_addr;
    net_addr_set(&net_addr, client.dest_port, client.dest_addr);
    client_req_send(&net_addr, &req);
}

