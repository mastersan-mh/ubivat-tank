/*
 * client.h
 *
 *  Created on: 8 дек. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_CLIENT_H_
#define SRC_CLIENT_H_

#include "net.h"
#include "g_events.h"
#include "menu.h"

#include "entity.h"
#include "map.h"

#define CLIENT_TIMEOUT 30000

#define CLIENT_EVENTS_MAX 10

#define CLIENT_REQ_QUEUE_SIZE 5

typedef struct client_player_s
{
    struct client_player_s * prev;
    struct client_player_s * next;

    /* ассоциированный сокет */
    //net_socket_t * ns;

    entity_t * entity;
    camera_t cam;

    size_t events_num;
    game_client_player_request_data_t events[CLIENT_EVENTS_MAX];

} client_player_t;

typedef struct
{
    game_client_request_t req;
} client_req_queue_t;

typedef struct
{


    //состояние игры
    struct
    {
        gamestate_t state;
        char * msg;
        /* флаги состояния игры */
        int flags;
        /* игрок победил */
        bool win;

        maplist_t * gamemap;
        maplist_t * custommap;

    } gamestate;


    net_socket_t * ns;

    enum
    {
        CLIENT_AWAITING_CONNECTION,
        CLIENT_LISTEN
    } state;

    /* время последнего получения сообщения */
    unsigned long time;

    client_player_t * players;

    size_t req_queue_num;
    client_req_queue_t req_queue[CLIENT_REQ_QUEUE_SIZE];

} client_t;

extern client_t client;


extern void cl_game_init(void);
extern void cl_done(void);

extern void client_start(int flags);
extern void client_stop(void);
extern bool client_running(void);

extern int client_player_num_get(void);
extern client_player_t * client_player_get(int playerId);


extern int client_connect(void);
extern void client_initcams(void);

extern void client_req_send(const game_client_request_t * req);
extern void client_req_join_send(void);
extern void client_player_action_send(int playerId, const char * action_name);
extern void client_req_gameabort_send(void);
extern void client_req_nextgamestate_send(void);
extern void client_req_gamesave_save_send(int isave);
extern void client_req_gamesave_load_send(int isave);
extern void client_req_setgamemap_send(const char * mapname);

extern void client_events_pump(void);

extern void client_handle(void);

#endif /* SRC_CLIENT_H_ */
