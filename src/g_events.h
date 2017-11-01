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

/* количество игроков клиента назначает клиент */
#define GAME_CLIENT_PLAYERSNUM_ASSIGN_CLIENT 0



/* события клиента */

typedef enum
{
    G_CLIENT_EVENT_LOCAL_KEY_PRESS,
    G_CLIENT_EVENT_LOCAL_KEY_RELEASE,
    G_CLIENT_EVENT_REMOTE_INFO, /* информация о срвере на запрос G_CLIENT_REQ_DISCOVERYSERVER */
    G_CLIENT_EVENT_REMOTE_CONNECTION_ACCEPTED,
    G_CLIENT_EVENT_REMOTE_CONNECTION_CLOSE,
    G_CLIENT_EVENT_REMOTE_GAME_STATE_SET,
    G_CLIENT_EVENT_REMOTE_PLAYERS_JOIN_AWAITING,
    G_CLIENT_EVENT_REMOTE_PLAYERS_ENTITY_SET,
} game_client_event_type_t;
typedef union
{

    struct
    {
        int key;
    } LOCAL_KEY_PRESS;
    struct
    {
        int key;
    } LOCAL_KEY_RELEASE;
    struct
    {
        int clients_num; /* количество клиентов на сервере */
    } REMOTE_INFO;
    struct
    {
        gamestate_t state;
    } REMOTE_GAME_STATE_SET;
    struct
    {
        int players_num; /**< amount of client local players */
    } REMOTE_PLAYERS_JOIN_AWAITING;
    struct
    {
        struct
        {
            char entityname[GAME_SERVER_EVENT_ENTNAME_SIZE];
            void /*entity_t */ * entity;
        } ent[2];
    } REMOTE_PLAYERS_ENTITY_SET;
} game_client_event_data_t;

typedef struct
{
    game_client_event_type_t type;
    game_client_event_data_t data;

} game_client_event_t;

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
