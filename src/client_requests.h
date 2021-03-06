/*
 * client_request.h
 *
 *  Created on: 1 нояб. 2017 г.
 *      Author: mastersan
 */

#ifndef SRC_CLIENT_REQUESTS_H_
#define SRC_CLIENT_REQUESTS_H_

#include "actions.h"

#define CLIENT_REQ_QUEUE_SIZE 20

/** @brief Запросы клиента на сервер */
typedef enum
{
    /** Непривилегированые запросы */
    G_CLIENT_REQ_DISCOVERYSERVER, /**< найти серверы */
    G_CLIENT_REQ_CONNECT,
    G_CLIENT_REQ_DISCONNECT,
    G_CLIENT_REQ_SPAWN,     /**< назначить игроку объект на карте */
    G_CLIENT_REQ_PLAYER_ACTION,
    /** Привилегированные запросы */
#define G_CLIENT_REQ_PRIVILEGED G_CLIENT_REQ_GAME_ABORT
    G_CLIENT_REQ_GAME_ABORT,
    G_CLIENT_REQ_GAME_SETMAP,
    G_CLIENT_REQ_READY, /**< перейти на следующее состояние, если допустимо */
    G_CLIENT_REQ_GAME_SAVE,
    G_CLIENT_REQ_GAME_LOAD,
} client_request_type_t;

typedef union
{
    struct
    {
        int players_num;
    } SPAWN;
    struct
    {
        int playerId;
        char action[GAME_ACTION_SIZE];
    } PLAYER_ACTION;
    struct
    {
        int isave;
    } GAME_SAVE;
    struct
    {
        int isave;
    } GAME_LOAD;
    struct
    {
        char mapname[MAP_FILENAME_SIZE];
    } GAME_SETMAP;
} client_request_data_t;

typedef struct
{
    client_request_type_t type;
    client_request_data_t data;
} client_request_t;


extern void client_req_send_discoveryserver(void);
extern void client_req_send_connect(void);
extern void client_req_send_disconnect(void);
extern void client_req_send_spawn(int players_num);
extern void client_req_send_player_action(int playerId, const char * action_name);
extern void client_req_send_game_abort(void);
extern void client_req_send_game_setmap(const char * mapname);
extern void client_req_send_ready(void);
extern void client_req_send_game_save(int isave);
extern void client_req_send_game_load(int isave);

#endif /* SRC_CLIENT_REQUESTS_H_ */
