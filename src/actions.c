#include "actions.h"
#include "cl_game.h"
#include "game.h"
#include "cl_input.h"

#include <string.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <GL/gl.h>


typedef struct
{
	bool light;
	bool blend;
}scene_t;

scene_t scene = {};

void kp_0(const char * action)
{
	scene.light = !scene.light;
	if(scene.light)
		glEnable(GL_LIGHTING);
	else
		glDisable(GL_LIGHTING);
	printf("scene.light = %d\n",scene.light);

}

void kp_B(const char * action)
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

static void action_sfactor(const char * action)
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

static void action_dfactor(const char * action)
{
	game_video_dfactor = factor_inc(game_video_dfactor);
	printf("game_video_dfactor = %d\n", game_video_dfactor);
}



static game_action_t game_actions[] = {
        { "menu", game_action_showmenu }, /*    ACTION_ENTER_MAINMENU */
        { "+sfactor", action_sfactor},
        { "+dfactor", action_dfactor}
};


const game_action_t * game_action_find(const char * action_str)
{
    for(size_t i = 0; i < ARRAYSIZE(game_actions); i++)
    {
        game_action_t * action = &game_actions[i];
        if(ACTIONS_EQ(action->action, action_str))
        {
            return action;
        }
    }
    return NULL;
}


