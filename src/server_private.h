/*
 * server_private.h
 *
 *  Created on: 3 нояб. 2017 г.
 *      Author: mastersan
 */

#ifndef SRC_SERVER_PRIVATE_H_
#define SRC_SERVER_PRIVATE_H_

#include "net.h"
#include "types.h"
#include "vars.h"
#include "g_events.h"
#include "entity.h"

#define SERVER_TX_QUEUE_SIZE 16

typedef struct server_player_s
{
    struct server_player_s * prev;
    struct server_player_s * next;

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

    bool joined;

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

typedef enum
{
    SERVER_STATE_IDLE,
    SERVER_STATE_INIT,
    SERVER_STATE_RUN,
    SERVER_STATE_DONE
} server_state_t;

typedef struct
{
    server_state_t state;
    server_gameflags_t flags;

    //состояние игры
    struct
    {
        gamestate_t state;

        char * msg;
        bool paused;

        maplist_t * gamemap;
        maplist_t * custommap;
        /* Разрешить переход на состояние сохранения игры.
       Если игра была только что прочитана, её не нужно сохранять */
        bool allow_state_gamesave;
    } gamestate;

    net_socket_t * ns;

    server_client_t * clients;
} server_t;

extern server_t server;

extern server_client_t * server_client_find_by_addr(const net_addr_t * addr);
extern void server_client_players_num_set(server_client_t * client, int players_num);

extern int server_client_join(server_client_t * client, int players_num);
extern int server_client_players_num_get(const server_client_t * client);
extern vardata_t * server_client_vardata_get(server_player_t * client, const char * varname, vartype_t vartype);
extern server_client_t * server_client_get(int id);
extern void server_unjoin_clients(void);
extern int server_client_num_get(void);

extern void server_client_disconnect(server_client_t * client);
extern void server_client_delete(server_client_t * client);

extern int server_gamesave_load(int isave);

extern void server_clients_delete(void);

extern server_client_t * server_client_create(int sock, const net_addr_t * net_addr, bool main);

extern const entityaction_t * server_entity_action_find(const entity_t * ent, const char * action_str);
extern server_player_t * server_player_create();
extern void server_player_delete(server_player_t * player);

extern void server_client_player_info_restore(server_player_t * player);
extern void server_client_player_info_store(server_player_t * player);
server_player_t * server_client_player_get_by_id(const server_client_t * client, int playerId);
void server_reply_send_connection_accepted(server_client_t * client);
void server_reply_send_gamestate(server_client_t * client, gamestate_t state);


#endif /* SRC_SERVER_PRIVATE_H_ */
