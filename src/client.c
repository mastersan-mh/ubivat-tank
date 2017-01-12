/*
 * client.c
 *
 *  Created on: 8 дек. 2016 г.
 *      Author: mastersan
 */

#include "common_list2.h"
#include "net.h"
#include "client.h"
#include "g_events.h"
#include "game.h"
#include "Z_mem.h"
#include "ui.h"
#include "video.h"

#include <string.h>

static client_t * clients;

void client_event_send(client_t * client, gclientevent_t * event)
{
	size_t buflen = 0;
	size_t len;
	char buf[sizeof(gclientevent_t)];
	memset(buf, 0, sizeof(buf));

	switch(event->type)
	{
		case GCLIENTEVENT_CONNECT:
			buf[buflen] = event->type;
			buflen++;
			break;
		case GCLIENTEVENT_DISCONNECT:
			buf[buflen] = event->type;
			buflen++;
			break;
		case GCLIENTEVENT_CONTROL:
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
int client_connect()
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
void clients_initcams()
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
		client->cam.pos.x = 0;
		client->cam.pos.y = 0;

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

static void client_listen_clients()
{
	struct sockaddr addr;
	socklen_t addr_len = 0;
	size_t buf_size;
	char * buf = NULL;
	size_t ofs;

	client_t * client;
	client_t * erased;


	LIST2_FOREACHM(clients, client, erased)
	{

		buf = net_recv(client->ns, &buf_size, &addr, &addr_len);
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
				ofs = 0;
				if(buf[ofs] == GHOSTEVENT_CONNECTION_ACCEPTED)
				{
					game_console_send("client: server accept connection to 0x%00000000x:%d.", client->ns->addr_in.sin_addr, ntohs(client->ns->addr_in.sin_port));

					ofs++;
					char * entname = &buf[ofs];
					ofs += GAME_HOSTEVENT_ENTNAME_LEN + 1;
					entity_t * clientent;
					memcpy(&clientent, &buf[ofs], sizeof(entity_t*));

					client->state = CLIENT_LISTEN;
					bool local_client = true;
					if(local_client)
					{
						client->entity = clientent;
					}
				}

				break;
			case CLIENT_LISTEN:
				client->time = time_current;
				if(!buf)
					break;
				if(buf[0] == GHOSTEVENT_CONNECTION_CLOSE)
				{
					LIST2_FOREACHM_EXCLUDE(clients, client, erased);
					client_delete(erased);
					game_console_send("client: host closed the connection.");
				}

				break;


		}
		net_recv_free(buf);

		LIST2_FOREACHM_NEXT(client, erased);

	}
}

void client_event_control_send(int clientId, const char * action_name)
{
	gclientevent_t event;
	event.type = GCLIENTEVENT_CONTROL;
	strncpy(event.control.action, action_name, GAME_EVENT_CONTROL_ACTION_LEN);

	client_t * client;
	int i;
	LIST2_LIST_TO_IENT(clients, client, i, clientId);
	client_event_send(client, &event);
}



void client()
{
	client_listen_clients();
}

#include "video.h"

#include "ent_player.h"

static void client_game_draw_cam(camera_t * cam, entity_t * player)
{

	player_t * pl = player->data;

	if(pl->bull)
	{
		cam->pos.x = pl->bull->pos.x;
		cam->pos.y = pl->bull->pos.y;
	}
	else
	{
		cam->pos.x = player->pos.x;
		cam->pos.y = player->pos.y;
	}

	map_draw(cam);
}


void client_draw()
{
	client_t * client;



	LIST2_FOREACH(clients, client)
	{
		entity_t * entity = client->entity;
		if(entity)
		{
			camera_t * cam = &client->cam;

			video_viewport_set(
				cam->x,
				cam->y,
				cam->sx,
				cam->sy
			);

			client_game_draw_cam(cam, entity);
			ui_draw(cam, entity);
		}
	}

	video_viewport_set(
		0.0f,
		0.0f,
		VIDEO_SCREEN_W,
		VIDEO_SCREEN_H
	);

	if(game.msg)
	{
		font_color_set3i(COLOR_1);
		video_printf_wide(96, 84, 128, game.msg);
		game.msg = NULL;
	};


}
