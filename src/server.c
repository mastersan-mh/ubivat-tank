/*
 * server.c
 *
 *  Created on: 27 дек. 2016 г.
 *      Author: mastersan
 */

#include "common/common_list2.h"
#include "common/common_hash.h"
#include "vars.h"
#include "g_events.h"
#include "sv_game.h"
#include "net_pdu.h"
#include "net.h"
#include "server.h"
#include "client_request.h"
#include "game.h"
#include "g_gamesave.h"
#include "map.h"
#include "menu.h"
#include "entity.h"

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
    server.gamestate.custommap = mapList;
    server.gamestate.gamemap   = mapList;
}

void server_done(void)
{

}

static void server_reply_send(server_client_t * client, const game_server_event_t * req)
{
    if(client->tx_queue_num >= SERVER_TX_QUEUE_SIZE)
    {
        game_console_send("SERVER: TX queue overflow.");
        return;
    }
    client->tx_queue[client->tx_queue_num].req = *req;
    client->tx_queue_num++;
}

void server_reply_send_info(server_client_t * client)
{
    game_server_event_t event;
    event.type = G_SERVER_EVENT_INFO;
    event.data.INFO.clients_num = 0;
    server_reply_send(client, &event);
}

void server_reply_send_connection_accepted(server_client_t * client)
{
    game_server_event_t event;
    event.type = G_SERVER_EVENT_CONNECTION_ACCEPTED;
    server_reply_send(client, &event);
}

void server_reply_send_player_join_awaiting(server_client_t * client)
{
    game_server_event_t event;
    event.type = G_SERVER_EVENT_PLAYERS_JOIN_AWAITING;
    event.data.PLAYERS_JOIN_AWAITING.players_num = client->players_num;
    server_reply_send(client, &event);
}

void server_reply_send_cliententity(server_client_t * client)
{
    game_server_event_t event;
    event.type = G_SERVER_EVENT_PLAYERS_ENTITY_SET;

    int i = 0;
    server_player_t * player;

    LIST2_FOREACHR(client->players, player)
    {
        if(i >= client->players_num)
            break;
        strncpy(event.data.PLAYERS_ENTITY_SET.ent[i].entityname, player->entity->info->name, GAME_SERVER_EVENT_ENTNAME_SIZE);
        event.data.PLAYERS_ENTITY_SET.ent[i].entity = player->entity;
        i++;
    }
    server_reply_send(client, &event);

}

void server_reply_send_gamestate(server_client_t * client, gamestate_t state)
{
    game_server_event_t event;
    event.type = G_SERVER_EVENT_GAME_STATE_SET;
    event.data.GAME_STATE_SET.state = state;
    server_reply_send(client, &event);
}

/*
 * получить данные переменной
 */
vardata_t * server_client_vardata_get(server_player_t * player, const char * varname, vartype_t vartype)
{
    static const char * list[] =
    {
            "INTEGER",
            "FLOAT",
            "STRING",
    };
    var_t * var = var_find(player->vars, varname);
    if(!var)
    {
        var = var_create(&player->vars, varname, vartype);
    }
    vardata_t * vardata = var->data;
    if( (int)vartype >= 0 && vardata->type != vartype )
    {
        game_console_send("Warning: Host client variable \"%s\" has type %s, but used as %s.", varname, list[vardata->type], list[vartype]);
    }
    return vardata;
}

/**
 * @brief сохранение информации о entity игрока в хранилище игрока
 */
static void server_client_player_info_store(server_player_t * player)
{
    if(player->entity->info->client_store)
        player->userstoredata = player->entity->info->client_store(
                player->entity->edata
        );

    entity_t * entity = player->entity;
    size_t vars_num = entity->info->vars_num;
    entityvarinfo_t * vars = entity->info->vars;

    size_t i;
    for(i = 0; i < vars_num; i++)
    {
        var_t * var = var_create(&player->vars, vars[i].name, vars[i].type);
        vardata_t * clientvardata = (vardata_t*)var->data;
        vardata_t * entityvardata = entity_vardata_get(entity, vars[i].name, -1);
        strncpy(clientvardata->name, vars[i].name, VARNAME_SIZE);
        clientvardata->type  = entityvardata->type;
        clientvardata->value = entityvardata->value;
    }

}

/**
 * @brief восстановление информации о entity игрока из хранилища игрока в entity
 * @brief (при переходе на следующий уровень и при чтении gamesave)
 */
static void server_client_player_info_restore(server_player_t * player)
{
    if(player->userstoredata)
    {
        player->entity->info->client_restore(
            player->entity,
            player->entity->edata,
            player->userstoredata
        );
        Z_free(player->userstoredata);
        player->userstoredata = NULL;
    }

    if(player->vars)
    {
        entity_t * entity = player->entity;

        void var_restore(vardata_t * clientvardata, void * args)
        {
            vardata_t * entityvardata = entity_vardata_get(entity, clientvardata->name, -1);
            if(!entityvardata)
                game_console_send("Error: Can not restore client entity info, entity has no variable \"%s\".", clientvardata->name);
            else
            {
                if(entityvardata->type != clientvardata->type)
                {
                    game_console_send("Error: Can not restore client entity info, variable \"%s\" has different types.", clientvardata->name);
                }
                else
                {
                    strncpy(entityvardata->name, clientvardata->name, VARNAME_SIZE);
                    entityvardata->type  = clientvardata->type;
                    entityvardata->value = clientvardata->value;
                }
            }
        }

        vars_dump(player->vars, "==== Client vars:");

        vars_foreach(player->vars, var_restore, NULL);
        vars_dump(player->entity->vars, "==== Entity vars:");

        vars_delete(&player->vars);
    }

}

static server_player_t * server_player_create()
{
    server_player_t * player = Z_malloc(sizeof(server_player_t));
    if(!player)
        game_halt("server_player_create(): Can not alloc memory, failed");
    player->entity = NULL;
    player->userstoredata = NULL;
    return player;
}

static void server_player_delete(server_player_t * player)
{
    Z_free(player->userstoredata);
    Z_free(player);
}

int server_client_players_num_get(const server_client_t * client)
{
    server_player_t * player;
    int num;
    LIST2_FOREACH_I(client->players, player, num);
    return num;
}

server_player_t * server_client_player_get_by_id(const server_client_t * client, int playerId)
{
    int players_num = server_client_players_num_get(client);
    if(playerId < 0 || playerId >= players_num)
        return NULL;
    server_player_t * player = client->players;
    playerId = players_num - 1 - playerId;
    int i;
    LIST2_LIST_TO_IENT(client->players, player, i, playerId);
    return player;
}
static server_client_t * server_client_create(int sock, const net_addr_t * net_addr, bool main)
{
    server_client_t * client = Z_malloc(sizeof(server_client_t));
    if(!client)
        game_halt("server_client_create(): Can not alloc memory, failed");

    client->joined = false;
    client->main = main;
    client->ns.sock = sock;
    client->ns.addr_ = *net_addr;
    client->players_num = GAME_CLIENT_PLAYERSNUM_ASSIGN_CLIENT;
    client->players = NULL;
    client->tx_queue_num = 0;
    LIST2_PUSH(server.clients, client);
    return client;
}

static void server_client_players_num_set(server_client_t * client, int players_num)
{
    client->players_num = players_num;
}


static void server_client_delete(server_client_t * client)
{
    server_player_t * player;
    while(!LIST2_IS_EMPTY(client->players))
    {
        player = client->players;
        LIST2_UNLINK(client->players, player);
        server_player_delete(player);
    }
    Z_free(client);
}

static void server_clients_delete(void)
{
    server_client_t * client;
    while(!LIST2_IS_EMPTY(server.clients))
    {
        client = server.clients;
        LIST2_UNLINK(server.clients, client);
        server_client_delete(client);
    }
}

static void server_client_disconnect(server_client_t * client)
{
    game_server_event_t event;
    event.type = G_SERVER_EVENT_CONNECTION_CLOSE;
    server_reply_send(client, &event);
}


static server_client_t * server_client_find_by_addr(const net_addr_t * addr)
{
    server_client_t * client;
    LIST2_FOREACH(server.clients, client)
    {
        if( !memcmp(&client->ns.addr_.addr, &addr->addr, sizeof(struct sockaddr)) )
            return client;
    }
    return NULL;
}

int server_client_num_get(void)
{
    server_client_t * client;
    int num;
    LIST2_FOREACH_I(server.clients, client, num);
    return num;
}

server_client_t * server_client_get(int id)
{
    int clients_num = server_client_num_get();
    if(id < 0 || id >= clients_num)
        return NULL;
    server_client_t * client = server.clients;
    id = clients_num - 1 - id;
    int i;
    LIST2_LIST_TO_IENT(server.clients, client, i, id);
    return client;
}

int server_client_join(server_client_t * client, int players_num)
{
    client->players_num = players_num;
    for(int i = 0; i < players_num; i++)
    {
        server_player_t * player = server_player_create();
        LIST2_PUSH(client->players, player);

        entity_t * entity = entries_client_join();
        if(!entity)
        {
            game_console_send("Error: No entity to spawn client.");
            return -1;
        }
        player->entity = entity;
        server_client_player_info_store(player);
    }
    client->joined = true;
    return 0;
}

/**
 * @description убирание клиентов из игры (не дисконект!), сохранение информации о клиентах
 */
void server_unjoin_clients(void)
{
    server_client_t * client;
    LIST2_FOREACHR(server.clients, client)
    {
        server_player_t * player;
        LIST2_FOREACHR(client->players, player)
        {
            server_client_player_info_store(player);
            player->entity = NULL;
        }
    }
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


static int server_gamesave_load(int isave)
{
    /* игра уже создана */
    gamesave_load_context_t ctx;
    if(g_gamesave_load_open(isave, &ctx))
        return -1;

    map_clear();

    //прочитаем карту
    if(map_load(ctx.mapfilename))
    {
        game_console_send("Error: Could not load map \"%s\".", ctx.mapfilename);
        //game_abort();

        g_gamesave_load_close(&ctx);
        return -1;
    }

    server.flags.localgame = ctx.flag_localgame;
    server.flags.allow_respawn = ctx.flag_allow_respawn;

    g_gamesave_load_close(&ctx);

    server.gamestate.allow_state_gamesave = false;
/* TODO:
    foreach_loaded_client()
    {
    server.loaded_client[i] = ctx.loadedclient[i];

    exiting_client_get(i).settings = server.loaded_client[i].settings;
//       server_client_players_num_set(client, players_num);

    }
    */
    return 0;
}

static entityaction_t * server_entity_action_find(entity_t * ent, const char * action_str)
{
    const entityinfo_t * info = ent->info;

    for(size_t i = 0; i < info->actions_num; i++)
    {
        entityaction_t * action = &info->actions[i];
        if(strncmp(action->action, action_str, GAME_CLIENT_REQ_PLAYER_ACTION_SIZE) == 0)
        {
            return action;
        }
    }
    return NULL;
}


void server_client_control_handle(server_client_t * client, const game_client_request_t * req)
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

static void server_fsm(const net_addr_t * sender, const game_client_request_t * req)
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
            game_console_send("server: client from 0x%00000000x:%d  already connected.", sender->addr_in.sin_addr, ntohs(sender->addr_in.sin_port));
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
        server_client_control_handle(client, req);
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


static int server_pdu_parse(const net_addr_t * sender, const char * buf, size_t buf_len)
{
    size_t sv_req_queue_num;
    game_client_request_t client_req;

    size_t ofs = 0;
    int16_t value16;

    PDU_POP_BUF(&value16, sizeof(value16));
    sv_req_queue_num = ntohs(value16);
    for(size_t i = 0; i < sv_req_queue_num; i++)
    {
        PDU_POP_BUF(&value16, sizeof(value16));
        client_req.type = htons(value16);
        switch(client_req.type)
        {
        /** Непривилегированые запросы */
        case G_CLIENT_REQ_DISCOVERYSERVER:
            break;
        case G_CLIENT_REQ_CONNECT:
            break;
        case G_CLIENT_REQ_DISCONNECT:
            break;
        case G_CLIENT_REQ_JOIN:
            PDU_POP_BUF(&value16, sizeof(value16));
            client_req.data.JOIN.players_num = ntohs(value16);
            break;
        case G_CLIENT_REQ_PLAYER_ACTION:
            PDU_POP_BUF(&value16, sizeof(value16));
            client_req.data.PLAYER_ACTION.playerId = ntohs(value16);
            PDU_POP_BUF(client_req.data.PLAYER_ACTION.action, GAME_CLIENT_REQ_PLAYER_ACTION_SIZE);
            break;
            /** Привилегированные запросы */
        case G_CLIENT_REQ_GAME_ABORT:
            break;
        case G_CLIENT_REQ_GAME_SETMAP:
            PDU_POP_BUF(client_req.data.GAME_SETMAP.mapname, MAP_FILENAME_SIZE);
            break;
        case G_CLIENT_REQ_GAME_NEXTSTATE:
            break;
        case G_CLIENT_REQ_GAME_SAVE:
            PDU_POP_BUF(&value16, sizeof(value16));
            client_req.data.GAME_SAVE.isave = ntohs(value16);
            break;
        case G_CLIENT_REQ_GAME_LOAD:
            PDU_POP_BUF(&value16, sizeof(value16));
            client_req.data.GAME_LOAD.isave = ntohs(value16);
            break;
        }
        server_fsm(sender, &client_req);
    }
    return 0;
}

int server_pdu_build(server_client_t * client, char * buf, size_t * buf_len, size_t buf_size)
{
    int16_t value16;
    size_t i;
    size_t ofs = 0;

    value16 = htons(client->tx_queue_num);
    PDU_PUSH_BUF(&value16, sizeof(value16));

    for(i = 0; i < client->tx_queue_num; i++)
    {
        game_server_event_t *event = &client->tx_queue[i].req;
        value16 = htons(event->type);
        PDU_PUSH_BUF(&value16, sizeof(value16));
        switch(event->type)
        {
        case G_SERVER_EVENT_INFO:
            /*
            nvalue32 = htonl( (uint32_t) event->info.clients_num );
            memcpy(&buf[buflen], &nvalue32, sizeof(nvalue32));
            buflen += sizeof(nvalue32);
             */
            break;
        case G_SERVER_EVENT_CONNECTION_ACCEPTED:
            break;
        case G_SERVER_EVENT_CONNECTION_CLOSE:
            break;
        case G_SERVER_EVENT_GAME_STATE_SET:
            value16 = htons(event->data.GAME_STATE_SET.state);
            PDU_PUSH_BUF(&value16, sizeof(value16));
            break;
        case G_SERVER_EVENT_PLAYERS_JOIN_AWAITING:
            value16 = htons(event->data.PLAYERS_JOIN_AWAITING.players_num);
            PDU_PUSH_BUF(&value16, sizeof(value16));
            break;
        case G_SERVER_EVENT_PLAYERS_ENTITY_SET:
        {
            int player_num = client->players_num;
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


static void server_listen(void)
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
        break;
    case SERVER_STATE_INIT:
        server.gamestate.state = GAMESTATE_2_MISSION_BRIEF;
        server.gamestate.paused = false;

        server.ns = net_socket_create(NET_PORT, "127.0.0.1");

        if(server.ns == NULL)
        {
            game_halt("socket() failed");
        }
        if(net_socket_bind(server.ns) < 0)
        {
            game_halt("server bind() failed");
        }
        server.gamestate.allow_state_gamesave = true;
        server.state = SERVER_STATE_RUN;
        break;
    case SERVER_STATE_RUN :
        server_listen();
        sv_game_mainTick();
        break;
    case SERVER_STATE_DONE:
        // удалить оставшихся игроков
        server_clients_delete();
        net_socket_close(server.ns);
        server.ns = NULL;
        //закроем карту
        map_clear();
        server.gamestate.state = GAMESTATE_1_NOGAME;
        server.state = SERVER_STATE_IDLE;
        break;
    }
}
