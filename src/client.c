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
void client_init(void)
{
    client.state            = CLIENT_STATE_IDLE;
    client.gstate.msg       = NULL;
    client.gstate.custommap = mapList;
    client.gstate.gamemap   = mapList;
    game_action_showmenu();
}

void client_done()
{

}

int client_player_num_get(void)
{
    client_player_t * player;
    size_t clients_num = 0;
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

void client_req_send_players_join(void)
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
    memcpy(req.data.PLAYER_ACTION.action, action_name, GAME_CLIENT_REQ_PLAYER_ACTION_SIZE);
    client_req_send(&req);
}

void client_req_gameabort_send(void)
{
    game_client_request_t req;
    req.type = G_CLIENT_REQ_GAME_ABORT;
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

void client_req_send_gamesave_save(int isave)
{
    game_client_request_t req;
    req.type = G_CLIENT_REQ_GAME_SAVE;
    req.data.GAME_SAVE.isave = isave;
    client_req_send(&req);
}

void client_req_send_gamesave_load(int isave)
{
    game_client_request_t req;
    req.type = G_CLIENT_REQ_GAME_LOAD;
    req.data.GAME_LOAD.isave = isave;
    client_req_send(&req);
}

void client_req_send_setgamemap(const char * mapname)
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
    int players_num = client.gstate.players_num;

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
}


static void client_players_delete(void)
{
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
            "GAMESTATE_1_NOGAME",
            "GAMESTATE_2_MISSION_BRIEF",
            "GAMESTATE_3_JOIN_AWAITING",
            "GAMESTATE_4_GAMESAVE",
            "GAMESTATE_5_INGAME",
            "GAMESTATE_6_INTERMISSION",
    };
    return list[state];
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

void client_fsm(const game_client_event_t * event)
{

#define INVALID_EVENT() game_console_send("client: invalid server gamestate %s event %d.", gamestate_to_str(gamestate), event->type)

#define FSM_CLIENT_DISCONECT() \
        do { \
            client_disconnect(); \
            game_console_send("client: server close the connection."); \
        } while (0);

#define FSM_GAMESTATE_SET() \
        do { \
            gamestate = event->data.REMOTE_GAME_STATE_SET.state; \
            game_console_send("client: server change gamestate to %s.", gamestate_to_str(gamestate)); \
        } while (0);


    bool statechanged = false;
    if(client.gamestate_prev != client.gamestate)
    {
        client.gamestate_prev = client.gamestate;
        statechanged = true;
    }

    gamestate_t gamestate = client.gamestate;

    switch(client.gamestate)
    {

    case GAMESTATE_1_NOGAME:

        switch(event->type)
        {
        case G_CLIENT_EVENT_LOCAL_KEY_PRESS:
            key_handle_press(event->data.LOCAL_KEY_PRESS.key);
            break;
        case G_CLIENT_EVENT_LOCAL_KEY_RELEASE:
            key_handle_release(event->data.LOCAL_KEY_RELEASE.key);
            break;
        case G_CLIENT_EVENT_REMOTE_INFO:
            break;
        case G_CLIENT_EVENT_REMOTE_CONNECTION_ACCEPTED:
            //game_console_send("client: server accept connection at 0x%00000000x:%d.", client.ns->addr_.addr_in.sin_addr, ntohs(client.ns->addr_.addr_in.sin_port));
            game_console_send("client: server accept connection.");
            client_req_send_game_nextstate();
            break;
        case G_CLIENT_EVENT_REMOTE_CONNECTION_CLOSE:
            break;
        case G_CLIENT_EVENT_REMOTE_GAME_STATE_SET:
            FSM_GAMESTATE_SET();
            break;
        case G_CLIENT_EVENT_REMOTE_PLAYERS_JOIN_AWAITING:
            break;
        case G_CLIENT_EVENT_REMOTE_PLAYERS_ENTITY_SET:
            break;
        }
        break;
    case GAMESTATE_2_MISSION_BRIEF:
        if(statechanged)
        {
            if(sound_started(NULL, GAME_SOUND_MENU))
                sound_play_stop(NULL, GAME_SOUND_MENU);
            sound_play_start(NULL, GAME_SOUND_MENU, SOUND_MUSIC1, -1);
        }
        switch(event->type)
        {
            case G_CLIENT_EVENT_LOCAL_KEY_PRESS:
                break;
            case G_CLIENT_EVENT_LOCAL_KEY_RELEASE:
                sound_play_start(NULL, 0, SOUND_MENU_ENTER, 1);
                client_req_send_game_nextstate();
                break;
            case G_CLIENT_EVENT_REMOTE_INFO:
                break;
            case G_CLIENT_EVENT_REMOTE_CONNECTION_ACCEPTED:
                break;
            case G_CLIENT_EVENT_REMOTE_CONNECTION_CLOSE:
                FSM_CLIENT_DISCONECT();
                break;
            case G_CLIENT_EVENT_REMOTE_GAME_STATE_SET:
                FSM_GAMESTATE_SET();
                break;
            case G_CLIENT_EVENT_REMOTE_PLAYERS_JOIN_AWAITING:
                break;
            case G_CLIENT_EVENT_REMOTE_PLAYERS_ENTITY_SET:
                break;
        }
        break;
    case GAMESTATE_3_JOIN_AWAITING:
        switch(event->type)
        {
            case G_CLIENT_EVENT_LOCAL_KEY_PRESS:
                break;
            case G_CLIENT_EVENT_LOCAL_KEY_RELEASE:
                break;
            case G_CLIENT_EVENT_REMOTE_INFO:
                break;
            case G_CLIENT_EVENT_REMOTE_CONNECTION_ACCEPTED:
                break;
            case G_CLIENT_EVENT_REMOTE_CONNECTION_CLOSE:
                FSM_CLIENT_DISCONECT();
                break;
            case G_CLIENT_EVENT_REMOTE_GAME_STATE_SET:
                FSM_GAMESTATE_SET();
                break;
            case G_CLIENT_EVENT_REMOTE_PLAYERS_JOIN_AWAITING:
            {
                int players_num = event->data.REMOTE_PLAYERS_JOIN_AWAITING.players_num;
                if(players_num > GAME_CLIENT_PLAYERSNUM_ASSIGN_CLIENT)
                {
                    client.gstate.players_num = players_num;
                    game_console_send("CLIENT: server assign players amount equal %d", players_num);
                }
                else
                {
                    game_console_send("CLIENT: server not assign players amount, client set equal %d", client.gstate.players_num);
                }
                client_req_send_players_join();
                client_req_send_game_nextstate();

                break;
            }
            case G_CLIENT_EVENT_REMOTE_PLAYERS_ENTITY_SET:
            {
                int players_num = client.gstate.players_num;

                for(int i = 0; i < players_num; i++)
                {
                    client_player_t * player = Z_malloc(sizeof(client_player_t));
                    LIST2_PUSH(client.players, player);
                }

                client_initcams();

                for(int i = 0; i < players_num; i++)
                {
                    entity_t * clientent = event->data.REMOTE_PLAYERS_ENTITY_SET.ent[i].entity;
                    bool local_client = true;
                    if(local_client)
                    {
                        client_player_get(i)->entity = clientent;
                    }
                }
                break;
            }
        }
        break;
    case GAMESTATE_4_GAMESAVE:
        game_menu_show(MENU_GAME_SAVE);
        switch(event->type)
        {
            case G_CLIENT_EVENT_LOCAL_KEY_PRESS:
                break;
            case G_CLIENT_EVENT_LOCAL_KEY_RELEASE:
                client_req_send_game_nextstate();
                break;
            case G_CLIENT_EVENT_REMOTE_INFO:
                break;
            case G_CLIENT_EVENT_REMOTE_CONNECTION_ACCEPTED:
                break;
            case G_CLIENT_EVENT_REMOTE_CONNECTION_CLOSE:
                FSM_CLIENT_DISCONECT();
                break;
            case G_CLIENT_EVENT_REMOTE_GAME_STATE_SET:
                FSM_GAMESTATE_SET();
                break;
            case G_CLIENT_EVENT_REMOTE_PLAYERS_JOIN_AWAITING:
                break;
            case G_CLIENT_EVENT_REMOTE_PLAYERS_ENTITY_SET:
                break;
        }
        break;
    case GAMESTATE_5_INGAME:
        if(statechanged)
        {
            sound_play_stop(NULL, GAME_SOUND_MENU);
        }
        switch(event->type)
        {
            case G_CLIENT_EVENT_LOCAL_KEY_PRESS:
                break;
            case G_CLIENT_EVENT_LOCAL_KEY_RELEASE:
                break;
            case G_CLIENT_EVENT_REMOTE_INFO:
                break;
            case G_CLIENT_EVENT_REMOTE_CONNECTION_ACCEPTED:
                break;
            case G_CLIENT_EVENT_REMOTE_CONNECTION_CLOSE:
                FSM_CLIENT_DISCONECT();
                break;
            case G_CLIENT_EVENT_REMOTE_GAME_STATE_SET:
                FSM_GAMESTATE_SET();
                break;
            case G_CLIENT_EVENT_REMOTE_PLAYERS_JOIN_AWAITING:
                break;
            case G_CLIENT_EVENT_REMOTE_PLAYERS_ENTITY_SET:
                break;
        }
        break;
    case GAMESTATE_6_INTERMISSION:
        if(statechanged)
        {
            if(sound_started(NULL, GAME_SOUND_MENU))
                sound_play_stop(NULL, GAME_SOUND_MENU);
            sound_play_start(NULL, GAME_SOUND_MENU, SOUND_MUSIC1, -1);
        }
        switch(event->type)
        {
            case G_CLIENT_EVENT_LOCAL_KEY_PRESS:
                break;
            case G_CLIENT_EVENT_LOCAL_KEY_RELEASE:
                client_req_send_game_nextstate();
                sound_play_start(NULL, 0, SOUND_MENU_ENTER, 1);
                break;
            case G_CLIENT_EVENT_REMOTE_INFO:
                break;
            case G_CLIENT_EVENT_REMOTE_CONNECTION_ACCEPTED:
                break;
            case G_CLIENT_EVENT_REMOTE_CONNECTION_CLOSE:
                FSM_CLIENT_DISCONECT();
                break;
            case G_CLIENT_EVENT_REMOTE_GAME_STATE_SET:
                FSM_GAMESTATE_SET();
                game_console_send("client: server say: PLAYER WIN!");
                client.gstate.win = true;
                break;
            case G_CLIENT_EVENT_REMOTE_PLAYERS_JOIN_AWAITING:
                break;
            case G_CLIENT_EVENT_REMOTE_PLAYERS_ENTITY_SET:
                break;
        }
        break;
    }

    client.gamestate = gamestate;

}

/*
 * state = true | false = pressed | released
 */
void client_event_key_input(int key, bool state_pressed)
{
    game_client_event_t event;

    if(state_pressed)
    {
        event.type = G_CLIENT_EVENT_LOCAL_KEY_PRESS;
        event.data.LOCAL_KEY_PRESS.key = key;
    }
    else
    {
        event.type = G_CLIENT_EVENT_LOCAL_KEY_RELEASE;
        event.data.LOCAL_KEY_RELEASE.key = key;
    }

    client_fsm(&event);

}


static int client_pdu_parse(game_client_event_t * event, const char * buf, size_t buf_len)
{
    int16_t value16;
    size_t i;
    size_t ofs = 0;

    PDU_POP_BUF(&value16, sizeof(value16));
    size_t events_num = ntohs(value16);

    for(i = 0; i < events_num; i++)
    {
        PDU_POP_BUF(&value16, sizeof(value16));
        game_server_event_type_t type = ntohs(value16);
        switch(type)
        {
        case G_SERVER_EVENT_INFO:
            event->type = G_CLIENT_EVENT_REMOTE_INFO;
            /*
            nvalue32 = htonl( (uint32_t) event->info.clients_num );
            memcpy(&buf[buflen], &nvalue32, sizeof(nvalue32));
            buflen += sizeof(nvalue32);
             */
            break;
        case G_SERVER_EVENT_CONNECTION_ACCEPTED:
            event->type = G_CLIENT_EVENT_REMOTE_CONNECTION_ACCEPTED;
            break;
        case G_SERVER_EVENT_CONNECTION_CLOSE:
            event->type = G_CLIENT_EVENT_REMOTE_CONNECTION_CLOSE;
            break;
        case G_SERVER_EVENT_GAME_STATE_SET:
            PDU_POP_BUF(&value16, sizeof(value16));
            event->type = G_CLIENT_EVENT_REMOTE_GAME_STATE_SET;
            event->data.REMOTE_GAME_STATE_SET.state = ntohs(value16);
            break;
        case G_SERVER_EVENT_PLAYERS_JOIN_AWAITING:
            PDU_POP_BUF(&value16, sizeof(value16));
            event->type = G_CLIENT_EVENT_REMOTE_PLAYERS_JOIN_AWAITING;
            event->data.REMOTE_PLAYERS_JOIN_AWAITING.players_num = ntohs(value16);
            break;
        case G_SERVER_EVENT_PLAYERS_ENTITY_SET:
        {
            event->type = G_CLIENT_EVENT_REMOTE_PLAYERS_ENTITY_SET;
            int player_num = client.gstate.players_num;
             for(int i = 0; i < player_num; i++)
            {
                PDU_POP_BUF(event->data.REMOTE_PLAYERS_ENTITY_SET.ent[i].entityname, GAME_SERVER_EVENT_ENTNAME_SIZE);
                PDU_POP_BUF(&event->data.REMOTE_PLAYERS_ENTITY_SET.ent[i].entity, sizeof(event->data.REMOTE_PLAYERS_ENTITY_SET.ent[i].entity));
            }
            break;
        }
        }
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
            PDU_PUSH_BUF(req->data.PLAYER_ACTION.action, GAME_CLIENT_REQ_PLAYER_ACTION_SIZE);
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
    client.game_next_state_sended = false;

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
        game_client_event_t event;

        client.time = time_current;
        err = client_pdu_parse(&event, buf, buf_len);
        if(err)
        {
            game_console_send("CLIENT: TX PDU parse error.");
        }
        else
        {
            client_fsm(&event);
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

void client_events_pump(void)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        //printf("event.type = %d\n", event.type);
        switch(event.type)
        {
        case SDL_KEYDOWN:
            if(!event.key.repeat)
                client_event_key_input(event.key.keysym.scancode, true);
            break;
        case SDL_KEYUP:
            if(!event.key.repeat)
                client_event_key_input(event.key.keysym.scancode, false);
            break;
        }
        //player_checkcode();
    }
}

void client_handle(void)
{
    switch(client.state)
    {
    case CLIENT_STATE_IDLE:
        client.gamestate = GAMESTATE_1_NOGAME;
        client.gamestate_prev = GAMESTATE_6_INTERMISSION;
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
        client_req_gameabort_send();
        client_players_delete();
        client.state = CLIENT_STATE_IDLE;
        client.gamestate = GAMESTATE_1_NOGAME;
        break;
    }


}

