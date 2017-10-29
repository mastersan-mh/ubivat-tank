/*
 * events.h
 *
 *  Created on: 28 дек. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_G_EVENTS_H_
#define SRC_G_EVENTS_H_

#include "map.h"
#include "game.h"

#define GAME_SERVER_EVENT_ENTNAME_SIZE (64)
#define GAME_CLIENT_PLAYER_REQUEST_CONTROL_ACTION_SIZE (64)

/** @brief Запросы клиента на сервер */
typedef enum
{
    /** Непривилегированые запросы */
    G_CLIENT_REQ_DISCOVERYSERVER, /**< найти серверы */
    G_CLIENT_REQ_CONNECT,
    G_CLIENT_REQ_DISCONNECT,
    G_CLIENT_REQ_JOIN,     /**< назначить игроку объект на карте */
    /** Привилегированные запросы */
#define G_CLIENT_REQ_PRIVILEGED G_CLIENT_REQ_GAME_ABORT
    G_CLIENT_REQ_GAME_ABORT,
    G_CLIENT_REQ_GAME_SETMAP,
    G_CLIENT_REQ_GAME_NEXTSTATE, /**< перейти на следующее состояние, если допустимо */
    G_CLIENT_REQ_GAME_SAVE,
    G_CLIENT_REQ_GAME_LOAD,
} game_client_request_type_t;

typedef union
{
    struct
    {
        int players_num;
    } JOIN;
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
} game_client_request_data_t;

typedef struct
{
    game_client_request_type_t type;
    game_client_request_data_t data;
} game_client_request_t;

/* Управление игроком */
typedef enum
{
    G_CLIENT_PLAYER_REQ_NONE, /**< нет запроса */
    G_CLIENT_PLAYER_REQ_CONTROL,
} game_client_player_request_type_t;

typedef struct
{
    struct
    {
        char action[GAME_CLIENT_PLAYER_REQUEST_CONTROL_ACTION_SIZE];
    } CONTROL;
} game_client_player_request_data_t;

typedef struct
{
    game_client_player_request_type_t type;
    game_client_player_request_data_t data;
} game_client_player_request_t;

/* тип события */
typedef enum
{
    G_SERVER_EVENT_INFO, /* информация о срвере на запрос G_CLIENT_REQ_DISCOVERYSERVER */
    G_SERVER_EVENT_CONNECTION_ACCEPTED,
    G_SERVER_EVENT_CONNECTION_CLOSE,
    G_SERVER_EVENT_GAME_STATE_SET,
    G_SERVER_EVENT_PLAYERS_JOIN_AWAITING,
    G_SERVER_EVENT_PLAYERS_ENTITY_SET,
} game_server_event_type_t;

typedef union
{
    struct
    {
        int clients_num; /* количество клиентов на сервере */
    } INFO;
    struct
    {
        gamestate_t state;
    } GAME_STATE_SET;
    struct
    {
        int players_num; /**< amount of client local players */
    } PLAYERS_JOIN_AWAITING;
    struct
    {
        struct
        {
            char entityname[GAME_SERVER_EVENT_ENTNAME_SIZE];
            void /*entity_t */ * entity;
        } ent[2];
    } PLAYERS_ENTITY_SET;
} game_server_event_data_t;


typedef struct
{
    game_server_event_type_t type;
    game_server_event_data_t data;
} game_server_event_t;


#endif /* SRC_G_EVENTS_H_ */
