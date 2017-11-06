/*
 * server_reply.h
 *
 *  Created on: 3 нояб. 2017 г.
 *      Author: mastersan
 */

#ifndef SRC_SERVER_REPLY_H_
#define SRC_SERVER_REPLY_H_

#include "game.h"
#include "g_events.h"

/* тип события */
typedef enum
{
    G_SERVER_REPLY_INFO, /* информация о срвере на запрос G_CLIENT_REQ_DISCOVERYSERVER */
    G_SERVER_REPLY_CONNECTION_ACCEPTED,
    G_SERVER_REPLY_CONNECTION_CLOSE,
    G_SERVER_REPLY_PLAYERS_ENTITY_SET,
} game_server_reply_type_t;

typedef union
{
    struct
    {
        int clients_num; /* количество клиентов на сервере */
    } INFO;
    struct
    {
        int players_num; /**< amount of client local players */
        struct
        {
            char entityname[GAME_SERVER_EVENT_ENTNAME_SIZE];
            void /*entity_t */ * entity;
        } ent[2];
    } PLAYERS_ENTITY_SET;
} game_server_reply_data_t;


typedef struct
{
    game_server_reply_type_t type;
    game_server_reply_data_t data;
} game_server_reply_t;

#endif /* SRC_SERVER_REPLY_H_ */
