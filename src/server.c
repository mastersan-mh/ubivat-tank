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
#include "g_gamesave.h"
#include "map.h"
#include "sound.h"
#include "menu.h"
#include "entity.h"

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

host_client_t * hclients = NULL;
static int clients_num = 0;

void server_init(void)
{
	sv_state.custommap = mapList;
	sv_state.gamemap   = mapList;
}

void server_done(void)
{

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
		case GHOSTEVENT_SETPLAYERENTITY:
			strncpy(&buf[buflen], event->setplayerentity.entityname, GAME_HOSTEVENT_ENTNAME_SIZE);
			buflen += GAME_HOSTEVENT_ENTNAME_SIZE;
			size_t size = sizeof(entity_t*);
			memcpy(&buf[buflen], &event->setplayerentity.entity, size);
			buflen += size;
			break;
		case GHOSTEVENT_GAMESTATE:
			buf[buflen++] = event->gamestate.state;
			break;
		case GHOSTEVENT_GAMESAVE_LOADED:
			buf[buflen++] = event->gamesave_loaded.flags;
			break;
	}

	net_send(
		client->ns,
		buf,
		buflen
	);
}

void host_event_cliententity_send(host_client_t * client)
{
	ghostevent_t hevent;
	hevent.type = GHOSTEVENT_SETPLAYERENTITY;
	strncpy(hevent.setplayerentity.entityname, client->entity->info->name, GAME_HOSTEVENT_ENTNAME_SIZE);
	hevent.setplayerentity.entity = client->entity;
	host_event_send(client, &hevent);
}

void host_event_gamestate_send(host_client_t * client, gamestate_t state)
{
	sv_state.state = state;
	ghostevent_t hevent;
	hevent.type = GHOSTEVENT_GAMESTATE;
	hevent.gamestate.state = state;
	host_event_send(client, &hevent);
}

void host_event_gameload_loaded_send(int flags)
{
	ghostevent_t hevent;
	hevent.type = GHOSTEVENT_GAMESAVE_LOADED;
	hevent.gamesave_loaded.flags = flags;
	host_client_t * client;
	LIST2_FOREACH(hclients, client)
	{
		host_event_send(client, &hevent);
	}
}


void host_setgamestate(gamestate_t state)
{
	//sv_state.state = state;
	ghostevent_t hevent;
	hevent.type = GHOSTEVENT_GAMESTATE;
	hevent.gamestate.state = state;
	host_client_t * client;
	LIST2_FOREACH(hclients, client)
	{
		host_event_send(client, &hevent);
	}
}

/**
 * подключение игрока к игре
 * @return id
 * @return -1 ошибка
 */
static void server_client_info_store(host_client_t * client)
{
	if(client->entity->info->client_store)
		client->userstoredata = (*client->entity->info->client_store)(
			client->entity->data
		);

	entity_t * entity = client->entity;
	size_t vars_num = entity->info->vars_num;
	entityvarinfo_t * vars = entity->info->vars;

	client->varsdata_num = vars_num;
	client->varsdata = Z_calloc(vars_num, sizeof(host_clientvardata_t));
	size_t i;
	for(i = 0; i < vars_num; i++)
	{
		entityvardata_t * vardata = entity_vardata_get(entity, vars[i].name, -1);
		strncpy(client->varsdata[i].varname, vars[i].name, _ENTITY_VARNAME_SIZE);
		client->varsdata[i].type    = vardata->type;
		client->varsdata[i].value   = vardata->value;
	}

}

/*
 * восстановление информации о entity клиента при переходе на следующий уровень и при чтении gamesave
 */
static void server_client_info_restore(host_client_t * client)
{
	size_t i;
	if(client->userstoredata)
	{
		(*client->entity->info->client_restore)(
				client->entity,
				client->entity->data,
				client->userstoredata
		);
		Z_free(client->userstoredata);
		client->userstoredata = NULL;
	}

	if(client->varsdata_num > 0)
	{
		entity_t * entity = client->entity;
		size_t vars_num = client->varsdata_num;
		host_clientvardata_t * vars = client->varsdata;
		for(i = 0; i < vars_num; i++)
		{
			entityvardata_t * entityvardata = entity_vardata_get(entity, vars[i].varname, -1);
			if(!entityvardata)
				game_console_send("Error: Can not restore client entity info, entity has no variable \"%s\".", vars[i].varname);
			else
			{
				if(entityvardata->type != vars[i].type)
				{
					game_console_send("Error: Can not restore client entity info, variable \"%s\" has different types.", vars[i].varname);
				}
				else
					entityvardata->value = vars[i].value;
			}
		}
		Z_free(client->varsdata);
	}

}

static host_client_t * host_client_create(const net_socket_t * ns, bool main)
{
	host_client_t * client = Z_malloc(sizeof(host_client_t));
	if(!client)
		game_halt("server_client_create(): Can not alloc memory, failed");

	client->entity = NULL;
	client->userstoredata = NULL;
	client->ns = (net_socket_t *) ns;
	client->main = main;

	LIST2_PUSH(hclients, client);

	clients_num++;

	return client;
}

static void host_client_delete(host_client_t * client)
{
	net_socket_close(client->ns);
	Z_free(client->userstoredata);
	Z_free(client);
}

static void host_client_disconnect(host_client_t * client)
{
	ghostevent_t event;
	event.type = GHOSTEVENT_CONNECTION_CLOSE;
	host_event_send(client, &event);

	LIST2_UNLINK(hclients, client);

	host_client_delete(client);
}

void host_clients_disconnect(void)
{
	ghostevent_t event;
	event.type = GHOSTEVENT_CONNECTION_CLOSE;
	host_client_t * client;

	while(hclients)
	{
		client = hclients;
		hclients = hclients->next;
		host_event_send(client, &event);
		host_client_delete(client);
		clients_num--;
	}
}

static host_client_t * host_client_find_by_addr(const struct sockaddr * addr)
{
	host_client_t * client;
	LIST2_FOREACH(hclients, client)
	{
		if( !memcmp(&client->ns->addr, addr, sizeof(struct sockaddr)) )
			return client;
	}
	return NULL;
}

int host_client_num_get(void)
{
	return clients_num;
}

host_client_t * host_client_get(int id)
{
	if(id < 0 || id >= clients_num) return NULL;
	host_client_t * client = hclients;
	id = clients_num - 1 - id;
	int i;
	LIST2_LIST_TO_IENT(hclients, client, i, id);
	return client;
}

int host_client_join(host_client_t * client)
{
	entity_t * entity = entries_client_join();
	if(entity == NULL)
	{
		game_console_send("Error: No entity to spawn client.");
		return -1;
	}
	client->entity = entity;

	server_client_info_restore(client);

	return 0;
}

/**
 * @description сохранение информации о клиенте
 */
/**
 * восстановление информации о клиенте
 */
/**
 * @description убирание клиентов из игры (не дисконект!), сохранение информации о клиентах
 */
void server_unjoin_clients(void)
{
	host_client_t * client;
	LIST2_FOREACH(hclients, client)
	{
		server_client_info_store(client);
		client->entity = NULL;
	}
}

host_clientvardata_t * sv_client_storedvars_get(host_client_t * client, const char * varname)
{
	size_t i;
	for(i = 0 ;i < client->varsdata_num; i++)
	{
		if(!strncmp(client->varsdata[i].varname, varname, _ENTITY_VARNAME_SIZE))
		{
			return &client->varsdata[i];
			break;
		}
	}
	game_console_send("Error: variable \"%s\" not found in clientvars.", varname);
	return NULL;
}

net_socket_t * host_ns = NULL;

void server_start(int flags)
{
	server_run = 1;
	sv_state.flags = flags;
	sv_state.state = GAMESTATE_MISSION_BRIEF;
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

	sv_state.allow_state_gamesave = true;

}

void server_stop(void)
{
	server_run = 0;
}


static int sv_gamesave_load(int isave)
{
	/* игра уже создана */
	gamesave_load_context_t ctx;
	if(g_gamesave_load_open(isave, &ctx))
		return -1;

	//прочитаем карту
	if(map_load(ctx.mapfilename))
	{
		game_console_send("Error: Could not load map \"%s\".", ctx.mapfilename);
		//game_abort();

		g_gamesave_load_close(&ctx);
		return -1;
	}

	sv_state.flags = ctx.flags;

	host_event_gameload_loaded_send(ctx.flags);

	g_gamesave_load_close(&ctx);

	sv_state.allow_state_gamesave = false;
	return 0;
}



static void server_listen(void)
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
	int value;
	bool mainclient;
	while( (value = recvfrom(host_ns->sock, buf, maxcontentlength, 0, &sender.addr, &sender_addr_len)) > 0 )
	{
		host_client_t * client;
		size_t ofs = 0;
		cevent.type = buf[ofs++];
		switch( cevent.type )
		{
			case GCLIENTEVENT_CONNECT:
			{
				game_console_send("server: client request connection from 0x%00000000x:%d.", sender.addr_in.sin_addr, ntohs(sender.addr_in.sin_port));

				net_socket_t * ns = net_socket_create_sockaddr(sender.addr);
				mainclient = (hclients == NULL);
				client = host_client_create(ns, mainclient);

				hevent.type = GHOSTEVENT_CONNECTION_ACCEPTED;
				host_event_send(client, &hevent);

				host_event_gamestate_send(client, sv_state.state);

				break;
			}
			case GCLIENTEVENT_DISCONNECT:
				game_console_send("server: client request disconnection from 0x%00000000x:%d.", sender.addr_in.sin_addr, ntohs(sender.addr_in.sin_port));

				break;
			case GCLIENTEVENT_JOIN:
			{
				client = host_client_find_by_addr(&sender.addr);
				if(client->entity)
				{
					game_console_send("server: client already joined.");
					break;
				}
				if(host_client_join(client) != 0)
				{
					game_console_send("server: can not join client, no entity to spawn.");
					break;
				}
				host_event_cliententity_send(client);
				game_console_send("server: client joined to game.");
				break;
			}
			case GCLIENTEVENT_CONTROL:
			{
				strncpy(cevent.control.action, &buf[ofs], GAME_EVENT_CONTROL_ACTION_SIZE);
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
				entity_t * ent = client->entity;
				if(ent)
				{
					bool found = false;

					const entityinfo_t * info = ent->info;

					for(i = 0; i < info->actions_num; i++)
					{
						entityaction_t * action = &info->actions[i];
						if(!strncmp(action->action, cevent.control.action, GAME_EVENT_CONTROL_ACTION_SIZE))
						{
							found = true;
							if(ent->spawned)
							{
								switch(sv_state.state)
								{
									case GAMESTATE_NOGAME:
										break;
									case GAMESTATE_MISSION_BRIEF:
										break;
									case GAMESTATE_GAMESAVE:
										break;
									case GAMESTATE_INGAME:
										/* func */
										if(action->action_f)
											(*action->action_f)(ent, ent->data, action->action);
										break;
									case GAMESTATE_INTERMISSION:
										break;
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
			case GCLIENTEVENT_SVCTRL_GAMEABORT:
			{
				client = host_client_find_by_addr(&sender.addr);
				if(!client || !client->main)
					break;
				game_console_send("server: client aborted game.");
				sv_game_abort();
				break;
			}
			case GCLIENTEVENT_SVCTRL_NEXTGAMESTATE:
			{
				client = host_client_find_by_addr(&sender.addr);
				if(!client || !client->main)
					break;

				switch(sv_state.state)
				{
					case GAMESTATE_NOGAME:
						break;
					case GAMESTATE_MISSION_BRIEF:
						if(sv_state.allow_state_gamesave)
							sv_state.state = GAMESTATE_GAMESAVE;
						else
							sv_state.state = GAMESTATE_INGAME;
						break;
					case GAMESTATE_GAMESAVE:
						sv_state.state = GAMESTATE_INGAME;
						break;
					case GAMESTATE_INGAME:
						break;
					case GAMESTATE_INTERMISSION:
						sv_state.state = GAMESTATE_MISSION_BRIEF;
						break;
				}

				break;
			}
			case GCLIENTEVENT_SVCTRL_GAMESAVE_SAVE:
			{
				int isave = buf[ofs++];
				g_gamesave_save(isave);

				break;
			}
			case GCLIENTEVENT_SVCTRL_GAMESAVE_LOAD:
			{
				int isave = buf[ofs++];
				sv_gamesave_load(isave);
				break;
			}
			case GCLIENTEVENT_SVCTRL_SETGAMEMAP:
			{
				char * mapname = &buf[ofs];
				sv_state.gamemap = map_find(mapname);
				if(!sv_state.gamemap)
				{
					game_console_send("Error: Map \"%s\" not found.", mapname);
					//game_abort();
					break;
				}
				if(map_load(mapname))
				{
					game_console_send("Error: Could not load map \"%s\".", mapname);
					//game_abort();
				}
				break;
			}

		}
	}

}

void server()
{
	if(!server_run)
		return;

	server_listen();

	if(!server_run)
	{
		//дисконнект всех игроков
		host_clients_disconnect();
		net_socket_close(host_ns);
		host_ns = NULL;
		//закроем карту
		map_clear();
		sv_state.state = GAMESTATE_NOGAME;
	}

	sv_game_mainTick();

}
