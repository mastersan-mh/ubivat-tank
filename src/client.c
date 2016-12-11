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
 * @return id
 * @return -1 ошибка
 */
int client_connect()
{
	client_t ** tmp;

	if(clients_size < clients_num + 1)
	{
		if(clients_size == 0) clients_size = 1;
		else clients_size *= 2;
		tmp = Z_realloc(clients, sizeof(client_t*) * clients_size);
		if(!tmp)game_halt("mobj_register(): failed");
		clients = tmp;
	}

	client_t * client = Z_malloc(sizeof(client_t));
	client->mobj = NULL;
	client->userstoredata = NULL;
	clients[clients_num] = client;
	clients_num++;

	return clients_num - 1;
}

void client_disconnect_all()
{
	while(clients_num > 0)
	{
		clients_num--;
		client_unspawn(clients_num);
		Z_free(clients[clients_num]->userstoredata);
		Z_free(clients[clients_num]);
	}
}

int client_num_get()
{
	return clients_num;
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

	mobj_t * player = mobj_new(
		MOBJ_PLAYER,
		spawn->pos.x,
		spawn->pos.y,
		spawn->dir,
		spawn,
		NULL
	);

	clients[id]->mobj = player;
}

void client_unspawn(int id)
{
	if(id < 0 || id >= clients_num) return;
	//mobj_free(clients[id]->mobj);

	clients[id]->mobj = NULL;
}

/**
 * сохранение информации о клиентах между уровнями
 */
void client_store_all()
{
	int id;
	for(id = 0; id < clients_num; id++)
	{
		client_t * client = clients[id];
		if(client->mobj->info == NULL) continue;
		client->userstoredata = client->mobj->info->client_store(
			&(client->storedata),
			client->mobj->data
		);
	}
}

/**
 * восстановление информации о клиентах
 */
void client_restore_all()
{
	int id;
	for(id = 0; id < clients_num; id++)
	{
		client_t * client = clients[id];
		if(client->mobj->info == NULL) continue;
		client->mobj->info->client_restore(
			client->mobj->data,
			&(client->storedata),
			client->userstoredata
		);
		Z_free(client->userstoredata);
		client->userstoredata = NULL;
	}
}
