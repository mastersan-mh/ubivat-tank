/*
 * client_request.c
 *
 *  Created on: 1 нояб. 2017 г.
 *      Author: mastersan
 */

#include "client_private.h"
#include "client_requests.h"
#include <assert.h>

void client_req_send(const game_client_request_t * req)
{
    if(client.req_queue_num >= CLIENT_REQ_QUEUE_SIZE)
    {
        game_console_send("CLIENT: REQ queue overflow.");
        return;
    }
    client.req_queue[client.req_queue_num].req = *req;
    client.req_queue_num++;
}

extern void client_req_send_discoveryserver(void)
{
    assert(0 && "client_req_send_discoveryserver(): unreleased function");
}

/**
 * @brief подключение игрока к игре
 */
void client_req_send_connect(void)
{
    game_client_request_t req;
    req.type = G_CLIENT_REQ_CONNECT;
    client_req_send(&req);
}
void client_req_send_disconnect(void)
{
    game_client_request_t req;
    req.type = G_CLIENT_REQ_DISCONNECT;
    client_req_send(&req);
}

void client_req_send_join(void)
{
    game_client_request_t req;
    req.type = G_CLIENT_REQ_JOIN;
    req.data.JOIN.players_num = client.gstate.players_num;
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
    game_client_request_t req;
    req.type = G_CLIENT_REQ_PLAYER_ACTION;
    req.data.PLAYER_ACTION.playerId = playerId;
    memcpy(req.data.PLAYER_ACTION.action, action_name, GAME_ACTION_SIZE);
    client_req_send(&req);
}

void client_req_send_game_abort(void)
{
    game_client_request_t req;
    req.type = G_CLIENT_REQ_GAME_ABORT;
    client_req_send(&req);
}

void client_req_send_game_setmap(const char * mapname)
{
    game_client_request_t req;
    req.type = G_CLIENT_REQ_GAME_SETMAP;
    strncpy(req.data.GAME_SETMAP.mapname, mapname, MAP_FILENAME_SIZE);
    client_req_send(&req);
}

void client_req_send_game_nextstate(void)
{
    if(client.game_next_state_sended) return;
    game_client_request_t req;
    req.type = G_CLIENT_REQ_GAME_NEXTSTATE;
    client_req_send(&req);
    client.game_next_state_sended = true;
}

void client_req_send_game_save(int isave)
{
    game_client_request_t req;
    req.type = G_CLIENT_REQ_GAME_SAVE;
    req.data.GAME_SAVE.isave = isave;
    client_req_send(&req);
}

void client_req_send_game_load(int isave)
{
    game_client_request_t req;
    req.type = G_CLIENT_REQ_GAME_LOAD;
    req.data.GAME_LOAD.isave = isave;
    client_req_send(&req);
}

