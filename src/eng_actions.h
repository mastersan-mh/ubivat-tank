/*
 * eng_actions.h
 *
 *  Created on: 25 сент. 2021 г.
 *      Author: mastersan
 */

#ifndef SRC_ENG_ACTIONS_H_
#define SRC_ENG_ACTIONS_H_

#include <stddef.h>

#define ENG_ACTION_NAME_SIZE 64

struct eng_game_action
{
    const char * name;
    void (*func)(void);
};

void eng_actions_init(void);
void eng_actions_done(void);

int eng_actions_register(
        const struct eng_game_action * game_actions,
        size_t game_actions_num
);

const struct eng_game_action * eng_action_get(
        const char * action_name
);

void eng_action_exec(const char * action_name);

#endif /* SRC_ENG_ACTIONS_H_ */
