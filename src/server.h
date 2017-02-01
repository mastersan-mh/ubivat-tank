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

//состояние игры
typedef struct
{
	gamestate_t state;

	char * msg;
	/* флаги состояния игры */
	int flags;
	bool paused;

	maplist_t * gamemap;
	maplist_t * custommap;
/* Разрешить переход на состояние сохранения игры.
   Если игра была только что прочитана, её не нужно сохранять */
	bool allow_state_gamesave;


} server_state_t;

#define _ENTITY_VARNAME_SIZE (64)

typedef struct
{
	char varname[_ENTITY_VARNAME_SIZE];
	entityvartype_t type;
	entityvarvalue_t value;
} host_clientvardata_t;

typedef struct host_client_s
{
	struct host_client_s * prev;
	struct host_client_s * next;

	struct entity_s * entity;
	void * userstoredata;

	/* сохраняемые переменные */
	size_t varsdata_num;
	host_clientvardata_t * varsdata;

	/* адрес клиента */
	net_socket_t * ns;
	/* клиент является главным, может управлять сервером.
	 * только от него принимаются команды управления сервером.
	 */
	bool main;
} host_client_t;

#include "entity.h"

extern void server_init(void);
extern void server_done(void);

extern int host_client_join(host_client_t * client);
extern void host_clients_disconnect(void);

extern int host_client_num_get(void);

extern void host_setgamestate(gamestate_t state);

extern host_client_t * host_client_get(int id);

extern void host_event_send_win(void);
extern void host_event_cliententity_send(host_client_t * client);
extern void host_event_gamestate_send(host_client_t * client, gamestate_t state);

extern void server_unjoin_clients(void);
extern void server_restore_client_info(host_client_t * client);

extern host_clientvardata_t * sv_client_storedvars_get(host_client_t * client, const char * varname);

extern void server_start(int flags);
extern void server_stop(void);

extern void server(void);

#endif /* SRC_SERVER_H_ */
