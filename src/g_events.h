/*
 * events.h
 *
 *  Created on: 28 дек. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_G_EVENTS_H_
#define SRC_G_EVENTS_H_

#include "entity.h"

#define GAME_EVENT_CONTROL_ACTION_LEN (63)
#define GAME_HOSTEVENT_ENTNAME_LEN (63)

typedef enum
{
	GCLIENTEVENT_CONNECT,
	GCLIENTEVENT_DISCONNECT,
	/* назначить игроку объект на карте */
	GCLIENTEVENT_JOIN,
	GCLIENTEVENT_GAMEABORT,
	GCLIENTEVENT_CONTROL,
	/* перейти на следующее состояние, если допустимо */
	GCLIENTEVENT_NEXTGAMESTATE,
} gclienteventtype_t;

typedef enum
{
	GHOSTEVENT_CONNECTION_ACCEPTED,
	GHOSTEVENT_CONNECTION_CLOSE,
	GHOSTEVENT_GAMESTATE,
	GHOSTEVENT_SETPLAYERENTITY,
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
	union
	{
		struct
		{
			char pad[1];
		} accepted;
		struct
		{
			uint8_t state;
		} gamestate;
		struct
		{
			uint8_t imenu;
		} imenu;
		struct
		{
			char entityname[GAME_HOSTEVENT_ENTNAME_LEN + 1];
			entity_t * entity;
		} setplayerentity;
	};
} ghostevent_t;

#endif /* SRC_G_EVENTS_H_ */
