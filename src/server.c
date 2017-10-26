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
#include "game.h"
#include "g_gamesave.h"
#include "map.h"
#include "sound.h"
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

server_state_t sv_state = {};

static int server_run = 0;

server_t server;

fd_set readfds;
fd_set writefds;
fd_set exceptfds;

void server_init(void)
{
    sv_state.custommap = mapList;
    sv_state.gamemap   = mapList;
}

void server_done(void)
{

}

static void server_req_send(server_client_t * client, const game_server_event_t * req)
{
    if(client->tx_queue_num >= SERVER_TX_QUEUE_SIZE)
    {
        game_console_send("SERVER: TX queue overflow.");
        return;
    }
    client->tx_queue[client->tx_queue_num].req = *req;
    client->tx_queue_num++;
}

void server_event_info_send(server_client_t * client)
{
    game_server_event_t event;
    event.type = G_SERVER_EVENT_INFO;
    event.data.INFO.clients_num = 0;
    server_req_send(client, &event);
}

void server_event_cliententity_send(server_client_t * client)
{
    game_server_event_t event;
    event.type = G_SERVER_EVENT_PLAYER_ENTITY_SET;

    int i = 0;
    server_player_t * player;

    event.data.PLAYER_ENTITY_SET.player2 = (client->players_num == 2);
    LIST2_FOREACHR(client->players, player)
    {
        if(i >= client->players_num)
            break;
        strncpy(event.data.PLAYER_ENTITY_SET.ent[i].entityname, player->entity->info->name, GAME_SERVER_EVENT_ENTNAME_SIZE);
        event.data.PLAYER_ENTITY_SET.ent[i].entity = player->entity;

    }
    server_req_send(client, &event);

}

void server_event_gamestate_send(server_client_t * client, gamestate_t state)
{
    sv_state.state = state;
    game_server_event_t event;
    event.type = G_SERVER_EVENT_GAME_STATE_SET;
    event.data.GAME_STATE_SET.state = state;
    server_req_send(client, &event);
}

void host_event_gameload_loaded_send(int flags)
{
    game_server_event_t event;
    event.type = G_SERVER_EVENT_GAME_LOADED;
    event.data.GAME_LOADED.flags = flags;
    server_client_t * client;
    LIST2_FOREACH(server.clients, client)
    {
        server_req_send(client, &event);
    }
}


void server_setgamestate(gamestate_t state)
{
    //sv_state.state = state;
    game_server_event_t event;
    event.type = G_SERVER_EVENT_GAME_STATE_SET;
    event.data.GAME_STATE_SET.state = state;
    server_client_t * client;
    LIST2_FOREACH(server.clients, client)
    {
        server_req_send(client, &event);
    }
}


/*
 * получить данные переменной
 */
vardata_t * server_client_vardata_get(server_player_t * client, const char * varname, vartype_t vartype)
{
    static const char * list[] =
    {
            "INTEGER",
            "FLOAT",
            "STRING",
    };
    var_t * var = var_find(client->vars, varname);
    if(!var)
    {
        var = var_create(&client->vars, varname, vartype);
    }
    vardata_t * vardata = var->data;
    if( (int)vartype >= 0 && vardata->type != vartype )
    {
        game_console_send("Warning: Host client variable \"%s\" has type %s, but used as %s.", varname, list[vardata->type], list[vartype]);
    }
    return vardata;
}

/**
 * @brief сохранение информации о клиенте
 */
static void server_client_player_info_store(server_player_t * player)
{
    if(player->entity->info->client_store)
        player->userstoredata = (*player->entity->info->client_store)(
                player->entity->data
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

/*
 * восстановление информации о entity клиента при переходе на следующий уровень и при чтении gamesave
 */
static void server_client_player_info_restore(server_player_t * player)
{
    if(player->userstoredata)
    {
        player->entity->info->client_restore(
            player->entity,
            player->entity->data,
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

static server_client_t * server_client_create(int sock, const net_addr_t * net_addr, bool main)
{
    server_client_t * client = Z_malloc(sizeof(server_client_t));
    if(!client)
        game_halt("server_client_create(): Can not alloc memory, failed");

    client->main = main;
    client->ns.sock = sock;
    client->ns.addr_ = *net_addr;
    client->players_num = 0;
    client->players = NULL;
    client->tx_queue_num = 0;
    LIST2_PUSH(server.clients, client);
    return client;
}


static void server_client_delete(server_client_t * client)
{
    LIST2_UNLINK(server.clients, client);
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
    while(!LIST2_IS_EMPTY(server.clients))
    {
        server_client_delete(server.clients);
    }
}

static void server_client_disconnect(server_client_t * client)
{
    game_server_event_t event;
    event.type = G_SERVER_EVENT_CONNECTION_CLOSE;
    server_req_send(client, &event);
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
    return 0;
}

/**
 * восстановление информации о клиенте
 */
/**
 * @description убирание клиентов из игры (не дисконект!), сохранение информации о клиентах
 */
void server_unjoin_clients(void)
{
    server_client_t * client;
    server_player_t * player;
    LIST2_FOREACHR(server.clients, client)
    {
        LIST2_FOREACHR(client->players, player)
                {
            server_client_player_info_store(player);
            player->entity = NULL;
                }
    }
}

void server_start(int flags)
{
    server_run = 1;
    sv_state.flags = flags;
    sv_state.state = GAMESTATE_MISSION_BRIEF;
    sv_state.paused = false;

    server.ns = net_socket_create(NET_PORT, "127.0.0.1");

    if(server.ns == NULL)
    {
        game_halt("socket() failed");
    }
    if(net_socket_bind(server.ns) < 0)
    {
        game_halt("server bind() failed");
    }

    sv_state.allow_state_gamesave = true;

}

void server_stop(void)
{
    server_run = 0;
}


static int sv_gamesave_load(int isave)
{
    /* игра уже создана */
    gamesave_load_context_t ctx;
    if(g_gamesave_load_open(isave, &ctx))
        return -1;

    //прочитаем карту
    if(map_load(ctx.mapfilename))
    {
        game_console_send("Error: Could not load map \"%s\".", ctx.mapfilename);
        //game_abort();

        g_gamesave_load_close(&ctx);
        return -1;
    }

    sv_state.flags = ctx.flags;

    host_event_gameload_loaded_send(ctx.flags);

    g_gamesave_load_close(&ctx);

    sv_state.allow_state_gamesave = false;
    return 0;
}



void server_client_control_handle(server_player_t * player, const game_client_player_request_t * req)
{
    /* TODO
	game_console_send("server: from 0x%00000000x:%d received player action %s.", sender.addr_in.sin_addr, ntohs(sender.addr_in.sin_port),
		req.control.action
	);
     */
    game_console_send("server: received player action %s.", req->data.CONTROL.action
    );

    /* execute action */
    entity_t * ent = player->entity;
    if(ent)
    {
        bool found = false;

        const entityinfo_t * info = ent->info;

        for(size_t i = 0; i < info->actions_num; i++)
        {
            entityaction_t * action = &info->actions[i];
            if(!strncmp(action->action, req->data.CONTROL.action, GAME_CLIENT_PLAYER_REQUEST_CONTROL_ACTION_SIZE))
            {
                found = true;
                if(ent->spawned)
                {
                    switch(sv_state.state)
                    {
                    case GAMESTATE_NOGAME:
                        break;
                    case GAMESTATE_MISSION_BRIEF:
                        break;
                    case GAMESTATE_GAMESAVE:
                        break;
                    case GAMESTATE_INGAME:
                        if(action->action_f)
                            action->action_f(ent, ent->data, action->action);
                        break;
                    case GAMESTATE_INTERMISSION:
                        break;
                    }
                }
                else
                {
                    if(info->spawn)
                    {
                        if(sv_state.flags & GAMEFLAG_2PLAYERS)
                        {
                            game_console_send("server: spawn client.");
                            server_client_player_info_restore(player);
                            info->spawn(ent, ent->data);
                            ent->spawned = true;
                            ent->alive = true;
                        }
                    }
                }
                break;
            }
        }
        if(!found)
        {
            game_console_send("server: unknown action :%d.", req->data.CONTROL.action);
        }
    }

}

static void server_fsm(const net_addr_t * sender, const game_client_request_t * req)
{
    switch(req->type)
    {
    /** Непривилегированые запросы */
    case G_CLIENT_REQ_DISCOVERYSERVER:
        /*
        host_event_info_send(ns);
         */
        break;
    case G_CLIENT_REQ_CONNECT:
    {


        game_console_send("server: client request connection from 0x%00000000x:%d.", sender->addr_in.sin_addr, ntohs(sender->addr_in.sin_port));
        bool mainclient = true;
        //net_socket_t * ns = net_socket_create_sockaddr(sender.addr);
        server_client_t * client = server_client_create(server.ns->sock, sender, mainclient);

        game_server_event_t event;
        event.type = G_SERVER_EVENT_CONNECTION_ACCEPTED;
        server_req_send(client, &event);

        server_event_gamestate_send(client, sv_state.state);

        break;
    }
    case G_CLIENT_REQ_DISCONNECT:
    {
        game_console_send("server: client request disconnection from 0x%00000000x:%d.", sender->addr_in.sin_addr, ntohs(sender->addr_in.sin_port));
        server_client_t * client = server_client_find_by_addr(sender);

        server_client_delete(client);

        break;
    }
    case G_CLIENT_REQ_JOIN:
    {
        server_client_t * client = server_client_find_by_addr(sender);
        if(client->players_num > 0)
        {
            game_console_send("server: client already joined.");
            break;
        }

        if(server_client_join(client, req->data.JOIN.players_num) != 0)
        {
            game_console_send("server: can not join client, no entities to spawn.");
            break;
        }
        server_event_cliententity_send(client);
        game_console_send("server: client joined to game.");
        break;
    }
    /** Привилегированные запросы */
    case G_CLIENT_REQ_GAME_ABORT:
    {
        server_client_t * client = server_client_find_by_addr(sender);
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
    }
    case G_CLIENT_REQ_GAME_SETMAP:
    {
        const char * mapname = req->data.GAME_SETMAP.mapname;
        sv_state.gamemap = map_find(mapname);
        if(!sv_state.gamemap)
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
        switch(sv_state.state)
        {
        case GAMESTATE_NOGAME:
            break;
        case GAMESTATE_MISSION_BRIEF:
            if(sv_state.allow_state_gamesave)
                sv_state.state = GAMESTATE_GAMESAVE;
            else
                sv_state.state = GAMESTATE_INGAME;
            break;
        case GAMESTATE_GAMESAVE:
            sv_state.state = GAMESTATE_INGAME;
            break;
        case GAMESTATE_INGAME:
            break;
        case GAMESTATE_INTERMISSION:
            sv_state.state = GAMESTATE_MISSION_BRIEF;
            break;
        }
        break;
    }
    case G_CLIENT_REQ_GAME_SAVE:
        g_gamesave_save(req->data.GAME_SAVE.isave);
        break;
    case G_CLIENT_REQ_GAME_LOAD:
        sv_gamesave_load(req->data.GAME_LOAD.isave);
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
        {
            server_client_t * client = server_client_find_by_addr(sender);
            if(!client)
            {
                game_console_send("SERVER: client 0x%00000000x:%d not found.", sender->addr_in.sin_addr, ntohs(sender->addr_in.sin_port));
            }
            else
            {
                game_console_send("SERVER: client 0x%00000000x:%d require disconnection.", sender->addr_in.sin_addr, ntohs(sender->addr_in.sin_port));
                server_client_disconnect(client);
            }
            break;
        }
        case G_CLIENT_REQ_JOIN:
            PDU_POP_BUF(&value16, sizeof(value16));
            client_req.data.JOIN.players_num = ntohs(value16);
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

    game_client_player_request_t client_player_req;

    server_client_t * client = server_client_find_by_addr(sender);
    if(!client)
    {
        return 0;
    }
    int i = 0;
    server_player_t * player;
    LIST2_FOREACHR(client->players, player)
    {
        if(i >= client->players_num)
            break;
        PDU_POP_BUF(&value16, sizeof(value16));
        client_player_req.type = ntohs(value16);
        switch(client_player_req.type)
        {
        case G_CLIENT_PLAYER_REQ_NONE:
            break;
        case G_CLIENT_PLAYER_REQ_CONTROL:
            PDU_POP_BUF(&value16, sizeof(value16));
            size_t sv_client_player_events_num = htons(value16);

            for(size_t i = 0; i < sv_client_player_events_num; i++)
            {
                PDU_POP_BUF(client_player_req.data.CONTROL.action, GAME_CLIENT_PLAYER_REQUEST_CONTROL_ACTION_SIZE);
                server_client_control_handle(player, &client_player_req);
            }
            break;
        }
        i++;
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
        case G_SERVER_EVENT_CONNECTION_CLOSE:
            break;
        case G_SERVER_EVENT_GAME_STATE_SET:
            value16 = htons(event->data.GAME_STATE_SET.state);
            PDU_PUSH_BUF(&value16, sizeof(value16));
            break;
        case G_SERVER_EVENT_GAME_LOADED:
            value16 = htons(event->data.GAME_LOADED.flags);
            PDU_PUSH_BUF(&value16, sizeof(value16));
            break;
        case G_SERVER_EVENT_PLAYER_ENTITY_SET:
        {
            value16 = htons(event->data.PLAYER_ENTITY_SET.player2);
            PDU_PUSH_BUF(&value16, sizeof(value16));
            int player_num = event->data.PLAYER_ENTITY_SET.player2 ? 2 : 1;
            for(int i = 0; i < player_num; i++)
            {
                PDU_PUSH_BUF(event->data.PLAYER_ENTITY_SET.ent[i].entityname, GAME_SERVER_EVENT_ENTNAME_SIZE);
                PDU_PUSH_BUF(&event->data.PLAYER_ENTITY_SET.ent[i].entity, sizeof(event->data.PLAYER_ENTITY_SET.ent[i].entity));
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

void server_handle()
{
    if(!server_run)
        return;

    server_listen();

    if(!server_run)
    {
        // удалить оставшихся игроков
        server_clients_delete();
        net_socket_close(server.ns);
        server.ns = NULL;
        //закроем карту
        map_clear();
        sv_state.state = GAMESTATE_NOGAME;
    }

    sv_game_mainTick();

}
