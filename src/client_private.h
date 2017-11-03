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

#include "entity.h"

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
    game_client_request_t req;
} client_req_queue_t;

typedef struct
{
    client_state_t state;

    gamestate_t gamestate;
    gamestate_t gamestate_prev;

    //состояние игры
    struct
    {
        int players_num; /**< amount of local players: 0, 1, 2 */

        char * msg;
        /* игрок победил */
        bool win;
    } gstate;

    net_socket_t * ns;

    /* время последнего получения сообщения */
    unsigned long time;

    client_player_t * players;

    size_t req_queue_num;
    client_req_queue_t req_queue[CLIENT_REQ_QUEUE_SIZE];
    bool game_next_state_sended;// костыль

} client_t;

extern client_t client;

extern void client_disconnect();

extern void client_initcams(void);

extern int client_player_num_get(void);

extern client_player_t * client_player_get(int playerId);
void client_player_delete(client_player_t * player);
void client_players_delete(void);


#endif /* SRC_CLIENT_PRIVATE_H_ */
