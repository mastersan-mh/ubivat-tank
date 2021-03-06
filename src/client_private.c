/*
 * client_private.c
 *
 *  Created on: 1 нояб. 2017 г.
 *      Author: mastersan
 */

#include "common/common_list2.h"
#include "client_private.h"
#include "video.h"
#include "world.h"

const char * client_gamestate_to_str(client_gamestate_t state)
{
    static const char *list[] =
    {
            "CLIENT_GAMESTATE_0_IDLE",
            "CLIENT_GAMESTATE_1_NOGAME",
            "CLIENT_GAMESTATE_2_WORLD_CREATING",
            "CLIENT_GAMESTATE_3_MISSION_BRIEF",
            "CLIENT_GAMESTATE_4_SPAWN_AWAITING",
            "CLIENT_GAMESTATE_5_GAMESAVE",
            "CLIENT_GAMESTATE_6_INGAME",
            "CLIENT_GAMESTATE_7_INTERMISSION",
            "CLIENT_GAMESTATE_8_ENDGAME",
    };
    return list[state];
}

void client_clean(void)
{
    if(client.sv_dedicated)
        world_destroy();
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
    int players_num = client.gstate.players_num;

    float cam_sx = (float)VIDEO_SCREEN_W * (float)VIDEO_SCALEX / (float)VIDEO_SCALE;
    float cam_sy = (float)VIDEO_SCREEN_H * (float)VIDEO_SCALEY / (float)VIDEO_SCALE;

    cam_sx /= players_num;

    float statusbar_h = 32.0f * (float)VIDEO_SCALEY / (float)VIDEO_SCALE;

    float border = 1.0;
    float x = border;
    size_t i;
    for(i = 0; i < CLIENT_PLAYERS_MAX; i++)
    {
        client_player_t * player = &client.players[i];
        if(!player->used)
            continue;

        VEC2_CLEAR(player->cam.origin);
        player->cam.x     = x;
        player->cam.y     = 0;
        player->cam.sx    = cam_sx - (border * 2.0);
        player->cam.sy    = cam_sy - statusbar_h;//184
        x = x + cam_sx;
    }

}

client_player_t * client_player_get(size_t iplayer)
{
    return &client.players[iplayer];
}

void client_player_delete(client_player_t * player)
{
    player->used = false;
}

void client_players_delete(void)
{
    size_t i;
    for(i = 0; i < CLIENT_PLAYERS_MAX; i++)
    {
        client_player_delete(&client.players[i]);
    }
}
