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

#include "common/common_queue.h"

#define SERVER_CLIENT_TX_QUEUE_SIZE 16

#define FOREACH_SERVER_CLIENTS(client, i) \
        for((client) = &server.clients[i = 0]; i < CLIENTS_MAX; (client) = &server.clients[i++])

typedef struct
{
    server_reply_t reply;
} server_client_tx_t;

typedef struct server_tx_s
{
    CIRCLEQ_ENTRY(server_tx_s) queue;
    net_addr_t net_addr;
    server_reply_t reply;
} server_tx_t;

typedef CIRCLEQ_HEAD(, server_tx_s) server_tx_head_t;

typedef struct server_player_vars_storage_s
{
    struct server_player_vars_storage_s * prev;
    struct server_player_vars_storage_s * next;
    size_t clientId;
    size_t playerId;

    /* сохраняемые переменные */
    void * vars;
} server_player_vars_storage_t;

typedef struct server_client_s
{
    bool used;

    /* адрес клиента */
    net_addr_t net_addr;

    /* состояние игры для клиента */

    bool joined;

    /* клиент является главным, может управлять сервером.
     * только от него принимаются команды управления сервером.
     */
    bool main;


    int players_num;

    size_t tx_queue_num;
    server_client_tx_t tx_queue[SERVER_CLIENT_TX_QUEUE_SIZE];

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
    size_t players_num;
} server_gamesave_client_info_t;

typedef struct
{
    server_event_head_t events;
    /* TX queue */
    server_tx_head_t txs;

    server_state_t state;
    server_gameflags_t flags;

    server_gamestate_t gamestate;
    //состояние игры
    struct
    {
        bool paused;

        maplist_t * gamemap;
        maplist_t * custommap;
    } gstate;

    int sock;
    uint16_t sv_port;
    in_addr_t sv_addr;

    server_client_t clients[CLIENTS_MAX];

    /* информация о клиентах из gamesave */
    size_t gamesave_clients_info_num;
    server_gamesave_client_info_t * gamesave_clients_info;

    /* массив по клиентам, для сохранения переменных */
    server_player_vars_storage_t * storages;

} server_t;

extern server_t server;

extern const char * server_gamestate_to_str(server_gamestate_t state);

extern void server_gamesave_clients_info_allocate(size_t clients_num);
extern void server_gamesave_clients_info_clean(void);
extern void server_gamesave_client_info_set(size_t clientId, size_t players_num);
extern size_t server_gamesave_client_info_get(size_t clientId);
extern void server_gamesave_client_info_mark(size_t clientId);

extern void server_storages_free();
extern server_player_vars_storage_t * server_storage_find(size_t clientId, size_t playerId);
extern server_player_vars_storage_t * server_storage_create(size_t clientId, size_t playerId);

extern server_client_t * server_client_find_by_addr(const net_addr_t * addr);

extern server_client_t * server_client_create(int sock, const net_addr_t * net_addr, bool main);
extern void server_client_delete(server_client_t * client);
extern void server_clients_delete(void);

extern int server_client_spawn(server_client_t * client, int players_num);
extern void server_clients_unspawn(void);
extern size_t server_clients_num_get(void);
ssize_t server_client_id_get(const server_client_t * client);
extern void server_client_players_num_set(server_client_t * client, int players_num);
extern server_client_t * server_client_get(size_t iclient);

extern int server_pdu_parse(const net_addr_t * sender, const char * buf, size_t buf_len);
extern int server_pdu_client_build(server_client_t * client, char * buf, size_t * buf_len, size_t buf_size);

extern void server_tx(void);




#endif /* SRC_SERVER_PRIVATE_H_ */
