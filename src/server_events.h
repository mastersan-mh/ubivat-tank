/*
 * server_events.h
 *
 *  Created on: 3 нояб. 2017 г.
 *      Author: mastersan
 */

#ifndef SRC_SERVER_EVENTS_H_
#define SRC_SERVER_EVENTS_H_

#include "net.h"
#include "map.h"
#include "actions.h"

/** @brief События сервер */
typedef enum
{
    G_SERVER_EVENT_LOCAL_WIN,
    /** удалённые события */
#define G_SERVER_EVENT_REMOTE G_SERVER_EVENT_REMOTE_DISCOVERYSERVER
    G_SERVER_EVENT_REMOTE_DISCOVERYSERVER, /**< найти сервер */
    G_SERVER_EVENT_REMOTE_CLIENT_CONNECT,
    G_SERVER_EVENT_REMOTE_CLIENT_DISCONNECT,
    G_SERVER_EVENT_REMOTE_CLIENT_SPAWN,     /**< назначить игроку объект на карте */
    G_SERVER_EVENT_REMOTE_CLIENT_READY, /**< Клиент готов для следующего состояния */
    G_SERVER_EVENT_REMOTE_CLIENT_PLAYER_ACTION,
    G_SERVER_EVENT_REMOTE_GAME_ABORT,
    G_SERVER_EVENT_REMOTE_GAME_SETMAP,
    G_SERVER_EVENT_REMOTE_GAME_SAVE,
    G_SERVER_EVENT_REMOTE_GAME_LOAD,
} game_server_event_type_t;

typedef union
{
    struct
    {
        int players_num;
    } REMOTE_JOIN;
    struct
    {
        int playerId;
        char action[GAME_ACTION_SIZE];
    } REMOTE_PLAYER_ACTION;
    struct
    {
        int isave;
    } REMOTE_GAME_SAVE;
    struct
    {
        int isave;
    } REMOTE_GAME_LOAD;
    struct
    {
        char mapname[MAP_FILENAME_SIZE];
    } REMOTE_GAME_SETMAP;
} game_server_event_data_t;

typedef struct
{
    const net_addr_t * sender; /* NULL = local */
    game_server_event_type_t type;
    game_server_event_data_t data;

} game_server_event_t;

void server_event_local_win();

#endif /* SRC_SERVER_EVENTS_H_ */
