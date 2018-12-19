/*
 * sv_game.c
 *
 *  Created on: 12 янв. 2017 г.
 *      Author: mastersan
 */

#include "sound.h"

#include "sv_game.h"

#include "game.h"
#include "game_progs.h"

#include "server.h"
#include "server_events.h"
#include "server_private.h"
#include "world.h"

#include "common/common_list2.h"

extern server_t server;

int sv_game_flag_localgame(void)
{
    return server.flags.localgame;
}

/*
 * первая карта
 */
int sv_game_is_first_map(void)
{
    return server.gstate.gamemap == mapList;
}

void sv_game_win(void)
{
    bool alive = true;
    // проверим что все игроки живы

    size_t i;
    server_player_t * player;
    FOREACH_SERVER_PLAYERS(player, i)
    {
        entity_common_t * common = player->body->entity;
        if( !common->alive )
        {
            alive = false;
            break;
        }
        if(!alive)
            break;
    }

    if( alive )
    {
        server_event_local_win();
    }
}
