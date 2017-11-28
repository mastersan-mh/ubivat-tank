/*
 * client_events.c
 *
 *  Created on: 1 нояб. 2017 г.
 *      Author: mastersan
 */

#include "client_events.h"
#include "client_fsm.h"
#include "client_private.h"

void client_events_handle(void)
{
    while(!CIRCLEQ_EMPTY(&client.events))
    {
        client_event_t * event = CIRCLEQ_FIRST(&client.events);
        CIRCLEQ_REMOVE(&client.events, event, queue);
        client_fsm(event);
        Z_free(event);
    }
}

void client_events_flush(void)
{
    while(!CIRCLEQ_EMPTY(&client.events))
    {
        client_event_t * event = CIRCLEQ_FIRST(&client.events);
        CIRCLEQ_REMOVE(&client.events, event, queue);
        Z_free(event);
    }
}

void client_event_send(
    const net_addr_t * sender,
    client_event_type_t type,
    const client_event_data_t * data)
{
    client_event_t * event = Z_malloc(sizeof(client_event_t));
    if(sender)
        event->sender = *sender;
    event->type = type;
    if(data)
        event->data = *data;

    CIRCLEQ_INSERT_TAIL(&client.events, event, queue);
}

/**
 * @brief client game stop, make client idle
 */
void client_event_local_stop(void)
{
    client_event_send(NULL, G_CLIENT_EVENT_LOCAL_STOP, NULL);
}

/**
 * @return state = { true = pressed | false = released }
 */
void client_event_local_key_input(bool key_repeat, int key, bool key_pressed)
{
    if(key_repeat)
        return;
    client_event_type_t type;
    client_event_data_t data;

    if(key_pressed)
    {
        type = G_CLIENT_EVENT_LOCAL_KEY_PRESS;
        data.LOCAL_KEY_PRESS.key = key;
    }
    else
    {
        type = G_CLIENT_EVENT_LOCAL_KEY_RELEASE;
        data.LOCAL_KEY_RELEASE.key = key;
    }

    client_event_send(NULL, type, &data);
}

/**
 * @param net_addr  NULL - localhost
 */
void client_event_local_connect(const net_addr_t * net_addr)
{
    client_event_data_t data;
    data.LOCAL_CONNECT.remotegame = (net_addr != NULL);
    if(net_addr)
        data.LOCAL_CONNECT.net_addr = *net_addr;
    client_event_send(NULL, G_CLIENT_EVENT_LOCAL_CONNECT, &data);
}

void client_event_local_discoveryserver(void)
{
    client_event_send(NULL, G_CLIENT_EVENT_LOCAL_DICOVERYSERVER, NULL);
}

void client_event_local_newgame(const char * mapname)
{
    client_event_data_t data;
    strncpy(data.LOCAL_NEWGAME.mapname, mapname, MAP_NAME_SIZE);
    client_event_send(NULL, G_CLIENT_EVENT_LOCAL_NEWGAME, &data);
}

void client_event_local_loadgame(int isave)
{
    client_event_data_t data;
    data.LOCAL_LOADGAME.isave = isave;
    client_event_send(NULL, G_CLIENT_EVENT_LOCAL_LOADGAME, &data);
}

void client_event_local_world_recreate(const char * mapfilename)
{
    client_event_data_t data;
    strncpy(data.LOCAL_WORLD_RECREATE.mapfilename, mapfilename, MAP_FILENAME_SIZE);
    client_event_send(NULL, G_CLIENT_EVENT_LOCAL_WORLD_RECREATE, &data);
}

void client_event_local_entergame(void)
{
    client_event_send(NULL, G_CLIENT_EVENT_LOCAL_ENTERGAME, NULL);
}
