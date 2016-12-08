/*
 * client.c
 *
 *  Created on: 8 дек. 2016 г.
 *      Author: mastersan
 */

#include "client.h"
#include "game.h"
#include "map.h"
#include "player.h"

static client_t ** clients;
static size_t clients_size = 0;
static size_t clients_num = 0;

/**
 * подключение игрока к игре
 * @return clientId
 * @return -1 ошибка
 */
int client_connect()
{
	client_t ** tmp;

	if(clients_size < clients_num + 1)
	{
		if(clients_size == 0) clients_size = 1;
		else clients_size *= 2;
		tmp = Z_realloc(clients, clients_size);
		if(!tmp)game_halt("mobj_register(): failed");
		clients = tmp;
	}

	client_t * client = Z_malloc(sizeof(client_t));
	client->mobj = NULL;
	clients[clients_num] = client;
	clients_num++;

	return clients_num - 1;
}

void client_disconnect_all()
{
	for( ; clients_num-- > 0; )
	{
		client_unspawn(clients_num);
		Z_free(clients[clients_num]);
	}
}

client_t * client_get(int id)
{
	if(id < 0 || id >= clients_num) return NULL;
	return clients[id];
}

void client_spawn(int id)
{
	if(id < 0 || id >= clients_num) return;
	mobj_t * spawn = player_spawn_get();
	mobj_t * player = player_create_player(spawn);
	clients[id]->mobj = player;

	if(id == 0)
	{
		game.P0 = clients[id]->mobj;
	}
	else if(id == 1)
	{
		game.P1 = clients[id]->mobj;
	}
}

void client_unspawn(int id)
{
	if(id < 0 || id >= clients_num) return;
	//mobj_free(clients[id]->mobj);

	clients[id]->mobj = NULL;

	if(id == 0)
	{
		game.P0 = NULL;
	}
	else if(id == 1)
	{
		game.P1 = NULL;
	}
}
