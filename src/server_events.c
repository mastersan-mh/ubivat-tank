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
    while(!TAILQ_EMPTY(&server.events))
    {
        game_server_event_entry_t * ev = TAILQ_FIRST(&server.events);
        TAILQ_REMOVE(&server.events, ev, entry);
        server_fsm(&ev->ev);
        Z_free(ev);
    }
}

void server_event_send(const game_server_event_t * event)
{
    game_server_event_entry_t * ev = Z_malloc(sizeof(game_server_event_entry_t));
    ev->ev = *event;
    TAILQ_INSERT_TAIL(&server.events, ev, entry);
}

/*
 * прерывание игры
 */
void server_event_local_stop(void)
{
    game_server_event_t event;
    event.type = G_SERVER_EVENT_LOCAL_STOP;
    server_event_send(&event);
}

void server_event_local_win()
{
    game_server_event_t event;
    event.type = G_SERVER_EVENT_LOCAL_WIN;
    server_event_send(&event);
}
