#include <actions.h>
#include <game.h>
#include <input.h>


#include <string.h>
#include <stdio.h>

#include <SDL2/SDL.h>
#include <GL/gl.h>
#include <player.h>


action_t action[ACTION_NUM] = {

		{player_moveUp_ON   , player_moveUp_OFF }, /* ACTION_PLAYER_MOVEL */
		{player_moveDown_ON , player_moveDown_OFF}, /* ACTION_PLAYER_MOVER */
		{player_moveLeft_ON , player_moveLeft_OFF }, /* ACTION_PLAYER_MOVE_LEFT */
		{player_moveRight_ON, player_moveRight_OFF}, /* ACTION_PLAYER_MOVE_RIGHT */
};

void action_init()
{
	input_key_bindAction(SDLK_UP   , action[ACTION_PLAYER_MOVE_UP]);
	input_key_bindAction(SDLK_DOWN , action[ACTION_PLAYER_MOVE_DOWN]);
	input_key_bindAction(SDLK_LEFT , action[ACTION_PLAYER_MOVE_LEFT]);
	input_key_bindAction(SDLK_RIGHT, action[ACTION_PLAYER_MOVE_RIGHT]);
}

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

void action_fog()
{
	static GLuint fogMode[]= { GL_EXP, GL_EXP2, GL_LINEAR };
	static int fogfilter = 0;
	fogfilter+=1;
	if(fogfilter>3)fogfilter=0;
	glFogi (GL_FOG_MODE, fogMode[fogfilter]);
	printf("fogfilter = %d\n",fogfilter);
}

void action_kp8()
{

}



