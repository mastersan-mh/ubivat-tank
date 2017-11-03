/*
 * server_reply.c
 *
 *  Created on: 3 нояб. 2017 г.
 *      Author: mastersan
 */

#include "common/common_list2.h"
#include "server_private.h"

void server_reply_send(server_client_t * client, const game_server_event_t * req)
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
