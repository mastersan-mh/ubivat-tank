/*
 * client.h
 *
 *  Created on: 8 дек. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_CLIENT_H_
#define SRC_CLIENT_H_

#include "net.h"
#include "g_events.h"
#include "menu.h"

#include "entity.h"
#include "map.h"

#define CLIENT_TIMEOUT 30000

//состояние игры
typedef struct
{
	bool quit;
	gamestate_t state;

	menu_selector_t imenu;
	bool show_menu;

	char * msg;
	/* флаги состояния игры */
	int flags;
	/* игрок победил */
	bool win;
	bool paused;
	int sound_playId;

	maplist_t * gamemap;
	maplist_t * custommap;

} client_state_t;

extern client_state_t cl_state;

typedef struct client_s
{
	struct client_s * prev;
	struct client_s * next;

	/* ассоциированный сокет */
	//net_socket_t * ns;
	enum
	{
		CLIENT_AWAITING_CONNECTION,
		CLIENT_LISTEN
	} state;

	/* время последнего получения сообщения */
	unsigned long time;

	entity_t * entity;
	camera_t cam;

} client_client_t;

extern void cl_game_init(void);
extern void cl_done(void);

extern int client_connect(void);
extern void clients_initcams(void);

extern void client_event_send(const client_client_t * client, const gclientevent_t * event);
extern void client_event_join_send(void);
extern void client_event_control_send(int clientId, const char * action_name);
extern void client_event_gameabort_send(void);
extern void client_event_nextgamestate_send(void);
extern void client_event_gamesave_save_send(int isave);
extern void client_event_gamesave_load_send(int isave);
extern void client_event_setgamemap_send(const char * mapname);

extern void client(void);

#endif /* SRC_CLIENT_H_ */
