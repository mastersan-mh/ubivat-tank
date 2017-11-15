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
    G_CLIENT_EVENT_LOCAL_CONNECT,
    G_CLIENT_EVENT_LOCAL_DICOVERYSERVER,
    G_CLIENT_EVENT_LOCAL_NEWGAME,
    G_CLIENT_EVENT_LOCAL_LOADGAME,
    G_CLIENT_EVENT_LOCAL_ENTERGAME,
    G_CLIENT_EVENT_REMOTE_INFO, /* информация о срвере на запрос G_CLIENT_REQ_DISCOVERYSERVER */
    G_CLIENT_EVENT_REMOTE_CONNECTION_ACCEPTED,
    G_CLIENT_EVENT_REMOTE_CONNECTION_CLOSE,
    G_CLIENT_EVENT_REMOTE_PLAYERS_ENTITY_SET,
    G_CLIENT_EVENT_REMOTE_GAME_ENDMAP,
} client_event_type_t;

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
        char mapname[MAP_NAME_SIZE];
    } LOCAL_NEWGAME;
    struct
    {
        int isave;
    } LOCAL_LOADGAME;
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
    struct
    {
        bool win; /**< win / lose */
        bool endgame; /* end of map / end of map and game*/
    } REMOTE_GAME_ENDMAP;
} client_event_data_t;

typedef struct client_event_s
{
    CIRCLEQ_ENTRY(client_event_s) queue;
    net_addr_t sender;
    client_event_type_t type;
    client_event_data_t data;
} client_event_t;

typedef CIRCLEQ_HEAD(client_event_head_s, client_event_s) client_event_head_t;

void client_events_handle(void);
void client_events_pop_all(void);

void client_event_send(
    const net_addr_t * sender,
    client_event_type_t type,
    const client_event_data_t * data);

extern void client_event_local_key_input(bool key_repeat, int key, bool state);
extern void client_event_local_connect(void);
extern void client_event_local_discoveryserver(void);
extern void client_event_local_newgame(const char * mapname);
extern void client_event_local_loadgame(int isave);
extern void client_event_local_entergame(void);

#endif /* SRC_CLIENT_EVENTS_H_ */
