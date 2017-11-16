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

bool client_ingame(void)
{
    return
            !(client.gamestate == CLIENT_GAMESTATE_0_IDLE ||
                    client.gamestate == CLIENT_GAMESTATE_1_NOGAME);
}

void client_init(void)
{
    CIRCLEQ_INIT(&client.events);
    client.state      = CLIENT_STATE_IDLE;
    client.gamestate  = CLIENT_GAMESTATE_0_IDLE;
    client.gstate.msg = NULL;

    /* flush queues */
    client.tx_queue_num = 0;
}

void client_done(void)
{

}

void client_flags_set(int flags)
{
    client.gstate.players_num = (flags & GAMEFLAG_2PLAYERS) ? 2 : 1;
}

void client_start(void)
{
    if(client.state != CLIENT_STATE_IDLE)
        return;
    client.state = CLIENT_STATE_INIT;
}

void client_stop(void)
{
    client.state = CLIENT_STATE_DONE;
}

bool client_running(void)
{
    return client.state != CLIENT_STATE_IDLE;
}

static int client_pdu_parse(const net_addr_t * sender, const char * buf, size_t buf_len)
{
    client_event_type_t evtype;
    client_event_data_t evdata;
    int16_t value16;
    size_t i;
    size_t ofs = 0;

    PDU_POP_BUF(&value16, sizeof(value16));
    size_t events_num = ntohs(value16);

    for(i = 0; i < events_num; i++)
    {
        PDU_POP_BUF(&value16, sizeof(value16));
        server_reply_type_t rtype = ntohs(value16);
        switch(rtype)
        {
            case G_SERVER_REPLY_INFO:
                evtype = G_CLIENT_EVENT_REMOTE_INFO;
                PDU_POP_BUF(&value16, sizeof(value16));
                evdata.REMOTE_INFO.clients_num = ntohs(value16);
                break;
            case G_SERVER_REPLY_CONNECTION_ACCEPTED:
                evtype = G_CLIENT_EVENT_REMOTE_CONNECTION_ACCEPTED;
                break;
            case G_SERVER_REPLY_CONNECTION_CLOSE:
                evtype = G_CLIENT_EVENT_REMOTE_CONNECTION_CLOSE;
                break;
            case G_SERVER_REPLY_PLAYERS_ENTITY_SET:
            {
                evtype = G_CLIENT_EVENT_REMOTE_PLAYERS_ENTITY_SET;
                PDU_POP_BUF(&value16, sizeof(value16));
                evdata.REMOTE_PLAYERS_ENTITY_SET.players_num = ntohs(value16);
                for(int i = 0; i < evdata.REMOTE_PLAYERS_ENTITY_SET.players_num; i++)
                {
                    PDU_POP_BUF(evdata.REMOTE_PLAYERS_ENTITY_SET.ent[i].entityname, GAME_SERVER_EVENT_ENTNAME_SIZE);
                    PDU_POP_BUF(&evdata.REMOTE_PLAYERS_ENTITY_SET.ent[i].entity, sizeof(evdata.REMOTE_PLAYERS_ENTITY_SET.ent[i].entity));
                }
                break;
            }
            case G_SERVER_REPLY_GAME_ENDMAP:
                evtype = G_CLIENT_EVENT_REMOTE_GAME_ENDMAP;
                PDU_POP_BUF(&value16, sizeof(value16));
                evdata.REMOTE_GAME_ENDMAP.win = (ntohs(value16) != 0);
                PDU_POP_BUF(&value16, sizeof(value16));
                evdata.REMOTE_GAME_ENDMAP.endgame = (ntohs(value16) != 0);
                break;
        }

        client_event_send(sender, evtype, &evdata);
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
    value16 = htons(client.tx_queue_num);
    PDU_PUSH_BUF(&value16, sizeof(value16));
    for(size_t i = 0; i < client.tx_queue_num; i++)
    {
        client_request_t * req = &client.tx_queue[i].req;
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
            case G_CLIENT_REQ_SPAWN:
                value16 = ntohs(req->data.SPAWN.players_num);
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
    client.tx_queue_num = 0;

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

    net_addr_t net_addr;
    socklen_t addr_len = 0;
    size_t buf_len;

    if(time_current - client.time > CLIENT_TIMEOUT)
    {
        game_console_send("client: server reply timeout.");
    }

    while(net_recv(client.ns, buf, &buf_len, PDU_BUF_SIZE, &net_addr.addr, &addr_len) == 0)
    {
        client.time = time_current;
        err = client_pdu_parse(&net_addr, buf, buf_len);
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
            break;
        case CLIENT_STATE_INIT:
            client.gamestate = CLIENT_GAMESTATE_0_IDLE;
            client.state = CLIENT_STATE_RUN;
            break;
        case CLIENT_STATE_RUN :
            client_net_io();
            client_events_handle();
            break;
        case CLIENT_STATE_DONE:
            client_events_flush();
            client_clean();
            client.state = CLIENT_STATE_IDLE;
            client.gamestate = CLIENT_GAMESTATE_0_IDLE;
            break;
    }
}

