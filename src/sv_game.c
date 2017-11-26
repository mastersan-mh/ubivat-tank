/*
 * sv_game.c
 *
 *  Created on: 12 янв. 2017 г.
 *      Author: mastersan
 */

#include "sound.h"

#include "sv_game.h"

#include "game.h"

#include "server.h"
#include "server_events.h"
#include "server_private.h"
#include "world.h"

#include "common/common_list2.h"

extern server_t server;

void sv_game_message_send(const char * mess)
{
    server.gstate.msg = (char*)mess;
};

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

    server_client_t * client;
    LIST2_FOREACHR(server.clients, client)
    {
        server_player_t * player;
        LIST2_FOREACHR(client->players, player)
        {
            entity_vars_common_t * common = player->entity->vars;
            if( !common->alive )
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
        server_event_local_win();
    }
}

/*
 * сообщения об ошибках
 */
int sv_game_nextmap(void)
{
    world_destroy();

    server.gstate.gamemap = server.gstate.gamemap->next;
    if(!server.gstate.gamemap)
    {
        // конец игры, последняя карта
        server_event_local_stop();
        return -1;
    }
    if(server_world_recreate(server.gstate.gamemap->filename))
    {
        server_event_local_stop();
        return -1;
    }

    return 0;
}
