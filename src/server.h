/*
 * server.h
 *
 *  Created on: 27 дек. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_SERVER_H_
#define SRC_SERVER_H_

#include "net.h"
#include "g_events.h"

#include "game.h"
#include "net.h"
#include "map.h"
#include "vars.h"

#define SERVER_TX_QUEUE_SIZE 16

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

typedef struct host_client_s
{
    struct host_client_s * prev;
    struct host_client_s * next;

    struct entity_s * entity;
    void * userstoredata;

    /* сохраняемые переменные */
    var_t * vars; /* vardata_t */

} server_player_t;

typedef struct
{
    game_server_event_t req;
} server_tx_t;

typedef struct server_client_s
{
    struct server_client_s * prev;
    struct server_client_s * next;

    /* клиент является главным, может управлять сервером.
     * только от него принимаются команды управления сервером.
     */
    bool main;
    /* адрес клиента */
    net_socket_t ns;

    int players_num;
    server_player_t * players;

    size_t tx_queue_num;
    server_tx_t tx_queue[SERVER_TX_QUEUE_SIZE];

} server_client_t;

typedef struct
{
    net_socket_t * ns;

    server_client_t * clients;
} server_t;

#include "entity.h"

extern void server_init(void);
extern void server_done(void);

extern int server_client_join(server_client_t * client, int players_num);

extern int server_client_num_get(void);

extern void server_setgamestate(gamestate_t state);

extern vardata_t * server_client_vardata_get(server_player_t * client, const char * varname, vartype_t vartype);

extern server_client_t * server_client_get(int id);

extern void server_event_info_send(server_client_t * client);
extern void server_event_send_win(void);
extern void server_event_cliententity_send(server_client_t * client);
extern void server_event_gamestate_send(server_client_t * client, gamestate_t state);

extern void server_unjoin_clients(void);
extern void server_restore_client_info(server_player_t * client);


extern void server_start(int flags);
extern void server_stop(void);

extern void server_handle(void);

#endif /* SRC_SERVER_H_ */
