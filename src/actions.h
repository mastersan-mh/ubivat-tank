#ifndef ACTIONS_H
#define ACTIONS_H

#include "types.h"

#define GAME_ACTION_SIZE 64

/* сравнение действий: true / false */
#define ACTIONS_EQ(act1, act2) \
        ( act1 && act2 && !strncmp(act1, act2, GAME_ACTION_SIZE) )

typedef struct
{
    char * action;
    void (*actionf_press)(const char * action);
    void (*actionf_release)(const char * action);
} game_action_t;

extern const game_action_t * game_action_find(const char * action_str);

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
