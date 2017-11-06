/*
 * server_reply.c
 *
 *  Created on: 3 нояб. 2017 г.
 *      Author: mastersan
 */

#include "common/common_list2.h"
#include "server_private.h"

static void server_reply_send(server_client_t * client, const game_server_reply_t * req)
{
    if(client->tx_queue_num >= SERVER_CLIENT_TX_QUEUE_SIZE)
    {
        game_console_send("SERVER: TX queue overflow.");
        return;
    }
    client->tx_queue[client->tx_queue_num].req = *req;
    client->tx_queue_num++;
}


void server_reply_send_info(server_client_t * client)
{
    game_server_reply_t reply;
    reply.type = G_SERVER_REPLY_INFO;
    reply.data.INFO.clients_num = 0;
    server_reply_send(client, &reply);
}

void server_reply_send_connection_accepted(server_client_t * client)
{
    game_server_reply_t reply;
    reply.type = G_SERVER_REPLY_CONNECTION_ACCEPTED;
    server_reply_send(client, &reply);
}

void server_reply_send_connection_close(server_client_t * client)
{
    game_server_reply_t reply;
    reply.type = G_SERVER_REPLY_CONNECTION_CLOSE;
    server_reply_send(client, &reply);
}

void server_reply_send_players_entity_set(server_client_t * client)
{
    game_server_reply_t reply;
    reply.type = G_SERVER_REPLY_PLAYERS_ENTITY_SET;
    reply.data.PLAYERS_ENTITY_SET.players_num = client->players_num;
    int i = 0;
    server_player_t * player;

    LIST2_FOREACHR(client->players, player)
    {
        if(i >= client->players_num)
            break;
        strncpy(reply.data.PLAYERS_ENTITY_SET.ent[i].entityname, player->entity->info->name, GAME_SERVER_EVENT_ENTNAME_SIZE);
        reply.data.PLAYERS_ENTITY_SET.ent[i].entity = player->entity;
        i++;
    }
    server_reply_send(client, &reply);

}

