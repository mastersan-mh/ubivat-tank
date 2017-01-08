/*
 * events.h
 *
 *  Created on: 28 дек. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_G_EVENTS_H_
#define SRC_G_EVENTS_H_

#define GAME_EVENT_CONTROL_ACTION_LEN (63)

typedef enum
{
	GCLIENTEVENT_CONNECT,
	GCLIENTEVENT_CONTROL
} gclienteventtype_t;

typedef enum
{
	GHOSTEVENT_CONNECTION_ACCEPTED,
	GHOSTEVENT_CONNECTION_CLOSE,
} ghosteventtype_t;
/*
#define CLIENT_MSG_CONNECT  0x01
//#define CLIENT_MSG_SPAWN_ME 0x02
#define CLIENT_MSG_PLAYER_ACTION 0x03
*/

typedef struct
{
	/* тип события */
	gclienteventtype_t type;
	union
	{
		struct
		{
			char action[GAME_EVENT_CONTROL_ACTION_LEN + 1];
		} control;
	};

} gclientevent_t;

typedef struct
{
	/* тип события */
	ghosteventtype_t type;
} ghostevent_t;

#endif /* SRC_G_EVENTS_H_ */
