#ifndef ACTIONS_H
#define ACTIONS_H

#include "eng_actions.h"

#include "types.h"

#define ACTION_ENTER_MAINMENU
#define ACTION_PLAYER_MOVE_UP                   "+move_north"
#define ACTION_PLAYER_MOVE_DOWN                 "+move_south"
#define ACTION_PLAYER_MOVE_LEFT                 "+move_west"
#define ACTION_PLAYER_MOVE_RIGHT                "+move_east"
#define ACTION_PLAYER_ATTACK_WEAPON1            "+attack_artillery"
#define ACTION_PLAYER_ATTACK_WEAPON2            "+attack_missile"
#define ACTION_PLAYER_ATTACK_WEAPON3            "+attack_mine"
#define ACTION_PLAYER2_MOVE_UP                  "+p2move_north"
#define ACTION_PLAYER2_MOVE_DOWN                "+p2move_south"
#define ACTION_PLAYER2_MOVE_LEFT                "+p2move_west"
#define ACTION_PLAYER2_MOVE_RIGHT               "+p2move_east"
#define ACTION_PLAYER2_ATTACK_WEAPON1           "+p2attack_artillery"
#define ACTION_PLAYER2_ATTACK_WEAPON2           "+p2attack_missile"
#define ACTION_PLAYER2_ATTACK_WEAPON3           "+p2attack_mine"
#define ACTION_CHEAT_WIN
#define ACTION_SFACTOR
#define ACTION_DFACTOR
#define ACTION____NUM  18

extern const struct eng_game_action actions[];

void action_sfactor(void);
void action_dfactor(void);

void kp_0(void);
void kp_B(void);

void action_zoom_in(void);
void action_zoom_out(void);
void action_tilt_in(void);
void action_tilt_out(void);
void action_spin_in(void);
void action_spin_out(void);
void action_twinkle(void);
void action_fog(void);
void action_switch_obj(void);



#endif // ACTIONS_H
