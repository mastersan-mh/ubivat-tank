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

extern server_t server;

/*
 * прерывание игры
 */
void sv_game_abort(void)
{
    server_stop();
}

void sv_game_message_send(const char * mess)
{
    server.gamestate.msg = (char*)mess;
};

int sv_game_is_custom_game(void)
{
    return server.gamestate.flags & GAMEFLAG_CUSTOMGAME;
}

/*
 * первая карта
 */
int sv_game_is_first_map(void)
{
    return server.gamestate.gamemap == mapList;
}

void sv_game_win(void)
{
    bool alive = true;
    // проверим что все игроки живы

    server_client_t * client;
    LIST2_FOREACHR(server.clients, client)
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
        server.gamestate.state = GAMESTATE_INTERMISSION;
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
    if(state_prev != server.gamestate.state)
    {
        server_setgamestate(server.gamestate.state);
        state_prev = server.gamestate.state;
        statechanged = true;
    }
    switch(server.gamestate.state)
    {
    case GAMESTATE_NOGAME:
        break;
    case GAMESTATE_MISSION_BRIEF:
        break;
    case GAMESTATE_GAMESAVE:
        break;
    case GAMESTATE_INGAME:
        if(statechanged)
        {
            server.gamestate.allow_state_gamesave = true;
        }
        sv_game_gameTick();
        break;
    case GAMESTATE_INTERMISSION:
        if(statechanged)
        {
            if(server.gamestate.flags & GAMEFLAG_CUSTOMGAME)
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
                    //server.gamestate.state = GAMESTATE_INTERMISSION;
                }
            }
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

    server.gamestate.gamemap = server.gamestate.gamemap->next;
    if(!server.gamestate.gamemap)
    {
        // конец игры, последняя карта
        sv_game_abort();
        return false;
    }
    ret = map_load(server.gamestate.gamemap->map);
    if(ret)
    {
        game_msg_error(ret);
        sv_game_abort();
        return false;
    }

    return true;
}
