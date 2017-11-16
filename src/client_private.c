/*
 * client_private.c
 *
 *  Created on: 1 нояб. 2017 г.
 *      Author: mastersan
 */

#include "common/common_list2.h"
#include "client_private.h"
#include "video.h"

const char * client_gamestate_to_str(client_gamestate_t state)
{
    static const char *list[] =
    {
            "CLIENT_GAMESTATE_1_NOGAME",
            "CLIENT_GAMESTATE_2_MISSION_BRIEF",
            "CLIENT_GAMESTATE_3_SPAWN_AWAITING",
            "CLIENT_GAMESTATE_4_GAMESAVE",
            "CLIENT_GAMESTATE_5_INGAME",
            "CLIENT_GAMESTATE_6_INTERMISSION",
    };
    return list[state];
}


void client_clean(void)
{
    client_req_send_game_abort();
    client_players_delete();
    /* flush queue */
    client.tx_queue_num = 0;
    game_menu_show(MENU_MAIN);
}

void client_disconnect()
{

}

/**
 * @brief инициализация камер игроков клиента
 */
void client_initcams(void)
{
    client_player_t * player;
    int players_num = client.gstate.players_num;

    float cam_sx = (float)VIDEO_SCREEN_W * (float)VIDEO_SCALEX / (float)VIDEO_SCALE;
    float cam_sy = (float)VIDEO_SCREEN_H * (float)VIDEO_SCALEY / (float)VIDEO_SCALE;

    cam_sx /= players_num;

    float statusbar_h = 32.0f * (float)VIDEO_SCALEY / (float)VIDEO_SCALE;

    float border = 1.0;
    float x = border;
    LIST2_FOREACH(client.players, player)
    {
        VEC2_CLEAR(player->cam.origin);

        player->cam.x     = x;
        player->cam.y     = 0;
        player->cam.sx    = cam_sx - (border * 2.0);
        player->cam.sy    = cam_sy - statusbar_h;//184
        x = x + cam_sx;
    }

}

int client_player_num_get(void)
{
    client_player_t * player;
    size_t clients_num = 0;
    LIST2_FOREACH_I(client.players, player, clients_num);
    return clients_num;
}

client_player_t * client_player_get(int playerId)
{
    int num, i;
    num = client_player_num_get();
    if(playerId < 0 || playerId >= num)
        return NULL;
    client_player_t * player;
    LIST2_LIST_TO_IENT(client.players, player, i, num - 1 - playerId);
    return player;
}

void client_player_delete(client_player_t * player)
{
    Z_free(player);
}

void client_players_delete(void)
{
    client_player_t * player;
    while(!LIST2_IS_EMPTY(client.players))
    {
        player = client.players;
        LIST2_UNLINK(client.players, player);
        client_player_delete(player);
    }
}
