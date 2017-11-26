/*
 * client_fsm.c
 *
 *  Created on: 1 нояб. 2017 г.
 *      Author: mastersan
 */

#include "net.h"

#include "client.h"
#include "client_fsm.h"

#include "types.h"
#include "sound.h"
#include "game.h"

#include "client_private.h"
#include "cl_input.h"
#include "client_events.h"

#include "world.h"

#include "common/common_list2.h"

#include <assert.h>

#define GAME_SOUND_MENU 10

#define INVALID_EVENT() game_console_send("client: invalid server gamestate %s event %d.", game_gamestate_to_str(gamestate), event->type)

#define FSM_WARNING_ALREADY_CONNECTED() \
        game_console_send("You already connected to the game, disconnect first.");

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


static void client_fsm_local_connect(const client_event_t * event)
{
    if(!event->data.LOCAL_CONNECT.remotegame)
    {
        client.sv_dedicated = false;
        client.dest_addr = INADDR_LOOPBACK;
        client_req_send_connect();
        return;
    }

    client.sv_dedicated = true;

    const net_addr_t *net_addr = &event->data.LOCAL_CONNECT.net_addr;

    client.dest_port = ntohs(net_addr->addr_in.sin_port);
    client.dest_addr = ntohl(net_addr->addr_in.sin_addr.s_addr);

    game_console_send("client: connecting to " PRINTF_NETADDR_FMT "...", PRINTF_NETADDR_VAL(*net_addr));

    client_req_send_connect();
}

void client_fsm_remote_info(const client_event_t * event)
{
    game_server_add(&event->sender, event->data.REMOTE_INFO.clients_num);
}

void client_fsm_world_create(const client_event_t * event)
{
    if(!client.sv_dedicated)
        return;
    game_console_send("client: Recreating world...");
    world_destroy();
    if(world_create(event->data.REMOTE_WORLD_CREATE.mapfilename))
    {
        game_console_send("client: Error: Can not create world.");
        return;
    }
    game_console_send("client: World created.");
}



void client_fsm(const client_event_t * event)
{
    client_gamestate_t gamestate = client.gamestate;

    switch(client.gamestate)
    {
        case CLIENT_GAMESTATE_0_IDLE:
        {
            switch(event->type)
            {
                case G_CLIENT_EVENT_LOCAL_STOP:
                    client_stop();
                    /* already stopped */
                    break;
                case G_CLIENT_EVENT_LOCAL_KEY_PRESS:
                    break;
                case G_CLIENT_EVENT_LOCAL_KEY_RELEASE:
                    break;
                case G_CLIENT_EVENT_LOCAL_CONNECT:
                    client_fsm_local_connect(event);
                    FSM_GAMESTATE_SET(CLIENT_GAMESTATE_1_NOGAME);
                    break;
                case G_CLIENT_EVENT_LOCAL_DICOVERYSERVER:
                    client_req_send_discoveryserver();
                    break;
                case G_CLIENT_EVENT_LOCAL_NEWGAME:
                    break;
                case G_CLIENT_EVENT_LOCAL_LOADGAME:
                    break;
                case G_CLIENT_EVENT_LOCAL_ENTERGAME:
                    break;
                case G_CLIENT_EVENT_REMOTE_INFO:
                    client_fsm_remote_info(event);
                    break;
                case G_CLIENT_EVENT_REMOTE_CONNECTION_ACCEPTED:
                    break;
                case G_CLIENT_EVENT_REMOTE_CONNECTION_CLOSE:
                    break;
                case G_CLIENT_EVENT_REMOTE_WORLD_CREATE:
                    break;
                case G_CLIENT_EVENT_REMOTE_PLAYERS_ENTITY_SET:
                    break;
                case G_CLIENT_EVENT_REMOTE_GAME_ENDMAP:
                    break;
            }
            break;
        }
        case CLIENT_GAMESTATE_1_NOGAME:
        {
            switch(event->type)
            {
                case G_CLIENT_EVENT_LOCAL_STOP:
                    client_clean();
                    client_stop();
                    FSM_GAMESTATE_SET(CLIENT_GAMESTATE_0_IDLE);
                    break;
                case G_CLIENT_EVENT_LOCAL_KEY_PRESS:
                    break;
                case G_CLIENT_EVENT_LOCAL_KEY_RELEASE:
                    break;
                case G_CLIENT_EVENT_LOCAL_CONNECT:
                    FSM_WARNING_ALREADY_CONNECTED();
                    break;
                case G_CLIENT_EVENT_LOCAL_DICOVERYSERVER:
                    client_req_send_discoveryserver();
                    break;
                case G_CLIENT_EVENT_LOCAL_NEWGAME:
                    client_req_send_game_setmap(event->data.LOCAL_NEWGAME.mapname);
                    break;
                case G_CLIENT_EVENT_LOCAL_LOADGAME:
                    client_req_send_game_load(event->data.LOCAL_LOADGAME.isave);
                    break;
                case G_CLIENT_EVENT_LOCAL_ENTERGAME:
                    break;
                case G_CLIENT_EVENT_REMOTE_INFO:
                    client_fsm_remote_info(event);
                    break;
                case G_CLIENT_EVENT_REMOTE_CONNECTION_ACCEPTED:
                    game_console_send("client: server " PRINTF_NETADDR_IPv4_FMT " accept connection.", PRINTF_NETADDR_VAL(event->sender));
                    sound_play_stop(NULL, GAME_SOUND_MENU);
                    sound_play_start(NULL, GAME_SOUND_MENU, SOUND_MUSIC1, -1);
                    client_req_send_ready();
                    client.gstate.win = false;
                    client.gstate.endgame = false;
                    FSM_GAMESTATE_SET(CLIENT_GAMESTATE_2_WORLD_CREATING);
                    break;
                case G_CLIENT_EVENT_REMOTE_CONNECTION_CLOSE:
                    FSM_CLIENT_DISCONECT();
                    break;
                case G_CLIENT_EVENT_REMOTE_WORLD_CREATE:
                    break;
                case G_CLIENT_EVENT_REMOTE_PLAYERS_ENTITY_SET:
                    break;
                case G_CLIENT_EVENT_REMOTE_GAME_ENDMAP:
                    break;
            }
            break;
        }
        case CLIENT_GAMESTATE_2_WORLD_CREATING:
        {
            switch(event->type)
            {
                case G_CLIENT_EVENT_LOCAL_STOP:
                    client_clean();
                    client_stop();
                    FSM_GAMESTATE_SET(CLIENT_GAMESTATE_0_IDLE);
                    break;
                case G_CLIENT_EVENT_LOCAL_KEY_PRESS:
                    break;
                case G_CLIENT_EVENT_LOCAL_KEY_RELEASE:
                    break;
                case G_CLIENT_EVENT_LOCAL_CONNECT:
                    FSM_WARNING_ALREADY_CONNECTED();
                    break;
                case G_CLIENT_EVENT_LOCAL_DICOVERYSERVER:
                    break;
                case G_CLIENT_EVENT_LOCAL_NEWGAME:
                    break;
                case G_CLIENT_EVENT_LOCAL_LOADGAME:
                    break;
                case G_CLIENT_EVENT_LOCAL_ENTERGAME:
                    break;
                case G_CLIENT_EVENT_REMOTE_INFO:
                    break;
                case G_CLIENT_EVENT_REMOTE_CONNECTION_ACCEPTED:
                    break;
                case G_CLIENT_EVENT_REMOTE_CONNECTION_CLOSE:
                    FSM_CLIENT_DISCONECT();
                    break;
                case G_CLIENT_EVENT_REMOTE_WORLD_CREATE:
                    client_fsm_world_create(event);
                    FSM_GAMESTATE_SET(CLIENT_GAMESTATE_3_MISSION_BRIEF);
                    break;
                case G_CLIENT_EVENT_REMOTE_PLAYERS_ENTITY_SET:
                    break;
                case G_CLIENT_EVENT_REMOTE_GAME_ENDMAP:
                    break;
            }
            break;
        }
        case CLIENT_GAMESTATE_3_MISSION_BRIEF:
        {
            switch(event->type)
            {
                case G_CLIENT_EVENT_LOCAL_STOP:
                    client_clean();
                    client_stop();
                    FSM_GAMESTATE_SET(CLIENT_GAMESTATE_0_IDLE);
                    break;
                case G_CLIENT_EVENT_LOCAL_KEY_PRESS:
                    sound_play_start(NULL, 0, SOUND_MENU_ENTER, 1);
                    client_req_send_spawn(client.gstate.players_num);
                    FSM_GAMESTATE_SET(CLIENT_GAMESTATE_4_SPAWN_AWAITING);
                    break;
                case G_CLIENT_EVENT_LOCAL_KEY_RELEASE:
                    break;
                case G_CLIENT_EVENT_LOCAL_CONNECT:
                    FSM_WARNING_ALREADY_CONNECTED();
                    break;
                case G_CLIENT_EVENT_LOCAL_DICOVERYSERVER:
                    client_req_send_discoveryserver();
                    break;
                case G_CLIENT_EVENT_LOCAL_NEWGAME:
                    break;
                case G_CLIENT_EVENT_LOCAL_LOADGAME:
                    break;
                case G_CLIENT_EVENT_LOCAL_ENTERGAME:
                    break;
                case G_CLIENT_EVENT_REMOTE_INFO:
                    client_fsm_remote_info(event);
                    break;
                case G_CLIENT_EVENT_REMOTE_CONNECTION_ACCEPTED:
                    break;
                case G_CLIENT_EVENT_REMOTE_CONNECTION_CLOSE:
                    FSM_CLIENT_DISCONECT();
                    FSM_GAMESTATE_SET(CLIENT_GAMESTATE_1_NOGAME);
                    break;
                case G_CLIENT_EVENT_REMOTE_WORLD_CREATE:
                    client_fsm_world_create(event);
                    FSM_GAMESTATE_SET(CLIENT_GAMESTATE_3_MISSION_BRIEF);
                    break;
                case G_CLIENT_EVENT_REMOTE_PLAYERS_ENTITY_SET:
                    break;
                case G_CLIENT_EVENT_REMOTE_GAME_ENDMAP:
                    break;
            }
            break;
        }
        case CLIENT_GAMESTATE_4_SPAWN_AWAITING:
        {
            switch(event->type)
            {
                case G_CLIENT_EVENT_LOCAL_STOP:
                    client_clean();
                    client_stop();
                    FSM_GAMESTATE_SET(CLIENT_GAMESTATE_0_IDLE);
                    break;
                case G_CLIENT_EVENT_LOCAL_KEY_PRESS:
                    break;
                case G_CLIENT_EVENT_LOCAL_KEY_RELEASE:
                    break;
                case G_CLIENT_EVENT_LOCAL_CONNECT:
                    FSM_WARNING_ALREADY_CONNECTED();
                    break;
                case G_CLIENT_EVENT_LOCAL_DICOVERYSERVER:
                    client_req_send_discoveryserver();
                    break;
                case G_CLIENT_EVENT_LOCAL_NEWGAME:
                    break;
                case G_CLIENT_EVENT_LOCAL_LOADGAME:
                    break;
                case G_CLIENT_EVENT_LOCAL_ENTERGAME:
                    break;
                case G_CLIENT_EVENT_REMOTE_INFO:
                    client_fsm_remote_info(event);
                    break;
                case G_CLIENT_EVENT_REMOTE_CONNECTION_ACCEPTED:
                    break;
                case G_CLIENT_EVENT_REMOTE_CONNECTION_CLOSE:
                    FSM_CLIENT_DISCONECT();
                    break;
                case G_CLIENT_EVENT_REMOTE_WORLD_CREATE:
                    client_fsm_world_create(event);
                    FSM_GAMESTATE_SET(CLIENT_GAMESTATE_3_MISSION_BRIEF);
                    break;
                case G_CLIENT_EVENT_REMOTE_PLAYERS_ENTITY_SET:
                {
                    int players_num = event->data.REMOTE_PLAYERS_ENTITY_SET.players_num;
                    if(players_num > GAME_CLIENT_PLAYERSNUM_ASSIGN_CLIENT)
                    {
                        client.gstate.players_num = players_num;
                        game_console_send("CLIENT: server assign players amount equal %d", players_num);
                    }
                    else
                    {
                        game_console_send("CLIENT: server not assign players amount, client set equal %d", client.gstate.players_num);
                    }

                    bool success = true;
                    entity_t *entities[client.gstate.players_num];
                    for(size_t i = 0; i < client.gstate.players_num; i++)
                    {
                        entity_id_t entityId = event->data.REMOTE_PLAYERS_ENTITY_SET.ent[i].entityId;
                        entities[i] = entity_find_by_id(entityId);
                        if(!entities[i])
                        {
                            game_console_send("client: No entity #%ld, can not create player #%d", (long)entityId, i);
                            success = false;
                            break;
                        }
                    }
                    if(!success)
                    {
                        game_abort();
                        break;
                    }

                    for(size_t i = 0; i < client.gstate.players_num; i++)
                    {
                        client_player_t * player = Z_malloc(sizeof(client_player_t));
                        LIST2_PUSH(client.players, player);
                        player->entity = entities[i];
                    }
                    client_initcams();

                    if(client.sv_dedicated)
                    {
                        FSM_GAMESTATE_SET(CLIENT_GAMESTATE_6_INGAME);
                    }
                    else
                    {
                        game_menu_show(MENU_GAME_SAVE);
                        FSM_GAMESTATE_SET(CLIENT_GAMESTATE_5_GAMESAVE);
                    }
                    break;
                }
                case G_CLIENT_EVENT_REMOTE_GAME_ENDMAP:
                    break;
            }
            break;
        }
        case CLIENT_GAMESTATE_5_GAMESAVE:
        {
            switch(event->type)
            {
                case G_CLIENT_EVENT_LOCAL_STOP:
                    client_clean();
                    client_stop();
                    FSM_GAMESTATE_SET(CLIENT_GAMESTATE_0_IDLE);
                    break;
                case G_CLIENT_EVENT_LOCAL_KEY_PRESS:
                    break;
                case G_CLIENT_EVENT_LOCAL_KEY_RELEASE:
                    break;
                case G_CLIENT_EVENT_LOCAL_CONNECT:
                    FSM_WARNING_ALREADY_CONNECTED();
                    break;
                case G_CLIENT_EVENT_LOCAL_DICOVERYSERVER:
                    client_req_send_discoveryserver();
                    break;
                case G_CLIENT_EVENT_LOCAL_NEWGAME:
                    break;
                case G_CLIENT_EVENT_LOCAL_LOADGAME:
                    break;
                case G_CLIENT_EVENT_LOCAL_ENTERGAME:
                    game_menu_hide();
                    sound_play_stop(NULL, GAME_SOUND_MENU);
                    FSM_GAMESTATE_SET(CLIENT_GAMESTATE_6_INGAME);
                    break;
                case G_CLIENT_EVENT_REMOTE_INFO:
                    client_fsm_remote_info(event);
                    break;
                case G_CLIENT_EVENT_REMOTE_CONNECTION_ACCEPTED:
                    break;
                case G_CLIENT_EVENT_REMOTE_CONNECTION_CLOSE:
                    FSM_CLIENT_DISCONECT();
                    break;
                case G_CLIENT_EVENT_REMOTE_WORLD_CREATE:
                    client_fsm_world_create(event);
                    FSM_GAMESTATE_SET(CLIENT_GAMESTATE_3_MISSION_BRIEF);
                    break;
                case G_CLIENT_EVENT_REMOTE_PLAYERS_ENTITY_SET:
                    break;
                case G_CLIENT_EVENT_REMOTE_GAME_ENDMAP:
                    break;
            }
            break;
        }
        case CLIENT_GAMESTATE_6_INGAME:
        {
            switch(event->type)
            {
                case G_CLIENT_EVENT_LOCAL_STOP:
                    client_clean();
                    client_stop();
                    FSM_GAMESTATE_SET(CLIENT_GAMESTATE_0_IDLE);
                    break;
                case G_CLIENT_EVENT_LOCAL_KEY_PRESS:
                    client_key_press(event->data.LOCAL_KEY_PRESS.key);
                    break;
                case G_CLIENT_EVENT_LOCAL_KEY_RELEASE:
                    client_key_release(event->data.LOCAL_KEY_RELEASE.key);
                    break;
                case G_CLIENT_EVENT_LOCAL_CONNECT:
                    FSM_WARNING_ALREADY_CONNECTED();
                    break;
                case G_CLIENT_EVENT_LOCAL_DICOVERYSERVER:
                    client_req_send_discoveryserver();
                    break;
                case G_CLIENT_EVENT_LOCAL_NEWGAME:
                    break;
                case G_CLIENT_EVENT_LOCAL_LOADGAME:
                    break;
                case G_CLIENT_EVENT_LOCAL_ENTERGAME:
                    break;
                case G_CLIENT_EVENT_REMOTE_INFO:
                    client_fsm_remote_info(event);
                    break;
                case G_CLIENT_EVENT_REMOTE_CONNECTION_ACCEPTED:
                    break;
                case G_CLIENT_EVENT_REMOTE_CONNECTION_CLOSE:
                    FSM_CLIENT_DISCONECT();
                    break;
                case G_CLIENT_EVENT_REMOTE_WORLD_CREATE:
                    client_fsm_world_create(event);
                    FSM_GAMESTATE_SET(CLIENT_GAMESTATE_3_MISSION_BRIEF);
                    break;
                case G_CLIENT_EVENT_REMOTE_PLAYERS_ENTITY_SET:
                    break;
                case G_CLIENT_EVENT_REMOTE_GAME_ENDMAP:
                    client.gstate.win = event->data.REMOTE_GAME_ENDMAP.win;
                    client.gstate.endgame = event->data.REMOTE_GAME_ENDMAP.endgame;
                    sound_play_stop(NULL, GAME_SOUND_MENU);
                    sound_play_start(NULL, GAME_SOUND_MENU, SOUND_MUSIC1, -1);
                    game_console_send("client: server say: PLAYER WIN!");
                    FSM_GAMESTATE_SET(CLIENT_GAMESTATE_7_INTERMISSION);
                    break;
            }
            break;
        }
        case CLIENT_GAMESTATE_7_INTERMISSION:
        {
            switch(event->type)
            {
                case G_CLIENT_EVENT_LOCAL_STOP:
                    client_clean();
                    client_stop();
                    FSM_GAMESTATE_SET(CLIENT_GAMESTATE_0_IDLE);
                    break;
                case G_CLIENT_EVENT_LOCAL_KEY_PRESS:
                    sound_play_start(NULL, 0, SOUND_MENU_ENTER, 1);
                    client_req_send_ready();
                    client_players_delete();
                    if(client.gstate.endgame)
                        FSM_GAMESTATE_SET(CLIENT_GAMESTATE_8_ENDGAME);
                    else
                        FSM_GAMESTATE_SET(CLIENT_GAMESTATE_2_WORLD_CREATING);
                    break;
                case G_CLIENT_EVENT_LOCAL_KEY_RELEASE:
                    break;
                case G_CLIENT_EVENT_LOCAL_CONNECT:
                    FSM_WARNING_ALREADY_CONNECTED();
                    break;
                case G_CLIENT_EVENT_LOCAL_DICOVERYSERVER:
                    client_req_send_discoveryserver();
                    break;
                case G_CLIENT_EVENT_LOCAL_NEWGAME:
                    break;
                case G_CLIENT_EVENT_LOCAL_LOADGAME:
                    break;
                case G_CLIENT_EVENT_LOCAL_ENTERGAME:
                    break;
                case G_CLIENT_EVENT_REMOTE_INFO:
                    client_fsm_remote_info(event);
                    break;
                case G_CLIENT_EVENT_REMOTE_CONNECTION_ACCEPTED:
                    break;
                case G_CLIENT_EVENT_REMOTE_CONNECTION_CLOSE:
                    FSM_CLIENT_DISCONECT();
                    break;
                case G_CLIENT_EVENT_REMOTE_WORLD_CREATE:
                    client_fsm_world_create(event);
                    FSM_GAMESTATE_SET(CLIENT_GAMESTATE_3_MISSION_BRIEF);
                    break;
                case G_CLIENT_EVENT_REMOTE_PLAYERS_ENTITY_SET:
                    break;
                case G_CLIENT_EVENT_REMOTE_GAME_ENDMAP:
                    break;
            }
            break;
        }
        case CLIENT_GAMESTATE_8_ENDGAME:
        {
            switch(event->type)
            {
                case G_CLIENT_EVENT_LOCAL_STOP:
                    client_clean();
                    client_stop();
                    FSM_GAMESTATE_SET(CLIENT_GAMESTATE_0_IDLE);
                    break;
                case G_CLIENT_EVENT_LOCAL_KEY_PRESS:
                    client_clean();
                    FSM_GAMESTATE_SET(CLIENT_GAMESTATE_0_IDLE);
                    break;
                case G_CLIENT_EVENT_LOCAL_KEY_RELEASE:
                    break;
                case G_CLIENT_EVENT_LOCAL_CONNECT:
                    FSM_WARNING_ALREADY_CONNECTED();
                    break;
                case G_CLIENT_EVENT_LOCAL_DICOVERYSERVER:
                    client_req_send_discoveryserver();
                    break;
                case G_CLIENT_EVENT_LOCAL_NEWGAME:
                    break;
                case G_CLIENT_EVENT_LOCAL_LOADGAME:
                    break;
                case G_CLIENT_EVENT_LOCAL_ENTERGAME:
                    break;
                case G_CLIENT_EVENT_REMOTE_INFO:
                    client_fsm_remote_info(event);
                    break;
                case G_CLIENT_EVENT_REMOTE_CONNECTION_ACCEPTED:
                    break;
                case G_CLIENT_EVENT_REMOTE_CONNECTION_CLOSE:
                    FSM_CLIENT_DISCONECT();
                    break;
                case G_CLIENT_EVENT_REMOTE_WORLD_CREATE:
                    client_fsm_world_create(event);
                    FSM_GAMESTATE_SET(CLIENT_GAMESTATE_3_MISSION_BRIEF);
                    break;
                case G_CLIENT_EVENT_REMOTE_PLAYERS_ENTITY_SET:
                    break;
                case G_CLIENT_EVENT_REMOTE_GAME_ENDMAP:
                    break;
            }
            break;
        }
    }

    client.gamestate = gamestate;

}
