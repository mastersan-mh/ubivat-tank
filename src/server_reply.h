/*
 * server_reply.h
 *
 *  Created on: 3 нояб. 2017 г.
 *      Author: mastersan
 */

#ifndef SRC_SERVER_REPLY_H_
#define SRC_SERVER_REPLY_H_

#include "types.h"

#include "game.h"
#include "g_events.h"

/* тип события */
typedef enum
{
    G_SERVER_REPLY_INFO, /* информация о срвере на запрос G_CLIENT_REQ_DISCOVERYSERVER */
    G_SERVER_REPLY_CONNECTION_RESULT,
    G_SERVER_REPLY_CONNECTION_CLOSE,
    G_SERVER_REPLY_GAME_NEXTMAP, /* следующая карта */
    G_SERVER_REPLY_PLAYERS_ENTITY_SET,
} server_reply_type_t;

typedef union
{
    struct
    {
        int clients_num; /* количество клиентов на сервере */
    } INFO;
    struct
    {
        bool accepted;
    }CONNECTION_RESULT;
    struct
    {
        int players_num; /**< amount of client local players */
        struct
        {
            char entityname[GAME_SERVER_EVENT_ENTNAME_SIZE];
            entity_id_t entityId;
        } ent[2];
    } PLAYERS_ENTITY_SET;
    struct
    {
        bool win; /**< win / lose */
        bool endgame; /* end of map / end of map and game*/
        char mapfilename[MAP_FILENAME_SIZE];
    } GAME_NEXTMAP;
} server_reply_data_t;


typedef struct
{
    server_reply_type_t type;
    server_reply_data_t data;
} server_reply_t;

#endif /* SRC_SERVER_REPLY_H_ */
