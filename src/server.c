/*
 * server.c
 *
 *  Created on: 27 дек. 2016 г.
 *      Author: mastersan
 */

#include "g_events.h"
#include "net.h"
#include "server.h"
#include "game.h"
#include "map.h"

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


static int server_run = 0;

fd_set readfds;
fd_set writefds;
fd_set exceptfds;

static host_client_t ** clients;
static size_t clients_size = 0;
static size_t clients_num = 0;

/**
 * подключение игрока к игре
 * @return id
 * @return -1 ошибка
 */
static int server_client_connect(const net_socket_t * ns)
{
	host_client_t ** tmp;

	if(clients_size < clients_num + 1)
	{
		if(clients_size == 0) clients_size = 1;
		else clients_size *= 2;
		tmp = Z_realloc(clients, sizeof(host_client_t*) * clients_size);
		if(!tmp)
			game_halt("server_client_connect(): Can not alloc memory, failed");
		clients = tmp;
	}

	host_client_t * client = Z_malloc(sizeof(host_client_t));
	client->entity = NULL;
	client->userstoredata = NULL;

	client->ns = (net_socket_t *) ns;

	clients[clients_num] = client;
	clients_num++;

	return clients_num - 1;
}

void server_disconnect_clients()
{
	while(clients_num > 0)
	{
		clients_num--;
		server_unspawn_client(clients_num);
		Z_free(clients[clients_num]->userstoredata);
		Z_free(clients[clients_num]);
	}
}

static host_client_t * server_client_find_by_addr(const struct sockaddr * addr)
{
	host_client_t * client;
	size_t i;
	for(i = 0; i < clients_num; i++)
	{
		client = clients[i];
		if( !memcmp(&client->ns->addr, addr, sizeof(struct sockaddr)) )
			return client;
		clients_num++;
	}
	return NULL;
}

int server_client_num_get()
{
	return clients_num;
}

host_client_t * server_client_get(int id)
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
		host_client_t * client = clients[id];
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
		host_client_t * client = clients[id];
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
	/*
	FD_ZERO(&readfds);
	FD_ZERO(&writefds);
	FD_ZERO(&exceptfds);

	int id;
	for(id = 0; id < clients_num; id++)
	{
		FD_SET(clients[id]->fd, &readfds);
	}
	*/
}

static void server_events_pump()
{
/*
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

*/

}


net_socket_t * host_ns = NULL;

void server_start()
{
	server_run = 1;

	host_ns = net_socket_create(NET_PORT, "127.0.0.1");

	if(host_ns == NULL)
	{
		game_halt("socket() failed");
	}

	if(bind(host_ns->sock, &host_ns->addr, sizeof(host_ns->addr)) < 0)
	{
		game_halt("bind");
	}

}

void server_stop()
{
	net_socket_close(host_ns);
	host_ns = NULL;
	server_run = 0;
}



void server()
{
	if(!server_run)
		return;

	union
	{
		struct sockaddr addr;
		struct sockaddr_in addr_in;
	} sender;

	size_t i;
	gevent_t event;
	//struct sockaddr sender_addr;

	socklen_t sender_addr_len = sizeof(sender.addr);

	/*
	 * https://www.linux.org.ru/forum/development/10072313
	 */

	char buf[2048];       // буфур для приема


	size_t maxcontentlength = 512;
	int value = recvfrom(host_ns->sock, buf, maxcontentlength, 0, &sender.addr, &sender_addr_len);
	if(value < 0)
	{
		return;
	}

	event.type = buf[0];
	switch( event.type )
	{
		case GEVENT_CLIENT_MSG_CONNECT:
			game_console_send("server: client request connection from 0x%00000000x:%d.", sender.addr_in.sin_addr, ntohs(sender.addr_in.sin_port));
			net_socket_t * ns = net_socket_create_sockaddr(sender.addr);
			int id = server_client_connect(ns);
			char reply = SERVER_MSG_CONNECTION_ACCEPTED;
			size_t size = 1;
			net_send(ns, &reply, size);
			server_spawn_client(id);
			break;
		case GEVENT_CONTROL:
			strncpy(event.control.action, &buf[1], GAME_EVENT_CONTROL_ACTION_LEN);
			game_console_send("server: from 0x%00000000x:%d received player action %s.", sender.addr_in.sin_addr, ntohs(sender.addr_in.sin_port),
				event.control.action
				);


			host_client_t * client = server_client_find_by_addr(&sender.addr);
			if(!client)
			{
				game_console_send("server: no client 0x%00000000x:%d.", sender.addr_in.sin_addr, ntohs(sender.addr_in.sin_port));
				break;
			}

			/* execute action */
			{
				bool found = false;
				entity_t * ent = client->entity;
				const entityinfo_t * info = ent->info;

				for(i = 0; i< info->actions_num; i++)
				{
					entityaction_t * action = &info->actions[i];
					if(!strncmp(action->action, event.control.action, GAME_EVENT_CONTROL_ACTION_LEN))
					{
						found = true;
						if(action->action_f)
							action->action_f(ent, ent->data, action->action);
						break;
					}
				}
				if(!found)
				{
					game_console_send("server: unknown action :%d.", event.control.action);
				}
			}

			break;


	}


}


