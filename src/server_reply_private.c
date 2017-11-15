/*
 * server_reply.c
 *
 *  Created on: 3 нояб. 2017 г.
 *      Author: mastersan
 */

#include "common/common_list2.h"
#include "server_private.h"

static void server_reply_send(server_client_t * client, const server_reply_t * req)
{
    if(client->tx_queue_num >= SERVER_CLIENT_TX_QUEUE_SIZE)
    {
        game_console_send("SERVER: TX queue overflow.");
        return;
    }
    client->tx_queue[client->tx_queue_num].req = *req;
    client->tx_queue_num++;
}


void server_reply_send_info(server_client_t * client, int clients_num)
{
    server_reply_t reply;
    reply.type = G_SERVER_REPLY_INFO;
    reply.data.INFO.clients_num = clients_num;
    server_reply_send(client, &reply);
}

void server_reply_send_connection_accepted(server_client_t * client)
{
    server_reply_t reply;
    reply.type = G_SERVER_REPLY_CONNECTION_ACCEPTED;
    server_reply_send(client, &reply);
}

void server_reply_send_connection_close(server_client_t * client)
{
    server_reply_t reply;
    reply.type = G_SERVER_REPLY_CONNECTION_CLOSE;
    server_reply_send(client, &reply);
}

void server_reply_send_players_entity_set(server_client_t * client)
{
    server_reply_t reply;
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

void server_reply_send_game_endmap(server_client_t * client, bool win, bool endgame)
{
    server_reply_t reply;
    reply.type = G_SERVER_REPLY_GAME_ENDMAP;
    reply.data.GAME_ENDMAP.win = win;
    reply.data.GAME_ENDMAP.endgame = endgame;
    server_reply_send(client, &reply);
}
