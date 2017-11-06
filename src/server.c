/*
 * server.c
 *
 *  Created on: 27 дек. 2016 г.
 *      Author: mastersan
 */

#include "common/common_list2.h"
#include "common/common_hash.h"

#include "net_pdu.h"
#include "net.h"

#include "menu.h"

#include "map.h"
#include "entity.h"
#include "vars.h"

#include "game.h"
#include "g_gamesave.h"
#include "g_events.h"

#include "server.h"
#include "sv_game.h"
#include "server_fsm.h"
#include "server_private.h"
#include "server_events.h"

#include "client_requests.h"


#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <netinet/in.h>

#include <errno.h>

#include <unistd.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

server_t server = {};

void server_init(void)
{
    server.gstate.custommap = mapList;
    server.gstate.gamemap   = mapList;
}

void server_done(void)
{

}

void server_start(int flags)
{
    server.state = SERVER_STATE_INIT;
    server.flags.localgame = !(flags & GAMEFLAG_CUSTOMGAME);
    server.flags.allow_respawn = (flags & GAMEFLAG_2PLAYERS) || (flags & GAMEFLAG_CUSTOMGAME);
}


void server_stop(void)
{
    server.state = SERVER_STATE_DONE;
}

static int server_pdu_parse(const net_addr_t * sender, const char * buf, size_t buf_len)
{
    size_t sv_req_queue_num;
    game_server_event_t event;
    event.sender = sender;

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
        server_fsm(&event);
    }
    return 0;
}

static int server_pdu_build(server_client_t * client, char * buf, size_t * buf_len, size_t buf_size)
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
        }
    }
    client->tx_queue_num = 0;
    return 0;
}


static void server_net_io(void)
{
    static char buf[PDU_BUF_SIZE];

    int err;

    net_addr_t sender;

    //struct sockaddr sender_addr;

    socklen_t sender_addr_len = sizeof(sender.addr);

    /*
     * https://www.linux.org.ru/forum/development/10072313
     */

    size_t buf_len;
    ssize_t size_;
    while( (size_ = recvfrom(server.ns->sock, buf, PDU_BUF_SIZE, 0, &sender.addr, &sender_addr_len)) > 0 )
    {
        buf_len = size_;

        err = server_pdu_parse(&sender, buf, buf_len);
        if(err)
        {
            game_console_send("CLIENT: TX PDU parse error.");
        }

    }

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

bool server_running(void)
{
    return server.state != SERVER_STATE_IDLE;
}

void server_handle()
{

    switch(server.state)
    {
    case SERVER_STATE_IDLE:
        server.gamestate = SERVER_GAMESTATE_1_NOGAME;
        server.gamestate_prev = SERVER_GAMESTATE_3_INTERMISSION;
        break;
    case SERVER_STATE_INIT:
        server.ns = net_socket_create(NET_PORT, "127.0.0.1");

        if(server.ns == NULL)
        {
            game_halt("socket() failed");
        }
        if(net_socket_bind(server.ns) < 0)
        {
            game_halt("server bind() failed");
        }

        server.gamestate = SERVER_GAMESTATE_1_NOGAME;
        server.gstate.paused = false;
        server.gstate.allow_state_gamesave = true;
        server.state = SERVER_STATE_RUN;
        server.storages = NULL;
        break;
    case SERVER_STATE_RUN :
        server_net_io();
        if(server.gamestate == SERVER_GAMESTATE_2_INGAME)
            sv_game_gameTick();
        break;
    case SERVER_STATE_DONE:
        // удалить оставшихся игроков
        server_clients_delete();
        net_socket_close(server.ns);
        server.ns = NULL;
        //закроем карту
        map_clear();
        server.gamestate = SERVER_GAMESTATE_1_NOGAME;
        server.state = SERVER_STATE_IDLE;
        server_storages_free();
        break;
    }
}
