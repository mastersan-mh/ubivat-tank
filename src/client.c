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

client_client_t client = {};

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

void client_req_send(const game_client_request_t * req)
{
	if(client.req_queue_num >= CLIENT_REQ_QUEUE_SIZE)
	{
		game_console_send("CLIENT: REQ queue overflow.");
		return;
	}
	client.req_queue[client.req_queue_num].req = *req;
	client.req_queue_num++;
}

void client_req_join_send(void)
{
	game_client_request_t req;
	req.req = G_CLIENT_REQ_JOIN;
	client_req_send(&req);
}

client_player_t * client_player_get(int playerId)
{
	client_player_t * player;
	int num, i;
	LIST2_FOREACH_I(client.players, player, num);
	if(playerId < 0 || playerId >= num)
		return NULL;
	LIST2_LIST_TO_IENT(client.players, player, i, num - 1 - playerId);
	return player;
}


void client_player_action_send(int playerId, const char * action_name)
{
	client_player_t * player;
	player = client_player_get(playerId);
	if(player->events_num >= CLIENT_EVENTS_MAX)
	{
		game_console_send("CLIENT: player %d events overflow.", playerId);
		return;
	}
	strncpy(player->events[player->events_num].control.action, action_name, GAME_EVENT_CONTROL_ACTION_SIZE);
	player->events_num++;
}

void client_req_gameabort_send(void)
{
	game_client_request_t req;
	req.req = G_CLIENT_REQ_GAME_ABORT;
	client_req_send(&req);
}

void client_req_nextgamestate_send(void)
{
	game_client_request_t req;
	req.req = G_CLIENT_REQ_GAME_NEXTSTATE;
	client_req_send(&req);
}

void client_req_gamesave_save_send(int isave)
{
	game_client_request_t req;
	req.req = G_CLIENT_REQ_GAME_SAVE;
	req.data.GAME_SAVE.isave = isave;
	client_req_send(&req);
}

void client_req_gamesave_load_send(int isave)
{
	game_client_request_t req;
	req.req = G_CLIENT_REQ_GAME_LOAD;
	req.data.GAME_LOAD.isave = isave;
	client_req_send(&req);
}

void client_req_setgamemap_send(const char * mapname)
{
	game_client_request_t req;
	req.req = G_CLIENT_REQ_GAME_SETMAP;
	strncpy(req.data.GAME_SETMAP.mapname, mapname, MAP_FILENAME_SIZE);
	client_req_send(&req);
}

static void client_player_delete(client_player_t * player)
{
	Z_free(player);
}

/**
 * подключение игрока к игре
 * @return error
 */
int client_connect(void)
{
	client_player_t * player;
	player = Z_malloc(sizeof(client_player_t));
	LIST2_PUSH(client.players, player);
	if(cl_state.flags & GAMEFLAG_2PLAYERS)
	{
		player = Z_malloc(sizeof(client_player_t));
		LIST2_PUSH(client.players, player);
	}

	client.state = CLIENT_AWAITING_CONNECTION;
	client.time = time_current;

	game_client_request_t req;
	req.req = G_CLIENT_REQ_CONNECT;
	client_req_send(&req);

	return 0;
}

/*
 * инициализация камер клиентов
 */
void clients_initcams(void)
{
	client_player_t * player;
	size_t clients_num;
	LIST2_FOREACH_I(client.players, player, clients_num);

	float cam_sx = (float)VIDEO_SCREEN_W * (float)VIDEO_SCALEX / (float)VIDEO_SCALE;
	float cam_sy = (float)VIDEO_SCREEN_H * (float)VIDEO_SCALEY / (float)VIDEO_SCALE;

	cam_sx /= clients_num;

	float statusbar_h = 32.0f * (float)VIDEO_SCALEY / (float)VIDEO_SCALE;

	float border = 1.0;
	float x = border;
	LIST2_FOREACH(client.players, player)
	{
		VEC2_CLEAR(player->cam.origin);

		player->cam.x     = x;
		player->cam.y     = 0;
		player->cam.sx    = cam_sx - (border * 2.0);
		player->cam.sy    = cam_sy - statusbar_h;//184
		x = x + cam_sx;
	}

}

static void client_disconnect(void)
{
	game_client_request_t req;
	req.req = G_CLIENT_REQ_DISCONNECT;
	client_req_send(&req);

	client_player_t * player;
	while(!LIST2_IS_EMPTY(client.players))
	{
		LIST2_UNLINK(client.players, player);
		client_player_delete(player);
	}
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

void client_start(int flags)
{
	client.ns = net_socket_create(NET_PORT, "127.0.0.1");

	if(client.ns == NULL)
	{
		game_halt("client socket() failed");
	}
	/*
	if(net_socket_bind(client_ns) < 0)
	{
		game_halt("client bind() failed");
	}
	*/
}



static void client_pdu_parse(char * buf, size_t buf_size)
{
	ghostevent_t hevent;
	char * entname;
	size_t ofs = 0;

	switch(client.state)
	{
	case CLIENT_AWAITING_CONNECTION:

		if(!buf)
		{
			if(time_current - client.time > CLIENT_TIMEOUT)
			{
				game_console_send("client: server reply timeout.");
			}
			break;
		}

		client.time = time_current;
		if(buf[ofs] == GHOSTEVENT_CONNECTION_ACCEPTED)
		{
			ofs++;
			game_console_send("client: server accept connection at 0x%00000000x:%d.", client.ns->addr_.addr_in.sin_addr, ntohs(client.ns->addr_.addr_in.sin_port));
			client.state = CLIENT_LISTEN;
		}

		break;
	case CLIENT_LISTEN:
		if(!buf)
			break;
		client.time = time_current;
		hevent.type = buf[ofs++];

		switch(hevent.type)
		{
		case GHOSTEVENT_INFO:
			break;
		case GHOSTEVENT_CONNECTION_ACCEPTED:
			break;
		case GHOSTEVENT_CONNECTION_CLOSE:
			client_disconnect();
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
			entity_t * clientent;
			ofs += GAME_HOSTEVENT_ENTNAME_SIZE;
			memcpy(&clientent, &buf[ofs], sizeof(entity_t*));
			bool local_client = true;
			if(local_client)
			{
				client_player_get(0)->entity = clientent;
			}
			ofs += GAME_HOSTEVENT_ENTNAME_SIZE;
			if(ofs < buf_size)
			{
				memcpy(&clientent, &buf[ofs], sizeof(entity_t*));
				if(local_client)
				{
					client_player_get(1)->entity = clientent;
				}
			}

			break;
		case GHOSTEVENT_GAMESAVE_LOADED:
			cl_state.flags = buf[ofs];
			client_connect();
			clients_initcams();
			break;
		}

		break;

	}
}



static char *client_pdu_build(size_t * pdu_size)
{
#define PDU_PUSH(pdu, ofs, data, size) \
		do { \
			memcpy(&(pdu)[(ofs)], (data), (size)); \
			(ofs) += (size); \
		} while (0);

	char * pdu;
	char * p;
	size_t ofs = 0;

	/* client requests */
	uint16_t req_queue_num = htons(client.req_queue_num);
	PDU_PUSH(pdu, ofs, &req_queue_num, sizeof(req_queue_num));
	for(size_t i = 0; i < client.req_queue_num; i++)
	{
		int16_t client_req;
		int16_t isave;
		game_client_request_t * req = &client.req_queue[i].req;
		client_req = htons(req->req);
		PDU_PUSH(pdu, ofs, &client_req, sizeof(client_req));
		switch(req->req)
		{
		/** Непривилегированые запросы */
		case G_CLIENT_REQ_DISCOVERYSERVER:
		case G_CLIENT_REQ_CONNECT:
		case G_CLIENT_REQ_DISCONNECT:
		case G_CLIENT_REQ_JOIN:
		/** Привилегированные запросы */
		case G_CLIENT_REQ_GAME_ABORT:
			break;
		case G_CLIENT_REQ_GAME_SETMAP:
			PDU_PISH(pdu, ofs, req->data.GAME_SETMAP.mapname, MAP_FILENAME_SIZE);
			break;
		case G_CLIENT_REQ_GAME_NEXTSTATE:
			break;
		case G_CLIENT_REQ_GAME_SAVE:
			isave = htons(req->data.GAME_SAVE.isave);
			PDU_PISH(pdu, ofs, isave, sizeof(isave));
			break;
		case G_CLIENT_REQ_GAME_LOAD:
			isave = htons(req->data.GAME_LOAD.isave);
			PDU_PISH(pdu, ofs, isave, sizeof(isave));
			break;
		}
	}
	client.req_queue_num = 0;

	/* players */
	client_player_t *player;
	LIST2_FOREACHR(client.players, player)
	{
		uint16_t player_req;
		if(player->events_num == 0)
		{
			player_req = htons(G_CLIENT_PLAYER_REQ_NONE);
			PDU_PUSH(pdu, ofs, &player_req, sizeof(player_req));
		}
		else
		{
			player_req = htons(G_CLIENT_PLAYER_REQ_CONTROL);
			PDU_PUSH(pdu, ofs, &player_req, sizeof(player_req));
			uint16_t pdu_events_num = htons(player->events_num);
			PDU_PUSH(pdu, ofs, &pdu_events_num, sizeof(pdu_events_num));
			for(size_t i = 0; i < player->events_num; i++)
			{
				PDU_PUSH(pdu, ofs, player->events[player->events_num].control.action, GAME_EVENT_CONTROL_ACTION_SIZE);
			}
			player->events_num = 0;
		}
	}
}


/*
 * слушать хост, получать от него сообщения для клиентов
 */
static void client_listen(void)
{
	if(!client.ns)
		return;

	struct sockaddr addr;
	socklen_t addr_len = 0;
	size_t buf_size;
	char * buf = NULL;

	do
	{
		buf = net_recv(client.ns, &buf_size, &addr, &addr_len);
		if(!buf)
			break;
		client_pdu_parse(buf, buf_size);
		net_pdu_free(buf);

	} while(1);

	buf = client_pdu_build(&buf_size);
	net_send(client.ns, buf, buf_size);
	net_pdu_free(buf);

	if(LIST2_IS_EMPTY(client.players))
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

void client_handle(void)
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

