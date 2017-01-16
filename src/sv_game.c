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

int sv_game_is_custom_game()
{
	return sv_state.flags & GAMEFLAG_CUSTOMGAME;
}

/*
 * первая карта
 */
int sv_game_is_first_map()
{
	return sv_state.gamemap == mapList;
}

void sv_game_win()
{
	sv_state.win = true;
	host_event_send_win();
}

/*
 * главная процедура игры
 */
void sv_game_gameTick()
{
	entities_handle();

	if( sv_state.win)
	{
		int i;
		bool alive = true;
		int num = host_client_num_get();
		// проверим что все игроки живы
		for(i = 0; i < num; i++)
		{
			if( ((player_t *)host_client_get(i)->entity->data)->items[ITEM_HEALTH] < 0 )
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
				server_store_clients_info();
				//игра по уровням
				if(sv_game_nextmap() == true)
					sv_state.state = GAMESTATE_INTERMISSION;
			}
		}
	}
}

void sv_game_mainTick()
{
	if(sv_state.state_prev != sv_state.state)
	{
		host_setgamestate(sv_state.state);
	}

	switch(sv_state.state)
	{
	case GAMESTATE_NOGAME:
		break;
	case GAMESTATE_INGAME:
		if(sv_state.state_prev != sv_state.state)
		{
			if(sv_state.sound_playId)
			{
				sound_play_stop(sv_state.sound_playId);
				sv_state.sound_playId = 0;
			}
		}
		sv_game_gameTick();
		break;
	case GAMESTATE_MISSION_BRIEF:
		if(sv_state.state_prev != sv_state.state)
		{
			sound_play_stop(sv_state.sound_playId);
			if(!sv_state.sound_playId)
				sv_state.sound_playId = sound_play_start(SOUND_MUSIC1, -1);
		}
		break;
	case GAMESTATE_INTERMISSION:
		if(sv_state.state_prev != sv_state.state)
		{
			sound_play_stop(sv_state.sound_playId);
			if(!sv_state.sound_playId)
				sv_state.sound_playId = sound_play_start(SOUND_MUSIC1, -1);
		}
		host_event_send_imenu(MENU_GAME_SAVE);
		break;
	}
	sv_state.state_prev = sv_state.state;

}



/*
 * сообщения об ошибках
 */
bool sv_game_nextmap()
{
	int ret;
	host_client_unspawn_id(0);
	host_client_unspawn_id(1);

	//закроем карту
	map_clear();
	sv_state.state = GAMESTATE_MISSION_BRIEF;
	sv_state.win   = false;
	//menu_interlevel();

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

	host_client_spawn_id(0);
	host_client_spawn_id(1);

	server_restore_clients_info();

	return true;
}
