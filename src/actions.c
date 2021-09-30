#include <actions.h>
#include "game.h"
#include <input.h>


#include <string.h>
#include <stdio.h>

#include <SDL2/SDL.h>
#include <GL/gl.h>
#include <player.h>


const struct eng_game_action actions[] =
{
        { "menu_main"  , game_action_enter_mainmenu},
        { "win"    , game_action_win},
        { "sfactor", action_sfactor },
        { "dfactor", action_dfactor },
        { "cam_x_inc"             , orig_x_inc           },
        { "cam_x_dec"             , orig_x_dec           },
        { "cam_y_inc"             , orig_y_inc           },
        { "cam_y_dec"             , orig_y_dec           },
        { "cam_z_inc"             , orig_z_inc           },
        { "cam_z_dec"             , orig_z_dec           },
        { "obj_ang_inc"           , orig_ang_inc           },
        { "obj_ang_dec"           , orig_ang_dec           },
        { ACTION_PLAYER_MOVE_UP   , player_moveUp_ON     },
        { "-move_north"           , player_moveUp_OFF    },
        { ACTION_PLAYER_MOVE_DOWN , player_moveDown_ON   },
        { "-move_south"           , player_moveDown_OFF  },
        { ACTION_PLAYER_MOVE_LEFT , player_moveLeft_ON   },
        { "-move_west"            , player_moveLeft_OFF  },
        { ACTION_PLAYER_MOVE_RIGHT, player_moveRight_ON  },
        { "-move_east"            , player_moveRight_OFF },
        { ACTION_PLAYER_ATTACK_WEAPON1, player_attack_weapon1_ON  },
        { "-attack_artillery"         , player_attack_weapon1_OFF },
        { ACTION_PLAYER_ATTACK_WEAPON2, player_attack_weapon2_ON  },
        { "-attack_missile"           , player_attack_weapon2_OFF },
        { ACTION_PLAYER_ATTACK_WEAPON3, player_attack_weapon3_ON  },
        { "-attack_mine" , player_attack_weapon3_OFF },
        { ACTION_PLAYER2_MOVE_UP   , player2_moveUp_ON  },
        { "-p2move_north"          , player2_moveUp_OFF },
        { ACTION_PLAYER2_MOVE_DOWN , player2_moveDown_ON  },
        { "-p2move_south"          , player2_moveDown_OFF },
        { ACTION_PLAYER2_MOVE_LEFT , player2_moveLeft_ON  },
        { "-p2move_west"           , player2_moveLeft_OFF },
        { ACTION_PLAYER2_MOVE_RIGHT, player2_moveRight_ON  },
        { "-p2move_east"           , player2_moveRight_OFF },
        { ACTION_PLAYER2_ATTACK_WEAPON1, player2_attack_weapon1_ON  },
        { "-p2attack_artillery"        , player2_attack_weapon1_OFF },
        { ACTION_PLAYER2_ATTACK_WEAPON2, player2_attack_weapon2_ON  },
        { "-p2attack_missile"          , player2_attack_weapon2_OFF },
        { ACTION_PLAYER2_ATTACK_WEAPON3, player2_attack_weapon3_ON  },
        { "-p2attack_mine"             , player2_attack_weapon3_OFF },
};

typedef struct
{
	bool light;
	bool blend;
}scene_t;

scene_t scene = {};

void kp_0(void)
{
	scene.light = !scene.light;
	if(scene.light)
		glEnable(GL_LIGHTING);
	else
		glDisable(GL_LIGHTING);
	printf("scene.light = %d\n",scene.light);

}

void kp_B(void)
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

static int factor_inc(int factor)
{
	factor++;
	return factor % 14;
}

void action_sfactor(void)
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

void action_dfactor(void)
{
	game_video_dfactor = factor_inc(game_video_dfactor);
	printf("game_video_dfactor = %d\n", game_video_dfactor);
}



