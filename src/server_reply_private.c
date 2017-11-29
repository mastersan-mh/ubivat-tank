/*
 * server_reply.c
 *
 *  Created on: 3 нояб. 2017 г.
 *      Author: mastersan
 */

#include "common/common_list2.h"
#include "server_private.h"

#include "common/common_queue.h"

static void server_reply_client_send(server_client_t * client, const server_reply_t * req)
{
    if(client->tx_queue_num >= SERVER_CLIENT_TX_QUEUE_SIZE)
    {
        game_console_send("SERVER: TX queue overflow.");
        return;
    }
    client->tx_queue[client->tx_queue_num].reply = *req;
    client->tx_queue_num++;
}

static void server_reply_send(const net_addr_t * net_addr, const server_reply_t * reply)
{
    server_tx_t * tx = Z_malloc(sizeof(server_tx_t));
    tx->net_addr = *net_addr;
    tx->reply = *reply;
    CIRCLEQ_INSERT_TAIL(&server.txs, tx, queue);
}

void server_reply_send_info(const net_addr_t * net_addr)
{
    server_reply_t reply;
    reply.type = G_SERVER_REPLY_INFO;
    reply.data.INFO.clients_num = server_clients_num_get();
    server_reply_send(net_addr, &reply);
}

void server_reply_send_connection_accepted(server_client_t * client)
{
    server_reply_t reply;
    reply.type = G_SERVER_REPLY_CONNECTION_ACCEPTED;
    server_reply_client_send(client, &reply);
}

void server_reply_send_connection_close(server_client_t * client)
{
    server_reply_t reply;
    reply.type = G_SERVER_REPLY_CONNECTION_CLOSE;
    server_reply_client_send(client, &reply);
}

void server_reply_send_game_nextmap(server_client_t * client, bool win, const char * mapfilename)
{
    server_reply_t reply;
    reply.type = G_SERVER_REPLY_GAME_NEXTMAP;
    reply.data.GAME_NEXTMAP.win = win;
    reply.data.GAME_NEXTMAP.endgame = (mapfilename == NULL);
    strncpy(reply.data.GAME_NEXTMAP.mapfilename, mapfilename, MAP_FILENAME_SIZE);
    server_reply_client_send(client, &reply);
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
        reply.data.PLAYERS_ENTITY_SET.ent[i].entityId = player->entity->id;
        i++;
    }
    server_reply_client_send(client, &reply);

}
