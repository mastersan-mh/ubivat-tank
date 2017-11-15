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
#include "entity_internal.h"
#include "server_reply.h"
#include "server_events.h"
#include "server_fsm.h"

#include <sys/queue.h>

#define SERVER_CLIENT_TX_QUEUE_SIZE 16

typedef struct server_player_s
{
    struct server_player_s * prev;
    struct server_player_s * next;

    struct entity_s * entity;

} server_player_t;

typedef struct
{
    net_socket_t * ns;
    server_reply_t req;
} server_tx_t;

typedef struct server_player_vars_storage_s
{
    struct server_player_vars_storage_s * prev;
    struct server_player_vars_storage_s * next;
    size_t clientId;
    size_t playerId;

    const entityinfo_t * info;
    /* сохраняемые переменные */
    void * vars;
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
    server_event_head_t events;

    server_state_t state;
    server_gameflags_t flags;

    server_gamestate_t gamestate;
    //состояние игры
    struct
    {

        char * msg;
        bool paused;

        maplist_t * gamemap;
        maplist_t * custommap;
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

extern const entity_action_t * server_entity_action_find(const entity_t * ent, const char * action_str);
server_player_t * server_client_player_get_by_id(const server_client_t * client, int playerId);

extern int server_gamesave_load(int isave);

extern int server_pdu_parse(const net_addr_t * sender, const char * buf, size_t buf_len);
extern int server_pdu_build(server_client_t * client, char * buf, size_t * buf_len, size_t buf_size);

extern void server_tx(void);




#endif /* SRC_SERVER_PRIVATE_H_ */
