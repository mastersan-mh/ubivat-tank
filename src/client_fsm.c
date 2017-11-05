/*
 * client_fsm.c
 *
 *  Created on: 1 нояб. 2017 г.
 *      Author: mastersan
 */

#include "client_fsm.h"

#include "types.h"
#include "sound.h"
#include "game.h"

#include "client_private.h"
#include "cl_input.h"
#include "client_events.h"

#include "common/common_list2.h"

#define GAME_SOUND_MENU 10

#define INVALID_EVENT() game_console_send("client: invalid server gamestate %s event %d.", game_gamestate_to_str(gamestate), event->type)

#define FSM_CLIENT_DISCONECT() \
        do { \
            client_disconnect(); \
            game_console_send("client: server close the connection."); \
        } while (0);

#define FSM_GAMESTATE_SET(gs) \
        do { \
            gamestate = (gs); \
            game_console_send("client: gamestate changed to %s.", client_gamestate_to_str(gamestate)); \
        } while (0)

void client_fsm(const game_client_event_t * event)
{
    bool statechanged = false;
    if(client.gamestate_prev != client.gamestate)
    {
        client.gamestate_prev = client.gamestate;
        statechanged = true;
    }
    client_gamestate_t gamestate = client.gamestate;

    switch(client.gamestate)
    {

        case CLIENT_GAMESTATE_1_NOGAME:
        {
            switch(event->type)
            {
                case G_CLIENT_EVENT_LOCAL_KEY_PRESS:
                    break;
                case G_CLIENT_EVENT_LOCAL_KEY_RELEASE:
                    break;
                case G_CLIENT_EVENT_REMOTE_INFO:
                    break;
                case G_CLIENT_EVENT_REMOTE_CONNECTION_ACCEPTED:
                    //game_console_send("client: server accept connection at 0x%00000000x:%d.", client.ns->addr_.addr_in.sin_addr, ntohs(client.ns->addr_.addr_in.sin_port));
                    game_console_send("client: server accept connection.");
                    client_req_send_ready();
                    FSM_GAMESTATE_SET(CLIENT_GAMESTATE_2_MISSION_BRIEF);
                    break;
                case G_CLIENT_EVENT_REMOTE_CONNECTION_CLOSE:
                    break;
                    break;
                case G_CLIENT_EVENT_REMOTE_PLAYERS_JOIN_AWAITING:
                    break;
                case G_CLIENT_EVENT_REMOTE_PLAYERS_ENTITY_SET:
                    break;
            }
            break;
        }
        case CLIENT_GAMESTATE_2_MISSION_BRIEF:
        {
            if(statechanged)
            {
                if(sound_started(NULL, GAME_SOUND_MENU))
                    sound_play_stop(NULL, GAME_SOUND_MENU);
                sound_play_start(NULL, GAME_SOUND_MENU, SOUND_MUSIC1, -1);
            }
            switch(event->type)
            {
                case G_CLIENT_EVENT_LOCAL_KEY_PRESS:
                    sound_play_start(NULL, 0, SOUND_MENU_ENTER, 1);
                    client_req_send_ready();
                    FSM_GAMESTATE_SET(CLIENT_GAMESTATE_3_JOIN_AWAITING);
                    break;
                case G_CLIENT_EVENT_LOCAL_KEY_RELEASE:
                    break;
                case G_CLIENT_EVENT_REMOTE_INFO:
                    break;
                case G_CLIENT_EVENT_REMOTE_CONNECTION_ACCEPTED:
                    break;
                case G_CLIENT_EVENT_REMOTE_CONNECTION_CLOSE:
                    FSM_CLIENT_DISCONECT();
                    FSM_GAMESTATE_SET(CLIENT_GAMESTATE_1_NOGAME);
                    break;
                case G_CLIENT_EVENT_REMOTE_PLAYERS_JOIN_AWAITING:
                    break;
                case G_CLIENT_EVENT_REMOTE_PLAYERS_ENTITY_SET:
                    break;
            }
            break;
        }
        case CLIENT_GAMESTATE_3_JOIN_AWAITING:
        {
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
                    client_req_send_join();
                    client_req_send_ready();
                    FSM_GAMESTATE_SET(CLIENT_GAMESTATE_4_GAMESAVE);
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
        }
        case CLIENT_GAMESTATE_4_GAMESAVE:
        {
            game_menu_show(MENU_GAME_SAVE);
            switch(event->type)
            {
                case G_CLIENT_EVENT_LOCAL_KEY_PRESS:
                    client_req_send_ready();
                    FSM_GAMESTATE_SET(CLIENT_GAMESTATE_5_INGAME);
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
                case G_CLIENT_EVENT_REMOTE_PLAYERS_JOIN_AWAITING:
                    break;
                case G_CLIENT_EVENT_REMOTE_PLAYERS_ENTITY_SET:
                    break;
            }
            break;
        }
        case CLIENT_GAMESTATE_5_INGAME:
        {
            if(statechanged)
            {
                sound_play_stop(NULL, GAME_SOUND_MENU);
            }
            switch(event->type)
            {
                case G_CLIENT_EVENT_LOCAL_KEY_PRESS:
                    client_key_press(event->data.LOCAL_KEY_PRESS.key);
                    break;
                case G_CLIENT_EVENT_LOCAL_KEY_RELEASE:
                    client_key_release(event->data.LOCAL_KEY_RELEASE.key);
                    break;
                case G_CLIENT_EVENT_REMOTE_INFO:
                    break;
                case G_CLIENT_EVENT_REMOTE_CONNECTION_ACCEPTED:
                    break;
                case G_CLIENT_EVENT_REMOTE_CONNECTION_CLOSE:
                    FSM_CLIENT_DISCONECT();
                    break;
                case G_CLIENT_EVENT_REMOTE_PLAYERS_JOIN_AWAITING:
                    break;
                case G_CLIENT_EVENT_REMOTE_PLAYERS_ENTITY_SET:
                    break;
            }
            break;
        }
        case CLIENT_GAMESTATE_6_INTERMISSION:
        {
            if(statechanged)
            {
                if(sound_started(NULL, GAME_SOUND_MENU))
                    sound_play_stop(NULL, GAME_SOUND_MENU);
                sound_play_start(NULL, GAME_SOUND_MENU, SOUND_MUSIC1, -1);
                game_console_send("client: server say: PLAYER WIN!");
                client.gstate.win = true;
            }
            switch(event->type)
            {
                case G_CLIENT_EVENT_LOCAL_KEY_PRESS:
                    client_req_send_ready();
                    sound_play_start(NULL, 0, SOUND_MENU_ENTER, 1);
                    FSM_GAMESTATE_SET(CLIENT_GAMESTATE_1_NOGAME);
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
                case G_CLIENT_EVENT_REMOTE_PLAYERS_JOIN_AWAITING:
                    break;
                case G_CLIENT_EVENT_REMOTE_PLAYERS_ENTITY_SET:
                    break;
            }
            break;
        }
    }

    client.gamestate = gamestate;

}
