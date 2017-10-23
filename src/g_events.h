/*
 * events.h
 *
 *  Created on: 28 дек. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_G_EVENTS_H_
#define SRC_G_EVENTS_H_

#include "map.h"
#include "game.h"

#define GAME_HOSTEVENT_ENTNAME_SIZE (64)
#define GAME_CLIENT_PLAYER_REQUEST_CONTROL_ACTION_SIZE (64)

/** @brief Запросы клиента на сервер */
typedef enum
{
	/** Непривилегированые запросы */
	G_CLIENT_REQ_DISCOVERYSERVER, /**< найти серверы */
	G_CLIENT_REQ_CONNECT,
	G_CLIENT_REQ_DISCONNECT,
	G_CLIENT_REQ_JOIN,     /**< назначить игроку объект на карте */
	/** Привилегированные запросы */
#define G_CLIENT_REQ_PRIVILEGED G_CLIENT_REQ_GAME_ABORT
	G_CLIENT_REQ_GAME_ABORT,
	G_CLIENT_REQ_GAME_SETMAP,
	G_CLIENT_REQ_GAME_NEXTSTATE, /**< перейти на следующее состояние, если допустимо */
	G_CLIENT_REQ_GAME_SAVE,
	G_CLIENT_REQ_GAME_LOAD,
} game_client_request_type_t;

typedef union
{
	struct
	{
		int isave;
	} GAME_SAVE;
	struct
	{
		int isave;
	} GAME_LOAD;
	struct
	{
		char mapname[MAP_FILENAME_SIZE];
	} GAME_SETMAP;
} game_client_request_data_t;

typedef struct
{
	game_client_request_type_t req;
	game_client_request_data_t data;
} game_client_request_t;

/* Управление игроком */
typedef enum
{
	G_CLIENT_PLAYER_REQ_NONE, /**< нет запроса */
	G_CLIENT_PLAYER_REQ_CONTROL,
} game_client_player_request_type_t;

typedef struct
{
	struct
	{
		char action[GAME_CLIENT_PLAYER_REQUEST_CONTROL_ACTION_SIZE];
	} CONTROL;
} game_client_player_request_data_t;


/* тип события */
typedef enum
{
	GHOSTEVENT_INFO, /* информация о срвере на запрос GCLIENTEVENT_DISCOVERYSERVER */
	GHOSTEVENT_CONNECTION_ACCEPTED,
	GHOSTEVENT_CONNECTION_CLOSE,
	GHOSTEVENT_GAMESTATE,
	GHOSTEVENT_SETPLAYERENTITY,
	GHOSTEVENT_GAMESAVE_LOADED,
} game_server_request_type_t;

typedef union
{
	struct
	{
		int clients_num; /* количество клиентов на сервере */
	} info;
	struct
	{
		char pad[1];
	} accepted;
	struct
	{
		gamestate_t state;
	} gamestate;
	struct
	{
		uint8_t imenu;
	} imenu;
	struct
	{
		char entityname[GAME_HOSTEVENT_ENTNAME_SIZE];
		void /*entity_t */ * entity;
	} setplayerentity;
	struct
	{
		int flags;
	} gamesave_loaded;
} game_server_request_data_t;


typedef struct
{
	game_server_request_type_t req;
	game_server_request_data_t data;
} game_server_request_t;


#endif /* SRC_G_EVENTS_H_ */
