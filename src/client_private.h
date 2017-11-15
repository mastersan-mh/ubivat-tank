/*
 * client_private.h
 *
 *  Created on: 1 нояб. 2017 г.
 *      Author: mastersan
 */

#ifndef SRC_CLIENT_PRIVATE_H_
#define SRC_CLIENT_PRIVATE_H_

#include "g_events.h"
#include "net.h"
#include "client_requests.h"
#include "client_fsm.h"

#include "entity_internal.h"

typedef enum
{
    CLIENT_STATE_IDLE,
    CLIENT_STATE_INIT,
    CLIENT_STATE_RUN,
    CLIENT_STATE_DONE
} client_state_t;

typedef struct client_player_s
{
    struct client_player_s * prev;
    struct client_player_s * next;

    /* ассоциированный сокет */
    //net_socket_t * ns;

    entity_t * entity;
    camera_t cam;

} client_player_t;

typedef struct
{
    client_request_t req;
} client_req_queue_t;

typedef struct
{
    client_event_head_t events;

    client_state_t state;

    client_gamestate_t gamestate;

    //состояние игры
    struct
    {
        int players_num; /**< amount of local players: 0, 1, 2 */

        char * msg;
        /* игрок победил */
        bool win;
        bool endgame;
    } gstate;

    net_socket_t * ns;

    /* время последнего получения сообщения */
    unsigned long time;

    client_player_t * players;

    size_t tx_queue_num;
    client_req_queue_t tx_queue[CLIENT_REQ_QUEUE_SIZE];

} client_t;

extern client_t client;

extern const char * client_gamestate_to_str(client_gamestate_t state);

extern void client_disconnect();

extern void client_initcams(void);

extern int client_player_num_get(void);

extern client_player_t * client_player_get(int playerId);
void client_player_delete(client_player_t * player);
void client_players_delete(void);


#endif /* SRC_CLIENT_PRIVATE_H_ */
