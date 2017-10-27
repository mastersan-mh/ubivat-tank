/*
 * client.c
 *
 *  Created on: 8 дек. 2016 г.
 *      Author: mastersan
 */

#include "common/common_list2.h"
#include "net_pdu.h"
#include "net.h"
#include "client.h"
#include "cl_input.h"
#include "cl_game.h"
#include "g_events.h"
#include "game.h"
#include "Z_mem.h"
#include "menu.h"
#include "ui.h"
#include "video.h"
#include "map.h"

#include <string.h>

client_state_t cl_state = {};

client_client_t client = {};

void cl_game_init(void)
{
    cl_state.quit = false;

    cl_state.imenu = MENU_MAIN;

    cl_state.msg       = NULL;
    cl_state.state     = GAMESTATE_NOGAME;
    cl_state.show_menu = true;
    cl_state.custommap = mapList;
    cl_state.gamemap   = mapList;

}

void cl_done(void)
{

}

int client_player_num_get(void)
{
    client_player_t * player;
    size_t clients_num;
    LIST2_FOREACH_I(client.players, player, clients_num);
    return clients_num;
}

client_player_t * client_player_get(int playerId)
{
    int num, i;
    num = client_player_num_get();
    if(playerId < 0 || playerId >= num)
        return NULL;
    client_player_t * player;
    LIST2_LIST_TO_IENT(client.players, player, i, num - 1 - playerId);
    return player;
}

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

void client_req_join_send(void)
{
    game_client_request_t req;
    req.type = G_CLIENT_REQ_JOIN;
    req.data.JOIN.players_num = (cl_state.flags & GAMEFLAG_2PLAYERS) ? 2 : 1;
    client_req_send(&req);
}

void client_player_action_send(int playerId, const char * action_name)
{
    client_player_t * player = client_player_get(playerId);
    if(!player)
    {
        /* клавиша игрока, которого нет */
        return;
    }
    if(player->events_num >= CLIENT_EVENTS_MAX)
    {
        game_console_send("CLIENT: player %d events overflow.", playerId);
        return;
    }
    strncpy(player->events[player->events_num].CONTROL.action, action_name, GAME_CLIENT_PLAYER_REQUEST_CONTROL_ACTION_SIZE);
    player->events_num++;
}

void client_req_gameabort_send(void)
{
    game_client_request_t req;
    req.type = G_CLIENT_REQ_GAME_ABORT;
    client_req_send(&req);
}

void client_req_nextgamestate_send(void)
{
    game_client_request_t req;
    req.type = G_CLIENT_REQ_GAME_NEXTSTATE;
    client_req_send(&req);
}

void client_req_gamesave_save_send(int isave)
{
    game_client_request_t req;
    req.type = G_CLIENT_REQ_GAME_SAVE;
    req.data.GAME_SAVE.isave = isave;
    client_req_send(&req);
}

void client_req_gamesave_load_send(int isave)
{
    game_client_request_t req;
    req.type = G_CLIENT_REQ_GAME_LOAD;
    req.data.GAME_LOAD.isave = isave;
    client_req_send(&req);
}

void client_req_setgamemap_send(const char * mapname)
{
    game_client_request_t req;
    req.type = G_CLIENT_REQ_GAME_SETMAP;
    strncpy(req.data.GAME_SETMAP.mapname, mapname, MAP_FILENAME_SIZE);
    client_req_send(&req);
}

static void client_player_delete(client_player_t * player)
{
    Z_free(player);
}

/**
 * подключение игрока к игре
 * @return error
 */
int client_connect(void)
{
    client_player_t * player;
    player = Z_malloc(sizeof(client_player_t));
    LIST2_PUSH(client.players, player);
    if(cl_state.flags & GAMEFLAG_2PLAYERS)
    {
        player = Z_malloc(sizeof(client_player_t));
        LIST2_PUSH(client.players, player);
    }

    client.state = CLIENT_AWAITING_CONNECTION;
    client.time = time_current;

    game_client_request_t req;
    req.type = G_CLIENT_REQ_CONNECT;
    client_req_send(&req);

    return 0;
}

/*
 * инициализация камер клиентов
 */
void client_initcams(void)
{
    client_player_t * player;
    int players_num = client_player_num_get();

    float cam_sx = (float)VIDEO_SCREEN_W * (float)VIDEO_SCALEX / (float)VIDEO_SCALE;
    float cam_sy = (float)VIDEO_SCREEN_H * (float)VIDEO_SCALEY / (float)VIDEO_SCALE;

    cam_sx /= players_num;

    float statusbar_h = 32.0f * (float)VIDEO_SCALEY / (float)VIDEO_SCALE;

    float border = 1.0;
    float x = border;
    LIST2_FOREACH(client.players, player)
    {
        VEC2_CLEAR(player->cam.origin);

        player->cam.x     = x;
        player->cam.y     = 0;
        player->cam.sx    = cam_sx - (border * 2.0);
        player->cam.sy    = cam_sy - statusbar_h;//184
        x = x + cam_sx;
    }

}

static void client_disconnect(void)
{
    game_client_request_t req;
    req.type = G_CLIENT_REQ_DISCONNECT;
    client_req_send(&req);

    client_player_t * player;
    while(!LIST2_IS_EMPTY(client.players))
    {
        player = client.players;
        LIST2_UNLINK(client.players, player);
        client_player_delete(player);
    }
}

static const char * gamestate_to_str(gamestate_t state)
{
    static const char *list[] =
    {
            "GAMESTATE_NOGAME",
            "GAMESTATE_MISSION_BRIEF",
            "GAMESTATE_GAMESAVE",
            "GAMESTATE_INGAME",
            "GAMESTATE_INTERMISSION",
    };
    return list[state];
}

void client_start(int flags)
{
    cl_state.flags = flags;
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
}


static void client_fsm(const game_server_event_t * event)
{
    switch(client.state)
    {
    case CLIENT_AWAITING_CONNECTION:
        /*
        if(!buf)
        {
            if(time_current - client.time > CLIENT_TIMEOUT)
            {
                game_console_send("client: server reply timeout.");
            }
            break;
        }
         */

        client.time = time_current;
        if(event->type == G_SERVER_EVENT_CONNECTION_ACCEPTED)
        {
            //game_console_send("client: server accept connection at 0x%00000000x:%d.", client.ns->addr_.addr_in.sin_addr, ntohs(client.ns->addr_.addr_in.sin_port));
            game_console_send("client: server accept connection.");
            client.state = CLIENT_LISTEN;
        }
        else
        {
            game_console_send("client: invalid server event.");
        }

        break;
    case CLIENT_LISTEN:
        client.time = time_current;

        switch(event->type)
        {
        case G_SERVER_EVENT_INFO:
            break;
        case G_SERVER_EVENT_CONNECTION_ACCEPTED:
            break;
        case G_SERVER_EVENT_CONNECTION_CLOSE:
            client_disconnect();
            game_console_send("client: host closed the connection.");
            break;
        case G_SERVER_EVENT_GAME_STATE_SET:
            cl_state.state = event->data.GAME_STATE_SET.state;
            game_console_send("client: host change gamestate to %s.", gamestate_to_str(cl_state.state));

            if(cl_state.state == GAMESTATE_INTERMISSION)
            {
                game_console_send("client: server say: PLAYER WIN!");
                cl_state.win = true;
            }

            break;
        case G_SERVER_EVENT_PLAYER_ENTITY_SET:
        {
            int player_num = event->data.PLAYER_ENTITY_SET.player2 ? 2 : 1;

            for(int i = 0; i < player_num; i++)
            {
                entity_t * clientent = event->data.PLAYER_ENTITY_SET.ent[i].entity;
                bool local_client = true;
                if(local_client)
                {
                    client_player_get(i)->entity = clientent;
                }
            }
            break;
        }
        case G_SERVER_EVENT_GAME_LOADED:
            cl_state.flags = event->data.GAME_LOADED.flags;
            client_connect();
            client_initcams();
            break;
        }

        break;

    }
}


static int client_pdu_parse(const char * buf, size_t buf_len)
{
    int16_t value16;
    size_t i;
    size_t ofs = 0;

    PDU_POP_BUF(&value16, sizeof(value16));
    size_t events_num = ntohs(value16);

    for(i = 0; i < events_num; i++)
    {
        PDU_POP_BUF(&value16, sizeof(value16));
        game_server_event_t reply;
        reply.type = ntohs(value16);
        switch(reply.type)
        {
        case G_SERVER_EVENT_INFO:
            /*
            nvalue32 = htonl( (uint32_t) event->info.clients_num );
            memcpy(&buf[buflen], &nvalue32, sizeof(nvalue32));
            buflen += sizeof(nvalue32);
             */
            break;
        case G_SERVER_EVENT_CONNECTION_ACCEPTED:
        case G_SERVER_EVENT_CONNECTION_CLOSE:
            break;
        case G_SERVER_EVENT_GAME_STATE_SET:
            PDU_POP_BUF(&value16, sizeof(value16));
            reply.data.GAME_STATE_SET.state = ntohs(value16);
            break;
        case G_SERVER_EVENT_GAME_LOADED:
            PDU_POP_BUF(&value16, sizeof(value16));
            reply.data.GAME_LOADED.flags = ntohs(value16);
            break;
        case G_SERVER_EVENT_PLAYER_ENTITY_SET:

            PDU_POP_BUF(&value16, sizeof(value16));
            reply.data.PLAYER_ENTITY_SET.player2 = ntohs(value16);

            int player_num = reply.data.PLAYER_ENTITY_SET.player2 ? 2 : 1;
            for(int i = 0; i < player_num; i++)
            {
                PDU_POP_BUF(reply.data.PLAYER_ENTITY_SET.ent[i].entityname, GAME_SERVER_EVENT_ENTNAME_SIZE);
                PDU_POP_BUF(&reply.data.PLAYER_ENTITY_SET.ent[i].entity, sizeof(reply.data.PLAYER_ENTITY_SET.ent[i].entity));
            }

            break;
        }


        client_fsm(&reply);
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
            /** Привилегированные запросы */
        case G_CLIENT_REQ_GAME_ABORT:
            break;
        case G_CLIENT_REQ_GAME_SETMAP:
            PDU_PUSH_BUF(req->data.GAME_SETMAP.mapname, MAP_FILENAME_SIZE);
            break;
        case G_CLIENT_REQ_GAME_NEXTSTATE:
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

    /* players */
    client_player_t *player;
    LIST2_FOREACHR(client.players, player)
    {
        if(player->events_num == 0)
        {
            value16 = htons(G_CLIENT_PLAYER_REQ_NONE);
            PDU_PUSH_BUF(&value16, sizeof(value16));
        }
        else
        {
            value16 = htons(G_CLIENT_PLAYER_REQ_CONTROL);
            PDU_PUSH_BUF(&value16, sizeof(value16));
            value16 = htons(player->events_num);
            PDU_PUSH_BUF(&value16, sizeof(value16));
            for(size_t i = 0; i < player->events_num; i++)
            {
                PDU_PUSH_BUF(player->events[i].CONTROL.action, GAME_CLIENT_PLAYER_REQUEST_CONTROL_ACTION_SIZE);
            }
            player->events_num = 0;
        }
    }
    return 0;
}


/*
 * слушать хост, получать от него сообщения для клиентов
 */
static void client_listen(void)
{
    static char buf[PDU_BUF_SIZE];
    int err;

    if(!client.ns)
        return;

    struct sockaddr addr;
    socklen_t addr_len = 0;
    size_t buf_len;
    while(net_recv(client.ns, buf, &buf_len, PDU_BUF_SIZE, &addr, &addr_len) == 0)
    {
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

    if(LIST2_IS_EMPTY(client.players))
        cl_state.state = GAMESTATE_NOGAME;
}

static void client_events_pump()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        //printf("event.type = %d\n", event.type);
        switch(event.type)
        {
        case SDL_KEYDOWN:
            input_key_setState(event.key.keysym.scancode, true);
            break;
        case SDL_KEYUP:
            input_key_setState(event.key.keysym.scancode, false);
            break;
        }
        //player_checkcode();
    }
}

extern int server_run;

void client_handle(void)
{
    if(!server_run)
    {
        cl_state.state = GAMESTATE_NOGAME;
    }

    client_listen();

    if(cl_state.state == GAMESTATE_GAMESAVE)
    {
        cl_state.show_menu = true;
        cl_state.imenu     = MENU_GAME_SAVE;
    }

    if(cl_state.show_menu)
    {
        menu_events_pump();
    }
    else
    {
        client_events_pump();
    }

    if(cl_state.show_menu)
    {
        cl_state.paused = true;
        menu_selector_t imenu_process = cl_state.imenu;
        cl_state.imenu = menu_handle(imenu_process);
        menu_draw(imenu_process);
    }
    else
    {
        cl_state.paused = false;
        cl_game_draw();
    }

}

