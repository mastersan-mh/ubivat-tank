/*
 * server_events.c
 *
 *  Created on: 3 нояб. 2017 г.
 *      Author: mastersan
 */

#include "server_events.h"
#include "server_fsm.h"
#include "server_private.h"

void server_events_handle(void)
{
    while(!CIRCLEQ_EMPTY(&server.events))
    {
        server_event_t * event = CIRCLEQ_FIRST(&server.events);
        CIRCLEQ_REMOVE(&server.events, event, queue);
        server_fsm(event);
        Z_free(event);
    }
}

void server_event_send(
    const net_addr_t * sender,
    server_event_type_t type,
    const server_event_data_t * data)
{
    server_event_t * event = Z_malloc(sizeof(server_event_t));
    if(sender)
        event->sender = *sender;
    event->type = type;
    if(data)
        event->data = *data;

    CIRCLEQ_INSERT_TAIL(&server.events, event, queue);
}

/*
 * прерывание игры
 */
void server_event_local_stop(void)
{
    server_event_send(
        NULL,
        G_SERVER_EVENT_LOCAL_STOP,
        NULL
    );
}

void server_event_local_win()
{
    server_event_send(
        NULL,
        G_SERVER_EVENT_LOCAL_WIN,
        NULL
    );
}
