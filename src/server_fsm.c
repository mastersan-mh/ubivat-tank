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

#include "world.h"
#include "game.h"
#include "game_progs.h"
#include "game_progs_internal.h"
#include "g_gamesave.h"

#include <stddef.h>

#define FSM_LOCAL_STOP() \
        do { \
            game_stop(); \
            server_gamesave_clients_info_clean(); \
            server_tx(); \
            server_stop(); \
        } while (0)

#define FSM_CLIENT_CHECK(client) \
        if(!client) \
        { \
            game_console_send("server: no client " PRINTF_NETADDR_FMT ".", PRINTF_NETADDR_VAL(event->sender)); \
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

#define FSM_CLIENT_CONNECT_CHECK(client) \
                if((client)) \
                { \
                    game_console_send("server: client from " PRINTF_NETADDR_FMT " already connected.", PRINTF_NETADDR_VAL((client)->net_addr)); \
                    break; \
                }

static int server_world_recreate(const char * mapname)
{
    server_client_t * client;
    world_destroy();

    if(world_create(mapname))
    {
        game_console_send("server: Error: Could not create world");
        LIST2_FOREACH(server.clients, client)
        {
            server_reply_send_game_nextmap(client, true, NULL);
        }
        server_event_local_stop();
        return -1;
    }


    //игра по уровням
    LIST2_FOREACH(server.clients, client)
    {
        server_reply_send_game_nextmap(client, true, world_mapfilename_get());
    }
    return 0;
}

static void server_fsm_local_win(void)
{
    server_gamesave_clients_info_clean();

    server_client_t * client;

    server_clients_unspawn();
    if(!server.flags.localgame)
    {
        //игра по выбору
        LIST2_FOREACH(server.clients, client)
        {
            server_reply_send_game_nextmap(client, true, NULL);
        }
        server_event_local_stop();
        return;
    }


    server.gstate.gamemap = server.gstate.gamemap->next;

    if(!server.gstate.gamemap)
    {
        world_destroy();
        game_console_send("server: can't set next map.");
        // конец игры, последняя карта
        LIST2_FOREACH(server.clients, client)
        {
            server_reply_send_game_nextmap(client, true, NULL);
        }
        server_event_local_stop();
        return;
    }

    server_world_recreate(server.gstate.gamemap->filename);
}

static void server_fsm_discovery(const server_event_t * event)
{
    server_reply_send_info(&event->sender);
}

static void server_fsm_client_connect(const server_event_t * event)
{
    bool accepted = false;
    server_client_t * client;
    game_console_send("server: client request connection from " PRINTF_NETADDR_IPv4_FMT ".", PRINTF_NETADDR_VAL(event->sender));
    bool mainclient = true;
    client = server_client_create(server.sock, &event->sender, mainclient);
    if(client)
    {
        char clientinfo[] = "name1: \"player\", name2: \"player\"";
        accepted = game_client_player_connect(clientinfo);
        if(!accepted)
            server_client_delete(client);
    }
    server_reply_send_connection_result(client, accepted);
    if(world_valid())
        server_reply_send_game_nextmap(client, true, world_mapfilename_get());
}

static void server_fsm_client_disconnect(const server_event_t * event, server_client_t * client)
{
    if(!client)
    {
        game_console_send("SERVER: client " PRINTF_NETADDR_FMT " not found.", PRINTF_NETADDR_VAL(event->sender));
        return;
    }
    game_console_send("SERVER: client " PRINTF_NETADDR_FMT " require disconnection.", PRINTF_NETADDR_VAL(event->sender));

    server_reply_send_connection_close(client);
    server_client_delete(client);
}

static void server_fsm_remote_game_abort(const server_event_t * event, server_client_t * client)
{
    game_console_send("server: client " PRINTF_NETADDR_FMT " aborted game.", PRINTF_NETADDR_VAL(event->sender));
    LIST2_FOREACH(server.clients, client)
    {
        server_fsm_client_disconnect(event, client);
    }
    server_event_local_stop();
}

static void server_fsm_game_setmap(const server_event_t * event)
{
    const char * mapname = event->data.REMOTE_GAME_SETMAP.mapname;
    server.gstate.gamemap = map_find(mapname);
    if(!server.gstate.gamemap)
    {
        game_console_send("Error: Map \"%s\" not found.", mapname);
        //game_abort();
        return;
    }

    server_world_recreate(mapname);
}

static int server_fsm_gamesave_load(int isave)
{
    /* игра уже создана */
    gamesave_load_context_t ctx;
    if(g_gamesave_load_open(isave, &ctx))
        return -1;

    int res = g_gamesave_load_read_header(&ctx);
    if(res)
    {
        world_destroy();
        game_console_send("server: Error: Could not load gamesave.");
        return -1;
    }

    // создадим мир
    if(server_world_recreate(ctx.mapfilename))
    {
        g_gamesave_load_close(&ctx);
        return -1;
    }

    server_gamesave_clients_info_allocate(ctx.clients_num);

    for(size_t clientId = 0; clientId < ctx.clients_num; clientId++)
    {
        size_t players_num = ctx.clients_descr[clientId];
        for(size_t playerId = 0; playerId < players_num; playerId++)
        {
            server_player_vars_storage_t * storage = server_storage_find(clientId, playerId);
            g_gamesave_load_player(&ctx, storage);
            server_gamesave_client_info_set(clientId, players_num);
        }
    }

    server.flags.localgame = ctx.flag_localgame;
    server.flags.allow_respawn = ctx.flag_allow_respawn;

    g_gamesave_load_close(&ctx);

    return 0;
}

static void server_fsm_control_handle(const server_event_t * event, server_client_t * client)
{
    size_t playerId = event->data.REMOTE_PLAYER_ACTION.playerId;
    server_player_t * player = server_client_player_get_by_id(client, playerId);

    if(!player)
        return;
    game_console_send("server: from client " PRINTF_NETADDR_FMT " player %d received action \"%s\".",
        PRINTF_NETADDR_VAL(event->sender),
        event->data.REMOTE_PLAYER_ACTION.playerId,
        event->data.REMOTE_PLAYER_ACTION.action);

    /* execute action */
    entity_t * entity = player->entity;

    if(!entity)
        return;

    const entity_action_t * action = game_player_action_find(event->data.REMOTE_PLAYER_ACTION.action);

    if(!action)
    {
        game_console_send("server: unknown action \"%d.\"", event->data.REMOTE_PLAYER_ACTION.action);
        return;
    }

    if(action->action_f)
        action->action_f((ENTITY)entity, action->action);

}

void server_fsm(const server_event_t * event)
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
                server_fsm_discovery(event);
                break;
            case G_SERVER_EVENT_REMOTE_CLIENT_CONNECT:
                FSM_CLIENT_CONNECT_CHECK(client);
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
                server_fsm_remote_game_abort(event, client);
                break;
            case G_SERVER_EVENT_REMOTE_GAME_SETMAP:
                server_fsm_game_setmap(event);
                FSM_GAMESTATE_SET(SERVER_GAMESTATE_2_INGAME);
                break;
            case G_SERVER_EVENT_REMOTE_GAME_SAVE:
                break;
            case G_SERVER_EVENT_REMOTE_GAME_LOAD:
                server_fsm_gamesave_load(event->data.REMOTE_GAME_LOAD.isave);
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
                server_fsm_local_win();
                break;
            case G_SERVER_EVENT_REMOTE_DISCOVERYSERVER:
                server_fsm_discovery(event);
                break;
            case G_SERVER_EVENT_REMOTE_CLIENT_CONNECT:
                FSM_CLIENT_CONNECT_CHECK(client);
                server_fsm_client_connect(event);
                break;
            case G_SERVER_EVENT_REMOTE_CLIENT_DISCONNECT:
                FSM_CLIENT_CHECK(client);
                server_fsm_client_disconnect(event, client);
                break;
            case G_SERVER_EVENT_REMOTE_CLIENT_SPAWN:
            {
                FSM_CLIENT_CHECK(client);
                if(client->joined)
                {
                    game_console_send("server: client already joined.");
                    break;
                }
                int players_num = event->data.REMOTE_JOIN.players_num;

                size_t clientId = server_client_id_get(client);
                if(players_num == 0)
                {
                    // сервер сам выбирает количество игроков
                    players_num = server_gamesave_client_info_get(clientId);
                }
                if(server_client_spawn(client, players_num) != 0)
                {
                    game_console_send("server: can not spawn players, no entities or players to spawn.");
                    break;
                }

                server_gamesave_client_info_mark(clientId);
                server_reply_send_players_entity_set(client);
                game_console_send("server: client joined to game.");
                FSM_GAMESTATE_SET(gamestate);
                break;
            }
            case G_SERVER_EVENT_REMOTE_CLIENT_READY:
                break;
            case G_SERVER_EVENT_REMOTE_CLIENT_PLAYER_ACTION:
                server_fsm_control_handle(event, client);
                break;
            case G_SERVER_EVENT_REMOTE_GAME_ABORT:
                FSM_CLIENT_CHECK_PRIVILEGED(client);
                server_fsm_remote_game_abort(event, client);
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
    }

    server.gamestate = gamestate;
}
