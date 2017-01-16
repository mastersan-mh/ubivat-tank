/*
 * server.h
 *
 *  Created on: 27 дек. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_SERVER_H_
#define SRC_SERVER_H_

#include "game.h"
#include "net.h"
#include "map.h"
#include "menu.h"

//состояние игры
typedef struct
{
	gamestate_t state;
	gamestate_t state_prev;

	char * msg;
	/* флаги состояния игры */
	int flags;
	/* игрок победил */
	bool win;
	bool show_menu;
	bool paused;
	int sound_playId;

	maplist_t * gamemap;
	maplist_t * custommap;

} server_state_t;

typedef struct client_storedata_s
{
	//фрагов за пройденые карты
	int fragstotal;
	//фрагов на карте
	int frags;
	int level;
	int scores;
} client_storedata_t;

typedef struct host_client_s
{
	struct host_client_s * prev;
	struct host_client_s * next;

	struct entity_s * entity;
	client_storedata_t storedata;
	void * userstoredata;
	/* адрес клиента */
	net_socket_t * ns;
} host_client_t;

#include "entity.h"

extern void server_init();
extern void server_done();

extern void host_clients_disconnect();

extern int host_client_num_get();

extern host_client_t * host_client_get(int id);

extern int host_client_spawn_id(int id);

extern void host_client_unspawn_id(int id);

extern void host_event_send_win();
extern void host_event_send_imenu(menu_selector_t imenu);

extern void server_store_clients_info();
extern void server_restore_clients_info();

extern void server_start();
extern void server_stop();

extern void server();

#endif /* SRC_SERVER_H_ */
