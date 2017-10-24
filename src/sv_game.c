/*
 * sv_game.c
 *
 *  Created on: 12 янв. 2017 г.
 *      Author: mastersan
 */

#include "game.h"
#include "server.h"
#include "sv_game.h"
#include "sound.h"
#include "common/common_list2.h"

extern server_t * server;

extern server_state_t sv_state;

/*
 * прерывание игры
 */
void sv_game_abort(void)
{
	server_stop();
}

void sv_game_message_send(const char * mess)
{
	sv_state.msg = (char*)mess;
};

int sv_game_is_custom_game(void)
{
	return sv_state.flags & GAMEFLAG_CUSTOMGAME;
}

/*
 * первая карта
 */
int sv_game_is_first_map(void)
{
	return sv_state.gamemap == mapList;
}

void sv_game_win(void)
{
	bool alive = true;
	// проверим что все игроки живы

	server_client_t * client;
	LIST2_FOREACHR(server->clients, client)
	{
		server_player_t * player;
		LIST2_FOREACHR(client->players, player)
		{
			if( !player->entity->alive )
			{
				alive = false;
				break;
			}
			if(!alive)
				break;
		}
	}
	if( alive )
	{
		sv_state.state = GAMESTATE_INTERMISSION;
	}
}

/*
 * главная процедура игры
 */
void sv_game_gameTick(void)
{
	entities_handle();
}

#define GAME_SOUND_MENU 10

void sv_game_mainTick(void)
{
	static gamestate_t state_prev = GAMESTATE_NOGAME;
	bool statechanged = false;
	if(state_prev != sv_state.state)
	{
		server_setgamestate(sv_state.state);
		state_prev = sv_state.state;
		statechanged = true;
	}
	switch(sv_state.state)
	{
		case GAMESTATE_NOGAME:
			break;
		case GAMESTATE_MISSION_BRIEF:
			if(statechanged)
			{
				if(sound_started(NULL, GAME_SOUND_MENU))
					sound_play_stop(NULL, GAME_SOUND_MENU);
				sound_play_start(NULL, GAME_SOUND_MENU, SOUND_MUSIC1, -1);
			}
			break;
		case GAMESTATE_GAMESAVE:
			break;
		case GAMESTATE_INGAME:
			if(statechanged)
			{
				sound_play_stop(NULL, GAME_SOUND_MENU);
				sv_state.allow_state_gamesave = true;
			}
			sv_game_gameTick();
			break;
		case GAMESTATE_INTERMISSION:
			if(statechanged)
			{
				if(sv_state.flags & GAMEFLAG_CUSTOMGAME)
				{
					//игра по выбору
					sv_game_abort();
				}
				else
				{
					server_unjoin_clients();

					//игра по уровням
					if(sv_game_nextmap() == true)
					{
						//sv_state.state = GAMESTATE_INTERMISSION;
					}
				}

				if(sound_started(NULL, GAME_SOUND_MENU))
					sound_play_stop(NULL, GAME_SOUND_MENU);
				sound_play_start(NULL, GAME_SOUND_MENU, SOUND_MUSIC1, -1);
			}
			break;
	}

}



/*
 * сообщения об ошибках
 */
bool sv_game_nextmap(void)
{
	int ret;

	//закроем карту
	map_clear();

	sv_state.gamemap = sv_state.gamemap->next;
	if(!sv_state.gamemap)
	{
		// конец игры, последняя карта
		sv_game_abort();
		return false;
	}
	ret = map_load(sv_state.gamemap->map);
	if(ret)
	{
		game_msg_error(ret);
		sv_game_abort();
		return false;
	}

	return true;
}
