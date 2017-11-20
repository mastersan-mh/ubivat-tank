/*
 * serve_fsm.h
 *
 *  Created on: 3 нояб. 2017 г.
 *      Author: mastersan
 */

#ifndef SRC_SERVER_FSM_H_
#define SRC_SERVER_FSM_H_

#include "net.h"
#include "server_events.h"

typedef enum
{
    SERVER_GAMESTATE_1_NOGAME, /* игра не создана */
    SERVER_GAMESTATE_2_INGAME,
} server_gamestate_t;

void server_fsm(const server_event_t * event);

#endif /* SRC_SERVER_FSM_H_ */
