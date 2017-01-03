/*
 * client.c
 *
 *  Created on: 8 дек. 2016 г.
 *      Author: mastersan
 */

#include "net.h"
#include "client.h"
#include "g_events.h"

#include "game.h"

#include "Z_mem.h"

#include <string.h>

static client_t ** clients;
static size_t clients_size = 0;
static size_t clients_num = 0;

void client_event_send(client_t * client, gevent_t * event)
{
/* TODO: this function*/

	size_t buflen = 0;
	size_t len;
	char buf[sizeof(gevent_t)];
	memset(buf, 0, sizeof(buf));

	switch(event->type)
	{
		case GEVENT_CLIENT_MSG_CONNECT:
			buf[buflen] = event->type;
			buflen++;
			break;
		case GEVENT_CONTROL:
			buf[buflen] = event->type;
			buflen++;
			len = strnlen(event->control.action, GAME_EVENT_CONTROL_ACTION_LEN) + 1;
			memcpy(&(buf[1]), event->control.action, len);
			buflen += len;
			break;
	}

	net_send(
		client->ns,
		buf,
		buflen
	);

}

/**
 * подключение игрока к игре
 * @return id
 * @return -1 ошибка
 */
int client_connect()
{
	net_socket_t * ns = net_socket_create(NET_PORT, "127.0.0.1");
	if(!ns)
		return -1;
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
	client->ns = ns;
	client->state = CLIENT_AWAITING_CONNECTION;

	client->time = time_current;


	clients[clients_num] = client;
	clients_num++;


	gevent_t event;
	event.type = GEVENT_CLIENT_MSG_CONNECT;
	client_event_send(client, &event);

	return clients_num - 1;
}

void client_connection_close(int i)
{
	/* TODO */
}


static void client_listen_clients()
{
	size_t i;
	struct sockaddr addr;
	socklen_t addr_len = 0;
	size_t buf_size;
	char * buf = NULL;

	for(i = 0; i < clients_num; i++)
	{
		client_t * client = clients[i];

		buf = net_recv(client->ns, &buf_size, &addr, &addr_len);
		switch(client->state)
		{
			case CLIENT_AWAITING_CONNECTION:
				if(!buf)
				{
					if(time_current - client->time > CLIENT_TIMEOUT)
					{
						client_connection_close(i);
					}
					break;
				}

				client->time = time_current;

				if(buf[0] == SERVER_MSG_CONNECTION_ACCEPTED)
				{
					game_console_send("client: server accept connection to 0x%00000000x:%d.", client->ns->addr_in.sin_addr, ntohs(client->ns->addr_in.sin_port));

					client->state = CLIENT_LISTEN;
				}

				break;
			case CLIENT_LISTEN:
				client->time = time_current;
				if(!buf)
					break;

				break;


		}
		net_recv_free(buf);

	}
}

void client_event_control_send(int clientId, const char * action_name)
{
	gevent_t event;
	event.type = GEVENT_CONTROL;
	strncpy(event.control.action, action_name, GAME_EVENT_CONTROL_ACTION_LEN);
	client_event_send(clients[clientId], &event);
}



void client()
{

	client_listen_clients();

}

