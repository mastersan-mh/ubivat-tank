/*
 * cl_game.c
 *
 *  Created on: 12 янв. 2017 г.
 *      Author: mastersan
 */

#include "common_list2.h"
#include "client.h"
#include "_gr2D.h"
#include "video.h"
#include "ui.h"
#include "game.h"

#include "ent_player.h"

extern client_t * clients;

bool cl_game_quit_get(void)
{
	return cl_state.quit;
}

void cl_game_quit_set(void)
{
	cl_state.quit = true;
}

void cl_game_action_showmenu(void)
{
	cl_state.show_menu = true;
}

/*
 * создание игры
 *
 * @return = 0 - игра создана успешно
 * @return = 1 - игра уже создана
 * @return = 2 - ошибка создания серверного игрока
 * @return = 3 - ошибка создания серверного игрока
 */
int cl_game_create(int flags)
{
	server_start(flags);
	cl_state.show_menu = false;
	return 0;
}

void cl_game_abort(void)
{
	client_event_gameabort_send();
}

/*
 * информация об уровне
 */
static void cl_game_state_missionbrief_draw(void)
{
	gr2D_setimage0(0, 0, image_get(IMG_MENU_I_INTERLV));
	font_color_set3i(COLOR_15);
	video_printf(160-06*4 ,8*5, "КАРТА:");
	font_color_set3i(COLOR_15);
	video_printf(160-16*4, 8*7 , map.name);
	video_printf(160-07*4, 8*10, "ЗАДАЧА:");
	video_printf(108     , 191 , "НАЖМИ ПРОБЕЛ");
	video_printf_wide(160 - 8 * 8, 8 * 12, 8 * 16, map.brief);
}

/*
 * заставка между уровнями
 */
static void cl_game_state_intermission_draw(void)
{
	static image_index_t list[] =
	{
			IMG_TANK0,
			IMG_TANK1,
			IMG_TANK2,
			IMG_TANK3,
			IMG_TANK4
	};
	gr2D_setimage0(0, 0, image_get(IMG_MENU_I_INTERLV));
	font_color_set3i(COLOR_15);
	video_printf(108,191,"НАЖМИ ПРОБЕЛ");

	int i;
	int num = host_client_num_get();

	int refy;
	if(num == 0) refy = 84;
	else refy = 76;

	font_color_set3i(COLOR_15);
	video_printf(48 + 8 * 00, refy      , "ОЧКИ      ФРАГИ      ВСЕГО ФРАГОВ");
/*
	client_t * client;
	LIST2_FOREACHR(clients, client)
*/
	for(i = 0; i < num; i++)
	{
		host_client_t * client = host_client_get(i);
		if(client->varsdata)
		{
			long level = sv_client_storedvars_get(client, "level")->value.i64;
			item_img_t * img = image_get(list[level]);
			gr2D_setimage1(26       , refy + 8 +     16 * i, img, 0, 0, c_p_MDL_box, c_p_MDL_box);
			font_color_set3i(COLOR_15);
			video_printf(48 + 8 *  0, refy + 8 + 4 + 16 * i, "%ld", sv_client_storedvars_get(client, "scores")->value.i64);
			video_printf(48 + 8 * 10, refy + 8 + 4 + 16 * i, "%ld", sv_client_storedvars_get(client, "frags")->value.i64);
			video_printf(48 + 8 * 21, refy + 8 + 4 + 16 * i, "%ld", sv_client_storedvars_get(client, "fragstotal")->value.i64);
		}
	}
}

static void client_game_draw_cam(camera_t * cam, entity_t * player)
{

	player_t * pl = player->data;

	if(pl->bull)
	{
		cam->pos.x = pl->bull->pos.x;
		cam->pos.y = pl->bull->pos.y;
	}
	else
	{
		cam->pos.x = player->pos.x;
		cam->pos.y = player->pos.y;
	}

	map_draw(cam);
}

static void cl_draw(void)
{
	client_t * client;



	LIST2_FOREACH(clients, client)
	{
		entity_t * entity = client->entity;
		if(entity)
		{
			camera_t * cam = &client->cam;

			video_viewport_set(
				cam->x,
				cam->y,
				cam->sx,
				cam->sy
			);

			client_game_draw_cam(cam, entity);
			ui_draw(cam, entity);
		}
	}

	video_viewport_set(
		0.0f,
		0.0f,
		VIDEO_SCREEN_W,
		VIDEO_SCREEN_H
	);

	if(cl_state.msg)
	{
		font_color_set3i(COLOR_1);
		video_printf_wide(96, 84, 128, cl_state.msg);
		cl_state.msg = NULL;
	};


}


void cl_game_draw(void)
{
	switch(cl_state.state)
	{
	case GAMESTATE_NOGAME:
		break;
	case GAMESTATE_MISSION_BRIEF:
		cl_game_state_missionbrief_draw();
		break;
	case GAMESTATE_GAMESAVE:
		break;
	case GAMESTATE_INGAME:
		cl_draw();
		break;
	case GAMESTATE_INTERMISSION:
		cl_game_state_intermission_draw();
		break;
	}
}

