/*
 * client_request.c
 *
 *  Created on: 1 нояб. 2017 г.
 *      Author: mastersan
 */

#include "client_private.h"
#include "client_requests.h"
#include <assert.h>

static void client_req_send(const client_request_t * req)
{
    if(client.tx_queue_num >= CLIENT_REQ_QUEUE_SIZE)
    {
        game_console_send("CLIENT: REQ queue overflow.");
        return;
    }
    client.tx_queue[client.tx_queue_num].req = *req;
    client.tx_queue_num++;
}

extern void client_req_send_discoveryserver(void)
{
    client_request_t req;
    req.type = G_CLIENT_REQ_DISCOVERYSERVER;
    client_req_send(&req);
}

/**
 * @brief подключение игрока к игре
 */
void client_req_send_connect(void)
{
    client_request_t req;
    req.type = G_CLIENT_REQ_CONNECT;
    client_req_send(&req);
}
void client_req_send_disconnect(void)
{
    client_request_t req;
    req.type = G_CLIENT_REQ_DISCONNECT;
    client_req_send(&req);
}

void client_req_send_spawn(void)
{
    client_request_t req;
    req.type = G_CLIENT_REQ_SPAWN;
    req.data.SPAWN.players_num = client.gstate.players_num;
    client_req_send(&req);
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
    client_req_send(&req);
}

void client_req_send_game_abort(void)
{
    client_request_t req;
    req.type = G_CLIENT_REQ_GAME_ABORT;
    client_req_send(&req);
}

void client_req_send_game_setmap(const char * mapname)
{
    client_request_t req;
    req.type = G_CLIENT_REQ_GAME_SETMAP;
    strncpy(req.data.GAME_SETMAP.mapname, mapname, MAP_FILENAME_SIZE);
    client_req_send(&req);
}

void client_req_send_ready(void)
{
    client_request_t req;
    req.type = G_CLIENT_REQ_READY;
    client_req_send(&req);
}

void client_req_send_game_save(int isave)
{
    client_request_t req;
    req.type = G_CLIENT_REQ_GAME_SAVE;
    req.data.GAME_SAVE.isave = isave;
    client_req_send(&req);
}

void client_req_send_game_load(int isave)
{
    client_request_t req;
    req.type = G_CLIENT_REQ_GAME_LOAD;
    req.data.GAME_LOAD.isave = isave;
    client_req_send(&req);
}

