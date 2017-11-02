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
void client_event_local_key_input(int key, bool state_pressed)
{
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

void client_events_pump(void)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        //printf("event.type = %d\n", event.type);
        switch(event.type)
        {
            case SDL_KEYDOWN:
                if(!event.key.repeat)
                    client_event_local_key_input(event.key.keysym.scancode, true);
                break;
            case SDL_KEYUP:
                if(!event.key.repeat)
                    client_event_local_key_input(event.key.keysym.scancode, false);
                break;
        }
        //player_checkcode();
    }
}
