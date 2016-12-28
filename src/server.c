/*
 * server.c
 *
 *  Created on: 27 дек. 2016 г.
 *      Author: mastersan
 */

#include "server.h"
#include "game.h"
#include "map.h"

#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>

#include <sys/ioctl.h>

static int server_run = 0;

fd_set readfds;
fd_set writefds;
fd_set exceptfds;

static client_t ** clients;
static size_t clients_size = 0;
static size_t clients_num = 0;

/**
 * подключение игрока к игре
 * @return id
 * @return -1 ошибка
 */
int server_client_connect()
{
	client_t ** tmp;

	if(clients_size < clients_num + 1)
	{
		if(clients_size == 0) clients_size = 1;
		else clients_size *= 2;
		tmp = Z_realloc(clients, sizeof(client_t*) * clients_size);
		if(!tmp)
			game_halt("client_connect(): Can not alloc memory, failed");
		clients = tmp;
	}

	client_t * client = Z_malloc(sizeof(client_t));
	client->entity = NULL;
	client->userstoredata = NULL;



	client->fd = socket(PF_LOCAL, SOCK_STREAM, 0);

	clients[clients_num] = client;
	clients_num++;

	return clients_num - 1;
}

void server_disconnect_clients()
{
	while(clients_num > 0)
	{
		clients_num--;
		close(clients[clients_num]->fd);
		server_unspawn_client(clients_num);
		Z_free(clients[clients_num]->userstoredata);
		Z_free(clients[clients_num]);
	}
}

int server_client_num_get()
{
	return clients_num;
}

client_t * server_client_get(int id)
{
	if(id < 0 || id >= clients_num) return NULL;
	return clients[id];
}

int server_spawn_client(int id)
{
	if(id < 0 || id >= clients_num)
	{
		game_console_send("Error: Could not spawn client: unknown client id %d.", id);
		return -1;
	}

	entity_t * entity = entries_client_spawn();

	if(entity == NULL)
	{
		game_console_send("Error: No entity to spawn client.");
		return -1;
	}

	clients[id]->entity = entity;
	return 0;
}

void server_unspawn_client(int id)
{
	if(id < 0 || id >= clients_num) return;
	//mobj_free(clients[id]->mobj);

	clients[id]->entity = NULL;
}

/**
 * сохранение информации о клиентах между уровнями
 */
void server_store_clients_info()
{
	int id;
	for(id = 0; id < clients_num; id++)
	{
		client_t * client = clients[id];
		if(client->entity->info == NULL) continue;
		client->userstoredata = client->entity->info->client_store(
			&(client->storedata),
			client->entity->data
		);
	}
}

/**
 * восстановление информации о клиентах
 */
void server_restore_clients_info()
{
	int id;
	for(id = 0; id < clients_num; id++)
	{
		client_t * client = clients[id];
		if(client->entity->info == NULL) continue;
		client->entity->info->client_restore(
			client->entity->data,
			&(client->storedata),
			client->userstoredata
		);
		Z_free(client->userstoredata);
		client->userstoredata = NULL;
	}
}

static void server_fd_set()
{
	FD_ZERO(&readfds);
	FD_ZERO(&writefds);
	FD_ZERO(&exceptfds);

	int id;
	for(id = 0; id < clients_num; id++)
	{
		FD_SET(clients[id]->fd, &readfds);
	}
}

static void server_events_pump()
{

	int id;

	static char buf[1000];

	struct timeval timeout;
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;

	int ret = select(FD_SETSIZE, &readfds, &writefds, &exceptfds, &timeout);


	switch(ret)
	{
		case 0:
			printf("timeout\n");
			break;
		case -1:
			perror("select");
			exit(1);
		default:

			for(id = 0; id < clients_num; id++)
			{
				int fd = clients[id]->fd;
				if (FD_ISSET(fd, &readfds))
				{
					int nread;
					ioctl(fd, FIONREAD, &nread);
					if (nread == 0)
					{
						//printf("done\n");
					}
					else
					{
						ssize_t count = read(fd, buf, nread);
						buf[count] = 0;
						//game_console_send("Event: server read event sended by client %d.", id);
						printf("client %d: read %ld: %s\n", id, count, buf);
					}
				}

			}
			break;
	}



}

void server_start()
{
	server_run = 1;
}

void server_stop()
{
	server_run = 0;
}

void server()
{
	if(!server_run)
		return;

	server_fd_set();

	server_events_pump();

}


