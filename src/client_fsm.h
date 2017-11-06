/*
 * client_fsm.h
 *
 *  Created on: 1 нояб. 2017 г.
 *      Author: mastersan
 */

#ifndef SRC_CLIENT_FSM_H_
#define SRC_CLIENT_FSM_H_

#include "client_events.h"

typedef enum
{
    CLIENT_GAMESTATE_1_NOGAME, /* игра не создана */
    CLIENT_GAMESTATE_2_MISSION_BRIEF,
    CLIENT_GAMESTATE_3_SPAWN_AWAITING,
    CLIENT_GAMESTATE_4_GAMESAVE,
    CLIENT_GAMESTATE_5_INGAME,
    CLIENT_GAMESTATE_6_INTERMISSION,
} client_gamestate_t;

extern void client_fsm(const game_client_event_t * event);


#endif /* SRC_CLIENT_FSM_H_ */
