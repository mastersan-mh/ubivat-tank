/*
 * client_events.h
 *
 *  Created on: 1 нояб. 2017 г.
 *      Author: mastersan
 */

#ifndef SRC_CLIENT_EVENTS_H_
#define SRC_CLIENT_EVENTS_H_

#include "types.h"
#include "g_events.h"

/* события клиента */

typedef enum
{
    G_CLIENT_EVENT_LOCAL_KEY_PRESS,
    G_CLIENT_EVENT_LOCAL_KEY_RELEASE,
    G_CLIENT_EVENT_LOCAL_ENTERGAME,
    G_CLIENT_EVENT_REMOTE_INFO, /* информация о срвере на запрос G_CLIENT_REQ_DISCOVERYSERVER */
    G_CLIENT_EVENT_REMOTE_CONNECTION_ACCEPTED,
    G_CLIENT_EVENT_REMOTE_CONNECTION_CLOSE,
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
        int players_num; /**< amount of client local players */
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

extern void client_event_local_key_input(bool key_repeat, int key, bool state);

#endif /* SRC_CLIENT_EVENTS_H_ */
