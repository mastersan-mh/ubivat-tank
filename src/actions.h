#ifndef ACTIONS_H
#define ACTIONS_H

#include <types.h>

enum action
{
	ACTION_ENTER_MAINMENU,
	ACTION_PLAYER_MOVE_UP,
	ACTION_PLAYER_MOVE_DOWN,
	ACTION_PLAYER_MOVE_LEFT,
	ACTION_PLAYER_MOVE_RIGHT,
	ACTION_PLAYER_ATTACK_WEAPON1,
	ACTION_PLAYER_ATTACK_WEAPON2,
	ACTION_PLAYER_ATTACK_WEAPON3,
	ACTION_PLAYER2_MOVE_UP,
	ACTION_PLAYER2_MOVE_DOWN,
	ACTION_PLAYER2_MOVE_LEFT,
	ACTION_PLAYER2_MOVE_RIGHT,
	ACTION_PLAYER2_ATTACK_WEAPON1,
	ACTION_PLAYER2_ATTACK_WEAPON2,
	ACTION_PLAYER2_ATTACK_WEAPON3,
	ACTION_CHEAT_WIN,
	ACTION_SFACTOR,
	ACTION_DFACTOR,
	ACTION____NUM
};

struct player_action
{
	actionf_t press;
	actionf_t release;
};

extern const struct player_action actions[ACTION____NUM];

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
