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
#include "server_reply.h"
#include "server_events.h"
#include "server_fsm.h"

#include <sys/queue.h>

typedef struct game_server_event_entry_s
{
    TAILQ_ENTRY(game_server_event_entry_s) entry;
    game_server_event_t ev;
} game_server_event_entry_t;

#define SERVER_CLIENT_TX_QUEUE_SIZE 16

typedef struct server_player_s
{
    struct server_player_s * prev;
    struct server_player_s * next;

    struct entity_s * entity;
    void * userstoredata;

} server_player_t;

typedef struct
{
    net_socket_t * ns;
    game_server_reply_t req;
} server_tx_t;

typedef struct server_player_vars_storage_s
{
    struct server_player_vars_storage_s * prev;
    struct server_player_vars_storage_s * next;
    size_t clientId;
    size_t playerId;
    /* сохраняемые переменные */
    var_t * vars; /* vardata_t */
} server_player_vars_storage_t;

typedef struct server_client_s
{
    struct server_client_s * prev;
    struct server_client_s * next;

    /* состояние игры для клиента */
    server_gamestate_t gamestate;

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
    server_tx_t tx_queue[SERVER_CLIENT_TX_QUEUE_SIZE];

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
    TAILQ_HEAD(event_s, game_server_event_entry_s) events;

    server_state_t state;
    server_gameflags_t flags;

    server_gamestate_t gamestate;
    server_gamestate_t gamestate_prev;
    //состояние игры
    struct
    {

        char * msg;
        bool paused;

        maplist_t * gamemap;
        maplist_t * custommap;
        /* Разрешить переход на состояние сохранения игры.
       Если игра была только что прочитана, её не нужно сохранять */
        bool allow_state_gamesave;
    } gstate;

    net_socket_t * ns;

    server_client_t * clients;

    /* массив по клиентам, для сохранения переменных */
    server_player_vars_storage_t * storages;

} server_t;

extern server_t server;

extern const char * server_gamestate_to_str(server_gamestate_t state);

extern void server_storages_free();
extern server_player_vars_storage_t * server_storage_find(size_t clientId, size_t playerId);
extern server_player_vars_storage_t * server_storage_create(size_t clientId, size_t playerId);

extern server_client_t * server_client_find_by_addr(const net_addr_t * addr);

extern server_client_t * server_client_create(int sock, const net_addr_t * net_addr, bool main);
extern void server_client_delete(server_client_t * client);
extern void server_clients_delete(void);

extern int server_client_spawn(server_client_t * client, int players_num);
extern void server_clients_unspawn(void);
extern int server_clients_num_get(void);
size_t server_client_id_get(server_client_t * client);
extern int server_client_players_num_get(const server_client_t * client);
extern void server_client_players_num_set(server_client_t * client, int players_num);
extern server_client_t * server_client_get(int id);


extern server_player_t * server_player_create();
extern void server_player_delete(server_player_t * player);
extern void server_player_info_store(server_player_vars_storage_t * storage, server_player_t * player);
extern void server_player_info_restore(server_player_t * player, server_player_vars_storage_t * storage);

extern vardata_t * server_storage_vardata_get(server_player_vars_storage_t * storage, const char * varname, vartype_t vartype);

extern const entityaction_t * server_entity_action_find(const entity_t * ent, const char * action_str);
server_player_t * server_client_player_get_by_id(const server_client_t * client, int playerId);

extern int server_gamesave_load(int isave);





#endif /* SRC_SERVER_PRIVATE_H_ */
