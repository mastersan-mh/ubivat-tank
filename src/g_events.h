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
	GEVENT_CLIENT_MSG_CONNECT,
	GEVENT_CONTROL
} geventtype_t;

/*
#define CLIENT_MSG_CONNECT  0x01
//#define CLIENT_MSG_SPAWN_ME 0x02
#define CLIENT_MSG_PLAYER_ACTION 0x03
*/

typedef struct
{
	/* тип события, geventtype_t */
	geventtype_t type;
	union
	{
		struct
		{
			char action[GAME_EVENT_CONTROL_ACTION_LEN + 1];
		} control;
	};

} gevent_t;

#endif /* SRC_G_EVENTS_H_ */
