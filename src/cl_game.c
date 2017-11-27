/*
 * cl_game.c
 *
 *  Created on: 12 янв. 2017 г.
 *      Author: mastersan
 */

#include "common/common_list2.h"

#include "types.h"
#include "world.h"

#include "client.h"
#include "client_private.h"

/* TODO: erase line */
#include "server.h"

#include "_gr2D.h"
#include "video.h"
#include "ui.h"
#include "game.h"

static void client_game_draw_nogame(void)
{
    video_image_draw(0, 0, IMG_MENU_I_INTERLV);
    video_printf(160 - 6 * 10 , 8 * 5, "Ожидание соединения...");
}

static void client_game_draw_world_creating(void)
{
    video_image_draw(0, 0, IMG_MENU_I_INTERLV);
    font_color_set3i(COLOR_15);
    video_printf(160-06*4 ,8*5, "СОЗДАНИЕ МИРА...");
}

/**
 * информация об уровне
 */
static void client_game_draw_missionbrief(void)
{
    map_t * map = world_map_get();

    video_image_draw(0, 0, IMG_MENU_I_INTERLV);
    font_color_set3i(COLOR_15);
    video_printf(160-06*4 ,8*5, "КАРТА:");
    font_color_set3i(COLOR_15);
    video_printf(160-16*4, 8*7 , map->mapname);
    video_printf(160-07*4, 8*10, "ЗАДАЧА:");
    video_printf(108     , 191 , "НАЖМИ ПРОБЕЛ");
    video_printf_wide(160 - 8 * 8, 8 * 12, 8 * 16, map->mapbrief);
}

/**
 * информация об уровне
 */
static void client_game_draw_spawn_awaiting(void)
{
    video_image_draw(0, 0, IMG_MENU_I_INTERLV);
    font_color_set3i(COLOR_15);
    video_printf(160 - 6 * 10 , 8 * 5, "Перемещаемся в мир...");
}

/*
 * заставка между уровнями
 */
static void cl_game_state_intermission_draw(void)
{
    static image_index_t list[] =
    {
            IMG_HUD_ICON_TANK0,
            IMG_HUD_ICON_TANK1,
            IMG_HUD_ICON_TANK2,
            IMG_HUD_ICON_TANK3,
            IMG_HUD_ICON_TANK4
    };
    video_image_draw(0, 0, IMG_MENU_I_INTERLV);
    font_color_set3i(COLOR_15);
    video_printf(108,191,"НАЖМИ ПРОБЕЛ");
/* TODO cl_game_state_intermission_draw
    int i;
    int num = server_client_num_get();

    int refy;
    if(num == 0) refy = 84;
    else refy = 76;

    font_color_set3i(COLOR_15);
    video_printf(48 + 8 * 00, refy      , "ОЧКИ      ФРАГИ      ВСЕГО ФРАГОВ");

    for(i = 0; i < num; i++)
    {
        struct server_client_s * client = server_client_get(i);
        if(client)
        {
            server_player_t * player;
            LIST2_FOREACHR(client->players, player)
            {
                long level = server_client_vardata_get(player, "level", VARTYPE_INTEGER)->value.i64;
                video_image_draw(26, refy + 8 + 16 * i, list[level]);
                font_color_set3i(COLOR_15);
                video_printf(48 + 8 *  0, refy + 8 + 4 + 16 * i, "%ld", server_client_vardata_get(player, "scores", VARTYPE_INTEGER)->value.i64);
                video_printf(48 + 8 * 10, refy + 8 + 4 + 16 * i, "%ld", server_client_vardata_get(player, "frags", VARTYPE_INTEGER)->value.i64);
                video_printf(48 + 8 * 21, refy + 8 + 4 + 16 * i, "%ld", server_client_vardata_get(player, "fragstotal", VARTYPE_INTEGER)->value.i64);
            }
        }
    }
*/
}

static void cl_game_state_endgame_draw(void)
{
    video_image_draw(0, 0, IMG_MENU_I_INTERLV);
    video_printf(160 - 6 * 10 , 8 * 5, "КОНЕЦ.");
}


static void client_game_draw_cam(camera_t * cam)
{
    map_render(world_map_get(), cam);
}

static void cl_draw(void)
{
    client_player_t * player;

    if(!world_valid())
        return;


    LIST2_FOREACH(client.players, player)
    {
        entity_t * entity = player->entity;
        if(entity)
        {
            camera_t * cam = &player->cam;

            video_viewport_set(
                cam->x,
                cam->y,
                cam->sx,
                cam->sy
            );
            if(entity->cam_entity)
            {
                VEC2_COPY(cam->origin, ((entity_vars_common_t*)(entity->cam_entity->vars))->origin);
            }
            else
            {
                static vec2_t cam_origin_default = {0.0, 0.0};
                VEC2_COPY(cam->origin, cam_origin_default);
            }
            client_game_draw_cam(cam);
            ui_draw(cam, entity);
        }
    }

    video_viewport_set(
        0.0f,
        0.0f,
        VIDEO_SCREEN_W,
        VIDEO_SCREEN_H
    );

    if(client.gstate.msg)
    {
        font_color_set3i(COLOR_1);
        video_printf_wide(96, 84, 128, client.gstate.msg);
        client.gstate.msg = NULL;
    };

    /* TODO
    extern server_t server;
    if(server.gamestate.msg)
    {
        font_color_set3i(COLOR_1);
        video_printf_wide(96, 84, 128, server.gamestate.msg);
        server.gamestate.msg = NULL;
    };
    */


}


void client_game_draw(void)
{
    switch(client.gamestate)
    {
        case CLIENT_GAMESTATE_0_IDLE:
            break;
        case CLIENT_GAMESTATE_1_NOGAME:
            client_game_draw_nogame();
            break;
        case CLIENT_GAMESTATE_2_WORLD_CREATING:
            client_game_draw_world_creating();
            break;
        case CLIENT_GAMESTATE_3_MISSION_BRIEF:
            client_game_draw_missionbrief();
            break;
        case CLIENT_GAMESTATE_4_SPAWN_AWAITING:
            client_game_draw_spawn_awaiting();
            break;
        case CLIENT_GAMESTATE_5_GAMESAVE:
            break;
        case CLIENT_GAMESTATE_6_INGAME:
            cl_draw();
            break;
        case CLIENT_GAMESTATE_7_INTERMISSION:
            cl_game_state_intermission_draw();
            break;
        case CLIENT_GAMESTATE_8_ENDGAME:
            cl_game_state_endgame_draw();
            break;
    }
}

