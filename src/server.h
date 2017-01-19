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

	char * msg;
	/* флаги состояния игры */
	int flags;
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
	/* клиент является главным, может управлять сервером.
	 * только от него принимаются команды управления сервером.
	 */
	bool main;
} host_client_t;

#include "entity.h"

extern void server_init();
extern void server_done();

extern int host_client_join(host_client_t * client);
extern void host_clients_disconnect();

extern int host_client_num_get();

extern void host_setgamestate(gamestate_t state);

extern host_client_t * host_client_get(int id);

extern void host_event_send_win();
extern void host_event_cliententity_send(host_client_t * client);
extern void host_event_gamestate_send(host_client_t * client, gamestate_t state);

extern void server_unjoin_clients();
extern void server_restore_client_info(host_client_t * client);

extern void server_start();
extern void server_stop();

extern void server();

#endif /* SRC_SERVER_H_ */
