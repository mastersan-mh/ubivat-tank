/*
 * serve_fsm.c
 *
 *  Created on: 3 нояб. 2017 г.
 *      Author: mastersan
 */

#include "server_private.h"
#include "server.h"
#include "server_reply.h"
#include "sv_game.h"
#include "server_fsm.h"
#include "game.h"
#include "g_gamesave.h"

static void server_client_fsm_control_handle(server_client_t * client, const game_client_request_t * req)
{
    server_player_t * player = server_client_player_get_by_id(client, req->data.PLAYER_ACTION.playerId);

    /* TODO
    game_console_send("server: from 0x%00000000x:%d received player action %s.", sender.addr_in.sin_addr, ntohs(sender.addr_in.sin_port),
        req.control.action
    );
     */

    if(!player)
        return;
    game_console_send("server: player %d received action %s.",
        req->data.PLAYER_ACTION.playerId,
        req->data.PLAYER_ACTION.action);

    /* execute action */
    entity_t * ent = player->entity;

    if(!ent)
        return;

    const entityinfo_t * info = ent->info;

    const entityaction_t * action = server_entity_action_find(ent, req->data.PLAYER_ACTION.action);

    if(!action)
    {
        game_console_send("server: unknown action :%d.", req->data.PLAYER_ACTION.action);
        return;
    }

    if(!ent->spawned)
    {
        if(server.flags.allow_respawn &&
                info->spawn)
        {
            game_console_send("server: spawn client.");
            server_client_player_info_restore(player);
            info->spawn(ent, ent->edata);
            ent->spawned = true;
            ent->alive = true;
        }
        return;
    }

    switch(server.gamestate.state)
    {
    case GAMESTATE_1_NOGAME:
        break;
    case GAMESTATE_2_MISSION_BRIEF:
        break;
    case GAMESTATE_4_GAMESAVE:
        break;
    case GAMESTATE_3_JOIN_AWAITING:
        break;
    case GAMESTATE_5_INGAME:
        if(action->action_f)
            action->action_f(ent, ent->edata, action->action);
        break;
    case GAMESTATE_6_INTERMISSION:
        break;
    }

}


void server_client_fsm(const net_addr_t * sender, const game_client_request_t * req)
{
    server_client_t * client = server_client_find_by_addr(sender);
    switch(req->type)
    {
    /* Непривилегированые запросы */
    case G_CLIENT_REQ_DISCOVERYSERVER:
        /*
        host_event_info_send(ns);
         */
        break;
    case G_CLIENT_REQ_CONNECT:
    {
        if(client)
        {
            game_console_send("server: client from 0x%00000000x:%d  already connected.", client->ns.addr_.addr_in, ntohs(client->ns.addr_.addr_in.sin_port));
            break;
        }
        game_console_send("server: client request connection from 0x%00000000x:%d.", sender->addr_in.sin_addr, ntohs(sender->addr_in.sin_port));
        bool mainclient = true;
        //net_socket_t * ns = net_socket_create_sockaddr(sender.addr);
        client = server_client_create(server.ns->sock, sender, mainclient);
/*
TODO:
        ctx = server_loadedclient_get(i);
        if(ctx)
        {
        exiting_client_get(i).settings = ctx.settings;
    //       server_client_players_num_set(client, players_num);
        }
*/


        server_reply_send_connection_accepted(client);
        server_reply_send_gamestate(client, server.gamestate.state);

        break;
    }
    case G_CLIENT_REQ_DISCONNECT:
        if(!client)
        {
            game_console_send("SERVER: client 0x%00000000x:%d not found.", sender->addr_in.sin_addr, ntohs(sender->addr_in.sin_port));
            break;
        }
        game_console_send("SERVER: client 0x%00000000x:%d require disconnection.", sender->addr_in.sin_addr, ntohs(sender->addr_in.sin_port));
        server_reply_send_gamestate(client, server.gamestate.state);
        server_client_disconnect(client);
        server_client_delete(client);
        client = NULL;
        break;
    case G_CLIENT_REQ_JOIN:
        if(client->joined)
        {
            game_console_send("server: client already joined.");
            break;
        }

        if(server_client_join(client, req->data.JOIN.players_num) != 0)
        {
            game_console_send("server: can not join client, no entities to spawn.");
            break;
        }

        server_reply_send_gamestate(client, server.gamestate.state);
        server_reply_send_cliententity(client);
        game_console_send("server: client joined to game.");
        break;
    case G_CLIENT_REQ_PLAYER_ACTION:
        server_client_fsm_control_handle(client, req);
        break;
    /* Привилегированные запросы */
    case G_CLIENT_REQ_GAME_ABORT:
        if(!client)
        {
            game_console_send("server: no client 0x%00000000x:%d.", sender->addr_in.sin_addr, ntohs(sender->addr_in.sin_port));
            break;
        }

        if(!client->main)
            break;
        game_console_send("server: client aborted game.");
        sv_game_abort();
        break;
    case G_CLIENT_REQ_GAME_SETMAP:
    {
        const char * mapname = req->data.GAME_SETMAP.mapname;
        server.gamestate.gamemap = map_find(mapname);
        if(!server.gamestate.gamemap)
        {
            game_console_send("Error: Map \"%s\" not found.", mapname);
            //game_abort();
            break;
        }
        if(map_load(mapname))
        {
            game_console_send("Error: Could not load map \"%s\".", mapname);
            //game_abort();
        }
        break;
    }
    case G_CLIENT_REQ_GAME_NEXTSTATE:
    {
        gamestate_t gamestate_next = server.gamestate.state;
        switch(server.gamestate.state)
        {
        case GAMESTATE_1_NOGAME:
            break;
        case GAMESTATE_2_MISSION_BRIEF:
            gamestate_next = GAMESTATE_3_JOIN_AWAITING;
            server_reply_send_player_join_awaiting(client);
            break;
        case GAMESTATE_3_JOIN_AWAITING:
            if(server.gamestate.allow_state_gamesave)
                gamestate_next = GAMESTATE_4_GAMESAVE;
            else
                gamestate_next = GAMESTATE_5_INGAME;
            break;
        case GAMESTATE_4_GAMESAVE:
            gamestate_next = GAMESTATE_5_INGAME;
            break;
        case GAMESTATE_5_INGAME:
            break;
        case GAMESTATE_6_INTERMISSION:
            gamestate_next = GAMESTATE_2_MISSION_BRIEF;
            break;
        }

        server_reply_send_gamestate(client, gamestate_next);

        server.gamestate.state = gamestate_next;

        break;
    }
    case G_CLIENT_REQ_GAME_SAVE:
        g_gamesave_save(req->data.GAME_SAVE.isave);
        break;
    case G_CLIENT_REQ_GAME_LOAD:
        server_gamesave_load(req->data.GAME_LOAD.isave);
        break;
    }

}
