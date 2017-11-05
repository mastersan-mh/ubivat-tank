/*
 * client.c
 *
 *  Created on: 8 дек. 2016 г.
 *      Author: mastersan
 */

#include "common/common_list2.h"
#include "sound.h"

#include "net_pdu.h"
#include "net.h"

#include "client.h"
#include "cl_input.h"
#include "cl_game.h"
#include "client_fsm.h"
#include "client_private.h"

#include "server_reply.h"

#include "g_events.h"
#include "game.h"
#include "Z_mem.h"
#include "menu.h"
#include "ui.h"
#include "video.h"
#include "map.h"

#include <string.h>

client_t client = {};

bool client_running(void)
{
    return client.state != CLIENT_STATE_IDLE;
}

client_gamestate_t client_gamestate_get(void)
{
    return client.gamestate;
}

void client_init(void)
{
    client.state            = CLIENT_STATE_IDLE;
    client.gstate.msg       = NULL;
    game_action_showmenu(NULL);
}

void client_done()
{

}

void client_start(int flags)
{
    client.gstate.players_num = (flags & GAMEFLAG_2PLAYERS) ? 2 : 1;
    client.state = CLIENT_STATE_INIT;
}

void client_stop(void)
{
    client.state = CLIENT_STATE_DONE;
}


static int client_pdu_parse(const char * buf, size_t buf_len)
{
    game_client_event_t event;
    int16_t value16;
    size_t i;
    size_t ofs = 0;

    PDU_POP_BUF(&value16, sizeof(value16));
    size_t events_num = ntohs(value16);

    for(i = 0; i < events_num; i++)
    {
        PDU_POP_BUF(&value16, sizeof(value16));
        game_server_reply_type_t type = ntohs(value16);
        switch(type)
        {
            case G_SERVER_REPLY_INFO:
                event.type = G_CLIENT_EVENT_REMOTE_INFO;
                /*
            nvalue32 = htonl( (uint32_t) event.info.clients_num );
            memcpy(&buf[buflen], &nvalue32, sizeof(nvalue32));
            buflen += sizeof(nvalue32);
                 */
                break;
            case G_SERVER_REPLY_CONNECTION_ACCEPTED:
                event.type = G_CLIENT_EVENT_REMOTE_CONNECTION_ACCEPTED;
                break;
            case G_SERVER_REPLY_CONNECTION_CLOSE:
                event.type = G_CLIENT_EVENT_REMOTE_CONNECTION_CLOSE;
                break;
            case G_SERVER_REPLY_PLAYERS_JOIN_AWAITING:
                PDU_POP_BUF(&value16, sizeof(value16));
                event.type = G_CLIENT_EVENT_REMOTE_PLAYERS_JOIN_AWAITING;
                event.data.REMOTE_PLAYERS_JOIN_AWAITING.players_num = ntohs(value16);
                break;
            case G_SERVER_REPLY_PLAYERS_ENTITY_SET:
            {
                event.type = G_CLIENT_EVENT_REMOTE_PLAYERS_ENTITY_SET;
                int player_num = client.gstate.players_num;
                for(int i = 0; i < player_num; i++)
                {
                    PDU_POP_BUF(event.data.REMOTE_PLAYERS_ENTITY_SET.ent[i].entityname, GAME_SERVER_EVENT_ENTNAME_SIZE);
                    PDU_POP_BUF(&event.data.REMOTE_PLAYERS_ENTITY_SET.ent[i].entity, sizeof(event.data.REMOTE_PLAYERS_ENTITY_SET.ent[i].entity));
                }
                break;
            }
        }
        client_fsm(&event);
    }

    return 0;
}

/**
 * @brief Build client PDU
 */
static int client_pdu_build(char * buf, size_t * buf_len, size_t buf_size)
{
    int16_t value16;
    size_t ofs = 0;

    /* client requests */
    value16 = htons(client.req_queue_num);
    PDU_PUSH_BUF(&value16, sizeof(value16));
    for(size_t i = 0; i < client.req_queue_num; i++)
    {
        game_client_request_t * req = &client.req_queue[i].req;
        value16 = htons(req->type);
        PDU_PUSH_BUF(&value16, sizeof(value16));
        switch(req->type)
        {
            /** Непривилегированые запросы */
            case G_CLIENT_REQ_DISCOVERYSERVER:
                break;
            case G_CLIENT_REQ_CONNECT:
                break;
            case G_CLIENT_REQ_DISCONNECT:
                break;
            case G_CLIENT_REQ_JOIN:
                value16 = ntohs(req->data.JOIN.players_num);
                PDU_PUSH_BUF(&value16, sizeof(value16));
                break;
            case G_CLIENT_REQ_PLAYER_ACTION:
                value16 = ntohs(req->data.PLAYER_ACTION.playerId);
                PDU_PUSH_BUF(&value16, sizeof(value16));
                PDU_PUSH_BUF(req->data.PLAYER_ACTION.action, GAME_ACTION_SIZE);
                break;
                /** Привилегированные запросы */
            case G_CLIENT_REQ_GAME_ABORT:
                break;
            case G_CLIENT_REQ_GAME_SETMAP:
                PDU_PUSH_BUF(req->data.GAME_SETMAP.mapname, MAP_FILENAME_SIZE);
                break;
            case G_CLIENT_REQ_READY:
                break;
            case G_CLIENT_REQ_GAME_SAVE:
                value16 = htons(req->data.GAME_SAVE.isave);
                PDU_PUSH_BUF(&value16, sizeof(value16));
                break;
            case G_CLIENT_REQ_GAME_LOAD:
                value16 = htons(req->data.GAME_LOAD.isave);
                PDU_PUSH_BUF(&value16, sizeof(value16));
                break;
        }
    }
    client.req_queue_num = 0;
    client.game_ready_sended = false;

    return 0;
}

/*
 * слушать хост, получать от него сообщения для клиентов
 */
static void client_net_io(void)
{
    static char buf[PDU_BUF_SIZE];
    int err;

    if(!client.ns)
        return;

    struct sockaddr addr;
    socklen_t addr_len = 0;
    size_t buf_len;

    if(time_current - client.time > CLIENT_TIMEOUT)
    {
        game_console_send("client: server reply timeout.");
    }

    while(net_recv(client.ns, buf, &buf_len, PDU_BUF_SIZE, &addr, &addr_len) == 0)
    {
        client.time = time_current;
        err = client_pdu_parse(buf, buf_len);
        if(err)
        {
            game_console_send("CLIENT: TX PDU parse error.");
        }
    }

    err = client_pdu_build(buf, &buf_len, PDU_BUF_SIZE);
    if(err)
    {
        game_console_send("CLIENT: TX buffer overflow.");
        return;
    }
    if(buf_len > 0)
    {
        net_send(client.ns, buf, buf_len);
    }
}


void client_handle(void)
{
    switch(client.state)
    {
        case CLIENT_STATE_IDLE:
            client.gamestate = CLIENT_GAMESTATE_1_NOGAME;
            client.gamestate_prev = CLIENT_GAMESTATE_6_INTERMISSION;
            break;
        case CLIENT_STATE_INIT:
            client.ns = net_socket_create(NET_PORT, "127.0.0.1");

            if(client.ns == NULL)
            {
                game_halt("client socket() failed");
            }
            /*
        if(net_socket_bind(client_ns) < 0)
        {
            game_halt("client bind() failed");
        }
             */
            client.state = CLIENT_STATE_RUN;
            break;
        case CLIENT_STATE_RUN :
            client_net_io();
            break;
        case CLIENT_STATE_DONE:
            client_req_send_game_abort();
            client_players_delete();
            client.state = CLIENT_STATE_IDLE;
            client.gamestate = CLIENT_GAMESTATE_1_NOGAME;
            break;
    }
}

