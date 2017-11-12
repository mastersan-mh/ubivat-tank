/*
 * serve_fsm.c
 *
 *  Created on: 3 нояб. 2017 г.
 *      Author: mastersan
 */

#include "common/common_list2.h"
#include "types.h"

#include "server_fsm.h"

#include "server.h"
#include "server_private.h"
#include "server_reply_private.h"
#include "server_reply.h"
#include "server_events.h"
#include "sv_game.h"


#include "game.h"
#include "g_gamesave.h"

#include <stddef.h>

extern bool sv_entity_valid;

#define FSM_LOCAL_STOP() \
        do { \
            server_tx(); \
            server_stop(); \
        } while (0)

#define FSM_CLIENT_CHECK(client) \
        if(!client) \
        { \
            game_console_send("server: no client 0x%00000000x:%d.", \
                event->sender.addr_in.sin_addr, ntohs(event->sender.addr_in.sin_port)); \
                break; \
        }

#define FSM_CLIENT_CHECK_PRIVILEGED(client) \
            FSM_CLIENT_CHECK((client)); \
            if(!client->main) break;

#define FSM_GAMESTATE_SET(gs) \
        do { \
            gamestate = (gs); \
            game_console_send("SERVER: change gamestate to %s.", server_gamestate_to_str(gamestate)); \
        } while (0)

#define FSM_CLIENT_CONNECT_CHECK() \
                if(client) \
                { \
                    game_console_send("server: client from 0x%00000000x:%d  already connected.", \
                        client->ns.addr_.addr_in, ntohs(client->ns.addr_.addr_in.sin_port)); \
                    break; \
                }

void server_fsm_client_connect(const game_server_event_t * event)
{
    server_client_t * client;
    game_console_send("server: client request connection from 0x%00000000x:%d.",
        event->sender.addr_in.sin_addr, ntohs(event->sender.addr_in.sin_port));
    bool mainclient = true;
    //net_socket_t * ns = net_socket_create_sockaddr(event->sender.addr);
    client = server_client_create(server.ns->sock, &event->sender, mainclient);
    server_reply_send_connection_accepted(client);
}

void server_fsm_client_disconnect(const game_server_event_t * event, server_client_t * client)
{
    if(!client)
    {
        game_console_send("SERVER: client 0x%00000000x:%d not found.",
            event->sender.addr_in.sin_addr, ntohs(event->sender.addr_in.sin_port));
        return;
    }
    game_console_send("SERVER: client 0x%00000000x:%d require disconnection.",
        event->sender.addr_in.sin_addr, ntohs(event->sender.addr_in.sin_port));

    server_reply_send_connection_close(client);
    server_client_delete(client);
}

void server_fsm_game_abort(const game_server_event_t * event, server_client_t * client)
{
    game_console_send("server: client 0x%00000000x:%d aborted game.",
        event->sender.addr_in.sin_addr, ntohs(event->sender.addr_in.sin_port));
    LIST2_FOREACH(server.clients, client)
    {
        server_fsm_client_disconnect(event, client);
    }
    server_event_local_stop();
}

void server_fsm_game_setmap(const game_server_event_t * event)
{
    const char * mapname = event->data.REMOTE_GAME_SETMAP.mapname;
    server.gstate.gamemap = map_find(mapname);
    if(!server.gstate.gamemap)
    {
        game_console_send("Error: Map \"%s\" not found.", mapname);
        //game_abort();
        return;
    }
    if(map_load(mapname))
    {
        game_console_send("Error: Could not load map \"%s\".", mapname);
        //game_abort();
    }
}

static void server_client_fsm_control_handle(const game_server_event_t * event, server_client_t * client)
{
    size_t clientId = server_client_id_get(client);
    size_t playerId = event->data.REMOTE_PLAYER_ACTION.playerId;
    server_player_t * player = server_client_player_get_by_id(client, playerId);

    /* TODO
    game_console_send("server: from 0x%00000000x:%d received player action %s.", sender.addr_in.sin_addr, ntohs(sender.addr_in.sin_port),
        req.control.action
    );
     */

    if(!player)
        return;
    game_console_send("server: from player %d received action \"%s.\"",
        event->data.REMOTE_PLAYER_ACTION.playerId,
        event->data.REMOTE_PLAYER_ACTION.action);

    /* execute action */
    entity_t * entity = player->entity;

    if(!entity)
        return;

    if(server.flags.allow_respawn && !entity->spawned)
    {
        game_console_send("server: respawn player.");
        server_player_vars_storage_t * storage = server_storage_find(clientId, playerId);
        void * vars = storage ? storage->vars : NULL;
        entity_respawn(entity, vars);
        return;
    }

    const entityaction_t * action = server_entity_action_find(entity, event->data.REMOTE_PLAYER_ACTION.action);

    if(!action)
    {
        game_console_send("server: unknown action \"%d.\"", event->data.REMOTE_PLAYER_ACTION.action);
        return;
    }

    if(action->action_f)
        action->action_f((ENTITY)entity, action->action);

}


void server_fsm(const game_server_event_t * event)
{
    server_gamestate_t gamestate = server.gamestate;

    server_client_t * client;
    if(event->type < G_SERVER_EVENT_REMOTE)
        client = NULL;
    else
        client = server_client_find_by_addr(&event->sender);

    switch(server.gamestate)
    {
    case SERVER_GAMESTATE_1_NOGAME:
        switch(event->type)
        {
            case G_SERVER_EVENT_LOCAL_STOP:
                FSM_LOCAL_STOP();
                break;
            case G_SERVER_EVENT_LOCAL_WIN:
                break;
            case G_SERVER_EVENT_REMOTE_DISCOVERYSERVER:
                break;
            case G_SERVER_EVENT_REMOTE_CLIENT_CONNECT:
                FSM_CLIENT_CONNECT_CHECK();
                server_fsm_client_connect(event);
                break;
            case G_SERVER_EVENT_REMOTE_CLIENT_DISCONNECT:
                FSM_CLIENT_CHECK(client);
                server_fsm_client_disconnect(event, client);
                break;
            case G_SERVER_EVENT_REMOTE_CLIENT_SPAWN:
                break;
            case G_SERVER_EVENT_REMOTE_CLIENT_READY:
                break;
            case G_SERVER_EVENT_REMOTE_CLIENT_PLAYER_ACTION:
                break;
            case G_SERVER_EVENT_REMOTE_GAME_ABORT:
                FSM_CLIENT_CHECK_PRIVILEGED(client);
                server_fsm_game_abort(event, client);
                break;
            case G_SERVER_EVENT_REMOTE_GAME_SETMAP:
                server_fsm_game_setmap(event);
                FSM_GAMESTATE_SET(SERVER_GAMESTATE_2_INGAME);
                break;
            case G_SERVER_EVENT_REMOTE_GAME_SAVE:
                break;
            case G_SERVER_EVENT_REMOTE_GAME_LOAD:
                server_gamesave_load(event->data.REMOTE_GAME_LOAD.isave);
                FSM_GAMESTATE_SET(SERVER_GAMESTATE_2_INGAME);
                break;
        }
        break;
    case SERVER_GAMESTATE_2_INGAME:
        switch(event->type)
        {
            case G_SERVER_EVENT_LOCAL_STOP:
                FSM_LOCAL_STOP();
                break;
            case G_SERVER_EVENT_LOCAL_WIN:
            {
                sv_entity_valid = false;

                server_client_t * client;
                server_clients_unspawn();
                if(!server.flags.localgame)
                {
                    //игра по выбору
                    LIST2_FOREACH(server.clients, client)
                    {
                        server_reply_send_game_endmap(client, true, true);
                    }
                    server_event_local_stop();
                    break;
                }
                //игра по уровням
                if(sv_game_nextmap())
                {
                    LIST2_FOREACH(server.clients, client)
                    {
                        server_reply_send_game_endmap(client, true, true);
                    }
                    game_console_send("server: can't next map.");
                    server_event_local_stop();
                    break;
                }
                LIST2_FOREACH(server.clients, client)
                {
                    server_reply_send_game_endmap(client, true, false);
                }
                FSM_GAMESTATE_SET(SERVER_GAMESTATE_2_INGAME);
                //FSM_GAMESTATE_SET(SERVER_GAMESTATE_3_INTERMISSION);
                break;
            }
            case G_SERVER_EVENT_REMOTE_DISCOVERYSERVER:
                break;
            case G_SERVER_EVENT_REMOTE_CLIENT_CONNECT:
                FSM_CLIENT_CONNECT_CHECK();
                server_fsm_client_connect(event);
                break;
            case G_SERVER_EVENT_REMOTE_CLIENT_DISCONNECT:
                FSM_CLIENT_CHECK(client);
                server_fsm_client_disconnect(event, client);
                break;
            case G_SERVER_EVENT_REMOTE_CLIENT_SPAWN:
                sv_entity_valid = true;
                FSM_CLIENT_CHECK(client);
                if(client->joined)
                {
                    game_console_send("server: client already joined.");
                    break;
                }
                if(server_client_spawn(client, event->data.REMOTE_JOIN.players_num) != 0)
                {
                    game_console_send("server: can not spawn players, no entities to spawn.");
                    break;
                }
                FSM_GAMESTATE_SET(gamestate);
                server_reply_send_players_entity_set(client);
                game_console_send("server: client joined to game.");
                break;
            case G_SERVER_EVENT_REMOTE_CLIENT_READY:
                break;
            case G_SERVER_EVENT_REMOTE_CLIENT_PLAYER_ACTION:
                server_client_fsm_control_handle(event, client);
                break;
            case G_SERVER_EVENT_REMOTE_GAME_ABORT:
                FSM_CLIENT_CHECK_PRIVILEGED(client);
                server_fsm_game_abort(event, client);
                break;
            case G_SERVER_EVENT_REMOTE_GAME_SETMAP:
                break;
            case G_SERVER_EVENT_REMOTE_GAME_SAVE:
                g_gamesave_save(event->data.REMOTE_GAME_SAVE.isave);
                break;
            case G_SERVER_EVENT_REMOTE_GAME_LOAD:
                break;
        }
        break;
    case SERVER_GAMESTATE_3_INTERMISSION:
        switch(event->type)
        {
            case G_SERVER_EVENT_LOCAL_STOP:
                FSM_LOCAL_STOP();
                break;
            case G_SERVER_EVENT_LOCAL_WIN:
                break;
            case G_SERVER_EVENT_REMOTE_DISCOVERYSERVER:
                break;
            case G_SERVER_EVENT_REMOTE_CLIENT_CONNECT:
                break;
            case G_SERVER_EVENT_REMOTE_CLIENT_DISCONNECT:
                FSM_CLIENT_CHECK(client);
                server_fsm_client_disconnect(event, client);
                break;
            case G_SERVER_EVENT_REMOTE_CLIENT_SPAWN:
                break;
            case G_SERVER_EVENT_REMOTE_CLIENT_READY:
                FSM_GAMESTATE_SET(SERVER_GAMESTATE_1_NOGAME);
                break;
            case G_SERVER_EVENT_REMOTE_CLIENT_PLAYER_ACTION:
                break;
            case G_SERVER_EVENT_REMOTE_GAME_ABORT:
                FSM_CLIENT_CHECK_PRIVILEGED(client);
                server_fsm_game_abort(event, client);
                break;
            case G_SERVER_EVENT_REMOTE_GAME_SETMAP:
                break;
            case G_SERVER_EVENT_REMOTE_GAME_SAVE:
                break;
            case G_SERVER_EVENT_REMOTE_GAME_LOAD:
                break;
        }
        break;
    }

    server.gamestate = gamestate;
}
