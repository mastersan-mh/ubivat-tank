#include <actions.h>
#include "game.h"
#include <input.h>


#include <string.h>
#include <stdio.h>

#include <SDL2/SDL.h>
#include <GL/gl.h>
#include <player.h>


action_t actions[__ACTION_NUM] = {
		{ game_action_enter_mainmenu, NULL }, /* 	ACTION_ENTER_MAINMENU */
		{ player_moveUp_ON   , player_moveUp_OFF    },      /* ACTION_PLAYER_MOVEL */
		{ player_moveDown_ON , player_moveDown_OFF  },      /* ACTION_PLAYER_MOVER */
		{ player_moveLeft_ON , player_moveLeft_OFF  },      /* ACTION_PLAYER_MOVE_LEFT */
		{ player_moveRight_ON, player_moveRight_OFF },      /* ACTION_PLAYER_MOVE_RIGHT */
		{ player_attack_weapon1_ON, player_attack_weapon1_OFF }, /* ACTION_PLAYER_ATTACK_WEAPON1 */
		{ player_attack_weapon2_ON, player_attack_weapon2_OFF }, /* ACTION_PLAYER_ATTACK_WEAPON2 */
		{ player_attack_weapon3_ON, player_attack_weapon3_OFF }, /* ACTION_PLAYER_ATTACK_WEAPON3 */
		{ player2_moveUp_ON   , player2_moveUp_OFF }, /* ACTION_PLAYER2_MOVE_UP */
		{ player2_moveDown_ON , player2_moveDown_OFF }, /* ACTION_PLAYER2_MOVE_DOWN */
		{ player2_moveLeft_ON , player2_moveLeft_OFF }, /* ACTION_PLAYER2_MOVE_LEFT */
		{ player2_moveRight_ON, player2_moveRight_OFF }, /* ACTION_PLAYER2_MOVE_RIGHT */
		{ player2_attack_weapon1_ON, player2_attack_weapon1_OFF }, /* ACTION_PLAYER2_ATTACK_WEAPON1 */
		{ player2_attack_weapon2_ON, player2_attack_weapon2_OFF }, /* ACTION_PLAYER2_ATTACK_WEAPON2 */
		{ player2_attack_weapon3_ON, player2_attack_weapon3_OFF },  /* ACTION_PLAYER2_ATTACK_WEAPON3 */
		{ game_action_win, NULL }, /* ACTION_CHEAT_WIN */
		{ action_sfactor, NULL},
		{ action_dfactor, NULL}
};

typedef struct
{
	bool light;
	bool blend;
}scene_t;

scene_t scene = {};

void kp_0()
{
	scene.light = !scene.light;
	if(scene.light)
		glEnable(GL_LIGHTING);
	else
		glDisable(GL_LIGHTING);
	printf("scene.light = %d\n",scene.light);

}

void kp_B()
{
	scene.blend= !scene.blend;
	if(scene.blend)
	{
		glEnable(GL_BLEND);        // Включаем смешивание
		glDisable(GL_DEPTH_TEST);  // Выключаем тест глубины
	}
	else
	{
		glDisable(GL_BLEND);        // Выключаем смешивание
		glEnable(GL_DEPTH_TEST);    // Включаем тест глубины
	}
	printf("scene.blend = %d\n",scene.blend);
}

int factor_inc(int factor)
{
	factor++;
	return factor % 14;
}

void action_sfactor()
{
	game_video_sfactor = factor_inc(game_video_sfactor);
	printf("game_video_sfactor = %d\n", game_video_sfactor);
/*
	static GLuint fogMode[]= { GL_EXP, GL_EXP2, GL_LINEAR };
	static int fogfilter = 0;
	fogfilter+=1;
	if(fogfilter>3)fogfilter=0;
	glFogi (GL_FOG_MODE, fogMode[fogfilter]);
	printf("fogfilter = %d\n",fogfilter);
*/
}

void action_dfactor()
{
	game_video_dfactor = factor_inc(game_video_dfactor);
	printf("game_video_dfactor = %d\n", game_video_dfactor);
}



