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

bool sv_entity_valid = false;

void server_init(void)
{
    CIRCLEQ_INIT(&server.events);
    CIRCLEQ_INIT(&server.txs);
    server.gstate.custommap = mapList;
    server.gstate.gamemap   = mapList;

}

void server_done(void)
{

}

void server_start(int flags)
{
    if(server.state != SERVER_STATE_IDLE)
        return;
    server.state = SERVER_STATE_INIT;
    server.flags.localgame = !(flags & GAMEFLAG_CUSTOMGAME);
    server.flags.allow_respawn = (flags & GAMEFLAG_2PLAYERS) || (flags & GAMEFLAG_CUSTOMGAME);
}


void server_stop(void)
{
    server.state = SERVER_STATE_DONE;
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

    server_tx();
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
            sv_entity_valid = false;
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
            server.state = SERVER_STATE_RUN;
            server.storages = NULL;
            break;
        case SERVER_STATE_RUN :
            server_net_io();
            server_events_handle();
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
