/*
 * server_events.c
 *
 *  Created on: 3 нояб. 2017 г.
 *      Author: mastersan
 */

#include "server_events.h"
#include "server_fsm.h"

void server_event_local_win()
{
    game_server_event_t event;
    event.sender = NULL;
    event.type = G_SERVER_EVENT_LOCAL_WIN;
    server_fsm(&event);
}
