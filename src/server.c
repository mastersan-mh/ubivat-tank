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

#include "world.h"

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

    /*
     * https://www.linux.org.ru/forum/development/10072313
     */

    int err;
    size_t buf_len;
    net_addr_t sender;

    while(net_recv(server.sock, buf, &buf_len, PDU_BUF_SIZE, &sender) == 0)
    {
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
            break;
        case SERVER_STATE_INIT:

            server.sv_port = NET_SERVER_RECIEVER_PORT;
            server.sv_addr = INADDR_ANY;
            net_addr_t net_addr;
            net_addr_set(&net_addr, server.sv_port, server.sv_addr);

            server.sock = net_socket_open_connectionless(&net_addr);
            if(server.sock < 0)
            {
                game_halt("server socket() failed");
            }

            server.gamestate = SERVER_GAMESTATE_1_NOGAME;
            server.gstate.paused = false;
            server.state = SERVER_STATE_RUN;
            server.storages = NULL;
            break;
        case SERVER_STATE_RUN :
            server_net_io();
            server_events_handle();
            break;
        case SERVER_STATE_DONE:
            // удалить оставшихся игроков
            server_clients_delete();
            net_socket_close(server.sock);
            world_destroy();
            server.gamestate = SERVER_GAMESTATE_1_NOGAME;
            server.state = SERVER_STATE_IDLE;
            server_storages_free();
            break;
    }
}
