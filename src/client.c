/*
 * client.c
 *
 *  Created on: 8 дек. 2016 г.
 *      Author: mastersan
 */

#include "common/common_list2.h"
#include "net.h"
#include "client.h"
#include "cl_input.h"
#include "cl_game.h"
#include "g_events.h"
#include "game.h"
#include "Z_mem.h"
#include "menu.h"
#include "ui.h"
#include "video.h"
#include "map.h"

#include <string.h>

client_state_t cl_state = {};

client_t * clients = NULL;

void cl_game_init(void)
{
	cl_state.quit = false;

	cl_state.imenu = MENU_MAIN;

	cl_state.msg       = NULL;
	cl_state.state     = GAMESTATE_NOGAME;
	cl_state.show_menu = true;
	cl_state.custommap = mapList;
	cl_state.gamemap   = mapList;

}

void cl_done(void)
{

}

void client_socket_event_send(const net_socket_t * ns, const gclientevent_t * event)
{
	size_t buflen = 0;
	size_t len;
	char buf[sizeof(gclientevent_t)];
	memset(buf, 0, sizeof(buf));

	buf[buflen++] = event->type;

	switch(event->type)
	{
		case GCLIENTEVENT_DISCOVERYSERVER:
			break;
		case GCLIENTEVENT_CONNECT:
			break;
		case GCLIENTEVENT_DISCONNECT:
			break;
		case GCLIENTEVENT_JOIN:
			break;
		case GCLIENTEVENT_CONTROL:
			len = strnlen(event->control.action, GAME_EVENT_CONTROL_ACTION_SIZE - 1);
			strncpy(&(buf[buflen]), event->control.action, len);
			buflen += len;
			buf[buflen++] = '\0';
			break;
		case GCLIENTEVENT_SVCTRL_GAMEABORT:
			break;
		case GCLIENTEVENT_SVCTRL_NEXTGAMESTATE:
			break;
		case GCLIENTEVENT_SVCTRL_GAMESAVE_SAVE:
			buf[buflen++] = event->gamesave.isave;
			break;
		case GCLIENTEVENT_SVCTRL_GAMESAVE_LOAD:
			buf[buflen++] = event->gamesave.isave;
			break;
		case GCLIENTEVENT_SVCTRL_SETGAMEMAP:
			len = strnlen(event->setgamemap.mapname, MAP_FILENAME_SIZE - 1);
			strncpy(&(buf[buflen]), event->setgamemap.mapname, len);
			buflen += len;
			buf[buflen++] = '\0';
			break;
	}

	net_send(
		ns,
		buf,
		buflen
	);

}

void client_event_send(const client_t * client, const gclientevent_t * event)
{
	client_socket_event_send(client->ns, event);
}

void client_event_join_send(void)
{
	gclientevent_t event;
	event.type = GCLIENTEVENT_JOIN;
	client_t * client;
	LIST2_FOREACH(clients, client)
	{
		client_event_send(client, &event);
	}

}

void client_event_control_send(int clientId, const char * action_name)
{
	gclientevent_t event;
	event.type = GCLIENTEVENT_CONTROL;
	strncpy(event.control.action, action_name, GAME_EVENT_CONTROL_ACTION_SIZE);

	client_t * client;
	int num, i;
	LIST2_FOREACH_I(clients, client, num);
	if(clientId < 0 || clientId >= num)
		return;
	LIST2_LIST_TO_IENT(clients, client, i, num - 1 - clientId);

	client_event_send(client, &event);
}

void client_event_gameabort_send(void)
{
	gclientevent_t event;
	event.type = GCLIENTEVENT_SVCTRL_GAMEABORT;
	client_t * client;
	LIST2_FOREACH(clients, client)
	{
		client_event_send(client, &event);
	}
}

void client_event_nextgamestate_send(void)
{
	gclientevent_t event;
	event.type = GCLIENTEVENT_SVCTRL_NEXTGAMESTATE;
	client_t * client;
	LIST2_FOREACH(clients, client)
	{
		client_event_send(client, &event);
	}
}

void client_event_gamesave_save_send(int isave)
{
	gclientevent_t event;
	event.type = GCLIENTEVENT_SVCTRL_GAMESAVE_SAVE;
	event.gamesave.isave = isave;
	client_t * client;
	LIST2_FOREACH(clients, client)
	{
		client_event_send(client, &event);
	}
}

void client_event_gamesave_load_send(int isave)
{
	gclientevent_t event;
	event.type = GCLIENTEVENT_SVCTRL_GAMESAVE_LOAD;
	event.gamesave.isave = isave;
	client_t * client;
	LIST2_FOREACH(clients, client)
	{
		client_event_send(client, &event);
	}
}

void client_event_setgamemap_send(const char * mapname)
{
	gclientevent_t event;
	event.type = GCLIENTEVENT_SVCTRL_SETGAMEMAP;
	strncpy(event.setgamemap.mapname, mapname, MAP_FILENAME_SIZE);
	client_t * client;
	LIST2_FOREACH(clients, client)
	{
		client_event_send(client, &event);
	}
}

static void client_delete(client_t * client)
{
	net_socket_close(client->ns);
	Z_free(client);
}

/**
 * подключение игрока к игре
 * @return id
 * @return -1 ошибка
 */
int client_connect(void)
{
	net_socket_t * ns = net_socket_create(NET_PORT, "127.0.0.1");
	if(!ns)
		return -1;

	client_t * client = Z_malloc(sizeof(client_t));
	client->ns = ns;
	client->state = CLIENT_AWAITING_CONNECTION;
	client->time = time_current;

	LIST2_PUSH(clients, client);

	gclientevent_t event;
	event.type = GCLIENTEVENT_CONNECT;
	client_event_send(client, &event);

	size_t i;
	LIST2_FOREACH_I(clients, client, i);
	return (i - 1);
}

/*
 * инициализация камер клиентов
 */
void clients_initcams(void)
{
	client_t * client;
	size_t clients_num;
	LIST2_FOREACH_I(clients, client, clients_num);

	float cam_sx = (float)VIDEO_SCREEN_W * (float)VIDEO_SCALEX / (float)VIDEO_SCALE;
	float cam_sy = (float)VIDEO_SCREEN_H * (float)VIDEO_SCALEY / (float)VIDEO_SCALE;

	cam_sx /= clients_num;

	float statusbar_h = 32.0f * (float)VIDEO_SCALEY / (float)VIDEO_SCALE;

	float border = 1.0;
	float x = border;
	LIST2_FOREACH(clients, client)
	{
		VEC2_CLEAR(client->cam.origin);

		client->cam.x     = x;
		client->cam.y     = 0;
		client->cam.sx    = cam_sx - (border * 2.0);
		client->cam.sy    = cam_sy - statusbar_h;//184
		x = x + cam_sx;
	}

}

static void client_disconnect(client_t * client)
{
	gclientevent_t event;
	event.type = GCLIENTEVENT_DISCONNECT;
	client_event_send(client, &event);

	LIST2_UNLINK(clients, client);

	client_delete(client);
}

static const char * gamestate_to_str(gamestate_t state)
{
	static const char *list[] =
	{
			"GAMESTATE_NOGAME",
			"GAMESTATE_MISSION_BRIEF",
			"GAMESTATE_GAMESAVE",
			"GAMESTATE_INGAME",
			"GAMESTATE_INTERMISSION",
	};
	return list[state];
}

/*
 * слушать хост, получать от него сообщения для клиентов
 */
static void client_listen(void)
{
	struct sockaddr addr;
	socklen_t addr_len = 0;
	size_t buf_size;
	char * buf = NULL;
	size_t ofs;

	client_t * client;
	client_t * erased;

	ghostevent_t hevent;

	char * entname;

	LIST2_FOREACHM(clients, client, erased)
	{

		for(;client != NULL;)
		{
			buf = net_recv(client->ns, &buf_size, &addr, &addr_len);
			if(!buf)
				break;
			ofs = 0;

			switch(client->state)
			{
				case CLIENT_AWAITING_CONNECTION:

					if(!buf)
					{
						if(time_current - client->time > CLIENT_TIMEOUT)
						{
							LIST2_FOREACHM_EXCLUDE(clients, client, erased);
							client_delete(erased);
						}
						break;
					}

					client->time = time_current;
					if(buf[ofs] == GHOSTEVENT_CONNECTION_ACCEPTED)
					{
						ofs++;
						game_console_send("client: server accept connection at 0x%00000000x:%d.", client->ns->addr_.addr_in.sin_addr, ntohs(client->ns->addr_.addr_in.sin_port));
						client->state = CLIENT_LISTEN;
					}

					break;
				case CLIENT_LISTEN:
					if(!buf)
						break;
					client->time = time_current;
					hevent.type = buf[ofs++];

					switch(hevent.type)
					{
						case GHOSTEVENT_INFO:
							break;
						case GHOSTEVENT_CONNECTION_ACCEPTED:
							break;
						case GHOSTEVENT_CONNECTION_CLOSE:
							LIST2_FOREACHM_EXCLUDE(clients, client, erased);
							client_delete(erased);
							game_console_send("client: host closed the connection.");
							break;
						case GHOSTEVENT_GAMESTATE:
							hevent.gamestate.state = buf[ofs];
							cl_state.state = hevent.gamestate.state;
							game_console_send("client: host change gamestate to %s.", gamestate_to_str(cl_state.state));

							if(hevent.gamestate.state == GAMESTATE_INTERMISSION)
							{
								game_console_send("client: host say: GAME WIN!");
								cl_state.win = true;
							}

							break;
						case GHOSTEVENT_SETPLAYERENTITY:
							entname = &buf[ofs];
							ofs += GAME_HOSTEVENT_ENTNAME_SIZE;
							entity_t * clientent;
							memcpy(&clientent, &buf[ofs], sizeof(entity_t*));
							bool local_client = true;
							if(local_client)
							{
								client->entity = clientent;
							}

							break;
						case GHOSTEVENT_GAMESAVE_LOADED:
							cl_state.flags = buf[ofs];
							if(cl_state.flags & GAMEFLAG_2PLAYERS)
							{
								client_connect();
								clients_initcams();
							}
							break;
					}

					break;

			}
			net_recv_free(buf);
		}

		LIST2_FOREACHM_NEXT(client, erased);

	}

	if(clients == NULL)
		cl_state.state = GAMESTATE_NOGAME;
}

static void client_events_pump()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		//printf("event.type = %d\n", event.type);
		switch(event.type)
		{
			case SDL_KEYDOWN:
				input_key_setState(event.key.keysym.scancode, true);
				break;
			case SDL_KEYUP:
				input_key_setState(event.key.keysym.scancode, false);
				break;
		}
		//player_checkcode();
	}
}

void client(void)
{

	client_listen();

	if(cl_state.state == GAMESTATE_GAMESAVE)
	{
		cl_state.show_menu = true;
		cl_state.imenu     = MENU_GAME_SAVE;
	}

	if(cl_state.show_menu)
	{
		menu_events_pump();
	}
	else
	{
		client_events_pump();
	}

	if(cl_state.show_menu)
	{
		cl_state.paused = true;
		menu_selector_t imenu_process = cl_state.imenu;
		cl_state.imenu = menu_handle(imenu_process);
		menu_draw(imenu_process);
	}
	else
	{
		cl_state.paused = false;
		cl_game_draw();
	}

}

