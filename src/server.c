/*
 * server.c
 *
 *  Created on: 27 дек. 2016 г.
 *      Author: mastersan
 */

#include "common_list2.h"
#include "g_events.h"
#include "sv_game.h"
#include "net.h"
#include "server.h"
#include "game.h"
#include "map.h"
#include "sound.h"
#include "menu.h"

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

fd_set readfds;
fd_set writefds;
fd_set exceptfds;

static host_client_t * clients;
static int clients_num = 0;

void server_init()
{
	sv_state.custommap = mapList;
	sv_state.gamemap   = mapList;
}

void server_done()
{

}

static void host_client_unspawn(host_client_t * client)
{
	client->entity = NULL;
}

void host_event_send(const host_client_t * client, const ghostevent_t * event)
{
	size_t buflen = 0;
	char buf[sizeof(ghostevent_t)];
	memset(buf, 0, sizeof(buf));
	buf[buflen] = event->type;
	buflen++;

	switch(event->type)
	{
		case GHOSTEVENT_CONNECTION_ACCEPTED:
			break;
		case GHOSTEVENT_CONNECTION_CLOSE:
			break;
		case GHOSTEVENT_CONNECTION_SETPLAYERENTITY:
			strncpy(&buf[buflen], event->setplayerentity.entityname, GAME_HOSTEVENT_ENTNAME_LEN);
			buflen += GAME_HOSTEVENT_ENTNAME_LEN+1;
			size_t size = sizeof(entity_t*);
			memcpy(&buf[buflen], &event->setplayerentity.entity, size);
			buflen += size;
			break;
		case GHOSTEVENT_GAMEWIN:
			break;
		case GHOSTEVENT_GAMESTATE:
			buf[buflen] = event->gamestate.state;
			buflen++;
			break;
		case GHOSTEVENT_IMENU:
			buf[buflen] = event->imenu.imenu;
			buflen++;
			break;
	}

	net_send(
		client->ns,
		buf,
		buflen
	);
}

void host_setcliententity(host_client_t * client)
{
	ghostevent_t hevent;
	hevent.type = GHOSTEVENT_CONNECTION_SETPLAYERENTITY;
	strncpy(hevent.setplayerentity.entityname, client->entity->info->name, GAME_HOSTEVENT_ENTNAME_LEN);
	hevent.setplayerentity.entity = client->entity;
	host_event_send(client, &hevent);
}

void host_event_send_win()
{
	host_client_t * client;
	ghostevent_t hevent;
	hevent.type = GHOSTEVENT_GAMEWIN;
	LIST2_FOREACH(clients, client)
		host_event_send(client, &hevent);
}

void host_setgamestate(gamestate_t state)
{
	sv_state.state = state;
			host_client_t * client;
	ghostevent_t hevent;
	hevent.type = GHOSTEVENT_GAMESTATE;
	hevent.gamestate.state = state;
	LIST2_FOREACH(clients, client)
		host_event_send(client, &hevent);

}

void host_event_send_imenu(menu_selector_t imenu)
{
	/* послать только серверу (первый игрок) */
	ghostevent_t hevent;
	hevent.type = GHOSTEVENT_IMENU;
	hevent.imenu.imenu = imenu;
	host_client_t * client;
	LIST2_FOREACH(clients, client)
	{
		if(!client->next)
			host_event_send(client, &hevent);
	}
}

/**
 * подключение игрока к игре
 * @return id
 * @return -1 ошибка
 */
static host_client_t * host_client_create(const net_socket_t * ns)
{
	host_client_t * client = Z_malloc(sizeof(host_client_t));
	if(!client)
		game_halt("server_client_create(): Can not alloc memory, failed");

	client->entity = NULL;
	client->userstoredata = NULL;
	client->ns = (net_socket_t *) ns;

	LIST2_PUSH(clients, client);

	clients_num++;

	return client;
}

static void host_client_delete(host_client_t * client)
{
	host_client_unspawn(client);
	net_socket_close(client->ns);
	Z_free(client->userstoredata);
	Z_free(client);
}

static void host_client_disconnect(host_client_t * client)
{
	ghostevent_t event;
	event.type = GHOSTEVENT_CONNECTION_CLOSE;
	host_event_send(client, &event);

	LIST2_UNLINK(clients, client);

	host_client_delete(client);
}

void host_clients_disconnect()
{
	ghostevent_t event;
	event.type = GHOSTEVENT_CONNECTION_CLOSE;
	host_client_t * client;

	while(clients)
	{
		client = clients;
		clients = clients->next;
		host_event_send(client, &event);
		host_client_delete(client);
		clients_num--;
	}
}

static host_client_t * host_client_find_by_addr(const struct sockaddr * addr)
{
	host_client_t * client;
	LIST2_FOREACH(clients, client)
	{
		if( !memcmp(&client->ns->addr, addr, sizeof(struct sockaddr)) )
			return client;
	}
	return NULL;
}

int host_client_num_get()
{
	return clients_num;
}

host_client_t * host_client_get(int id)
{
	if(id < 0 || id >= clients_num) return NULL;
	host_client_t * client = clients;
	id = clients_num - 1 - id;
	int i;
	LIST2_LIST_TO_IENT(clients, client, i, id);
	return client;
}

static int host_client_spawn(host_client_t * client)
{
	entity_t * entity = entries_client_spawn();
	if(entity == NULL)
	{
		game_console_send("Error: No entity to spawn client.");
		return -1;
	}
	client->entity = entity;
	return 0;
}

int host_client_spawn_id(int id)
{
	if(id < 0 || id >= clients_num)
	{
		game_console_send("Error: Could not spawn client: unknown client id %d.", id);
		return -1;
	}

	host_client_t * client = clients;
	id = clients_num - 1 - id;
	int i;
	LIST2_LIST_TO_IENT(clients, client, i, id);
	return host_client_spawn(client);
}

void host_client_unspawn_id(int id)
{
	if(id < 0 || id >= clients_num)
		return;
	host_client_t * client = clients;
	id = clients_num - 1 - id;
	int i;
	LIST2_LIST_TO_IENT(clients, client, i, id);
	host_client_unspawn(client);
}

/**
 * сохранение информации о клиентах между уровнями
 */
void server_store_clients_info()
{
	host_client_t * client;
	LIST2_FOREACH(clients, client)
	{
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
	host_client_t * client;
	LIST2_FOREACH(clients, client)
	{
		client->entity->info->client_restore(
			client->entity->data,
			&(client->storedata),
			client->userstoredata
		);
		Z_free(client->userstoredata);
		client->userstoredata = NULL;
	}
}

net_socket_t * host_ns = NULL;

void server_start(int flags)
{
	server_run = 1;
	sv_state.state = GAMESTATE_MISSION_BRIEF;

	sv_state.show_menu = false;
	sv_state.paused = false;

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
	//дисконнект всех игроков
	host_clients_disconnect();

	net_socket_close(host_ns);
	host_ns = NULL;
	server_run = 0;
}



static void server_listen()
{

	union
	{
		struct sockaddr addr;
		struct sockaddr_in addr_in;
	} sender;

	size_t i;
	ghostevent_t hevent;
	gclientevent_t cevent;
	//struct sockaddr sender_addr;

	socklen_t sender_addr_len = sizeof(sender.addr);

	/*
	 * https://www.linux.org.ru/forum/development/10072313
	 */

	// буфур для приема
	char buf[2048];

	size_t maxcontentlength = 512;
	int value = recvfrom(host_ns->sock, buf, maxcontentlength, 0, &sender.addr, &sender_addr_len);
	if(value < 0)
	{
		return;
	}

	host_client_t * client;

	cevent.type = buf[0];
	switch( cevent.type )
	{
		case GCLIENTEVENT_CONNECT:
			game_console_send("server: client request connection from 0x%00000000x:%d.", sender.addr_in.sin_addr, ntohs(sender.addr_in.sin_port));
			net_socket_t * ns = net_socket_create_sockaddr(sender.addr);
			client = host_client_create(ns);
			if(host_client_spawn(client) != 0)
			{
				game_console_send("server: can not spawn client, no entity to spawn");
			}
			else
			{
				hevent.type = GHOSTEVENT_CONNECTION_ACCEPTED;
				host_event_send(client, &hevent);

				host_setcliententity(client);
				host_setgamestate(sv_state.state);

			}
			break;
		case GCLIENTEVENT_DISCONNECT:
			game_console_send("server: client request disconnection from 0x%00000000x:%d.", sender.addr_in.sin_addr, ntohs(sender.addr_in.sin_port));

			break;

		case GCLIENTEVENT_GAMEABORT:
			game_console_send("server: client aborted game.");
			sv_game_abort();
			break;
		case GCLIENTEVENT_CONTROL:
			strncpy(cevent.control.action, &buf[1], GAME_EVENT_CONTROL_ACTION_LEN);
			game_console_send("server: from 0x%00000000x:%d received player action %s.", sender.addr_in.sin_addr, ntohs(sender.addr_in.sin_port),
				cevent.control.action
			);
			client = host_client_find_by_addr(&sender.addr);
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

				for(i = 0; i < info->actions_num; i++)
				{
					entityaction_t * action = &info->actions[i];
					if(!strncmp(action->action, cevent.control.action, GAME_EVENT_CONTROL_ACTION_LEN))
					{
						found = true;
						if(ent->spawned)
						{
							if(sv_state.state == GAMESTATE_INGAME)
							{
								/* func */
								if(action->action_f)
									action->action_f(ent, ent->data, action->action);
							}
							else if(sv_state.state == GAMESTATE_MISSION_BRIEF)
							{
								sound_play_start(SOUND_MENU_ENTER, 1);
								sv_state.state = GAMESTATE_INGAME;
							}
							else if(sv_state.state == GAMESTATE_INTERMISSION)
							{
								sound_play_start(SOUND_MENU_ENTER, 1);
								// *imenu = MENU_GAME_SAVE;
								sv_state.show_menu = true;
							}
						}
						else
						{
							if(info->spawn)
							{
								if(sv_state.flags & GAMEFLAG_2PLAYERS)
								{
									game_console_send("server: spawn client.");
									info->spawn(ent, ent->data);
									ent->spawned = true;
								}
							}
						}
						break;
					}
				}
				if(!found)
				{
					game_console_send("server: unknown action :%d.", cevent.control.action);
				}
			}

			break;

	}


}

void server()
{
	if(!server_run)
		return;

	server_listen();

	sv_game_mainTick();
}
