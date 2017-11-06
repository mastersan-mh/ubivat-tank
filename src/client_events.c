/*
 * client_events.c
 *
 *  Created on: 1 нояб. 2017 г.
 *      Author: mastersan
 */

#include "client_events.h"
#include "client_fsm.h"

/**
 * state = true | false = pressed | released
 */
void client_event_local_key_input(bool key_repeat, int key, bool state_pressed)
{
    if(key_repeat)
        return;
    game_client_event_t event;

    if(state_pressed)
    {
        event.type = G_CLIENT_EVENT_LOCAL_KEY_PRESS;
        event.data.LOCAL_KEY_PRESS.key = key;
    }
    else
    {
        event.type = G_CLIENT_EVENT_LOCAL_KEY_RELEASE;
        event.data.LOCAL_KEY_RELEASE.key = key;
    }

    client_fsm(&event);
}

void client_event_local_entergame()
{
    game_client_event_t event;
    event.type = G_CLIENT_EVENT_LOCAL_ENTERGAME;
    client_fsm(&event);
}
