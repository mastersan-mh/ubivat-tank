#ifndef ACTIONS_H
#define ACTIONS_H

#include "types.h"

typedef enum
{
	ACTION_ENTER_MAINMENU,
	ACTION_SFACTOR,
	ACTION_DFACTOR,
	ACTION_NUM
}actions_t;

typedef struct
{
	actionf_t press;
	actionf_t release;
}action_t;

extern action_t actions[ACTION_NUM];

void action_sfactor();
void action_dfactor();

void kp_0();
void kp_B();

void action_zoom_in();
void action_zoom_out();
void action_tilt_in();
void action_tilt_out();
void action_spin_in();
void action_spin_out();
void action_twinkle();
void action_fog();
void action_switch_obj();



#endif // ACTIONS_H
