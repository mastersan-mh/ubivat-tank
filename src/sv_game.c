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
#include "ent_player.h"
#include "common_list2.h"

extern host_client_t * hclients;

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
	host_client_t * client;
	LIST2_FOREACH(hclients, client)
	{
		if( ((player_t *)client->entity->data)->items[ITEM_HEALTH] < 0 )
			alive = false;
	}
	if( alive )
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
				sv_state.state = GAMESTATE_INTERMISSION;
		}
	}
}

/*
 * главная процедура игры
 */
void sv_game_gameTick(void)
{
	entities_handle();
}

void sv_game_mainTick(void)
{
	static gamestate_t state_prev = GAMESTATE_NOGAME;
	if(state_prev != sv_state.state)
	{
		host_setgamestate(sv_state.state);
	}
	switch(sv_state.state)
	{
		case GAMESTATE_NOGAME:
			break;
		case GAMESTATE_MISSION_BRIEF:
			if(state_prev != sv_state.state)
			{
				sound_play_stop(sv_state.sound_playId);
				if(!sv_state.sound_playId)
					sv_state.sound_playId = sound_play_start(SOUND_MUSIC1, -1);
			}
			break;
		case GAMESTATE_GAMESAVE:
			break;
		case GAMESTATE_INGAME:
			if(state_prev != sv_state.state)
			{
				if(sv_state.sound_playId)
				{
					sound_play_stop(sv_state.sound_playId);
					sv_state.sound_playId = 0;
				}
			}
			sv_game_gameTick();
			break;
		case GAMESTATE_INTERMISSION:
			if(state_prev != sv_state.state)
			{
				sound_play_stop(sv_state.sound_playId);
				if(!sv_state.sound_playId)
					sv_state.sound_playId = sound_play_start(SOUND_MUSIC1, -1);
			}
			break;
	}
	state_prev = sv_state.state;

}



/*
 * сообщения об ошибках
 */
bool sv_game_nextmap(void)
{
	int ret;

	//закроем карту
	map_clear();
	sv_state.state = GAMESTATE_MISSION_BRIEF;


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
