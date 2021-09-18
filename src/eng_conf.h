/**
 * @file eng_conf.h
 *
 * game configuration file
 *
 */

#ifndef SRC_ENG_CONF_H_
#define SRC_ENG_CONF_H_

#include <SDL2/SDL_scancode.h>

struct keybind
{
	int key;
	char * action_name;
};

int eng_conf_init(void);
void eng_conf_done(void);

const char * eng_scancode_name_get(SDL_Scancode scancode);

int eng_conf_key_bind(
        SDL_Scancode scancode,
        const char * action
);

const char * eng_conf_key_action_get(
        SDL_Scancode scancode
);

const SDL_Scancode * eng_conf_action_key_get(const char * action_name);

void eng_conf_key_unbind(
        SDL_Scancode scancode
);

void eng_conf_key_unbindall(void);

void eng_conf_bindall_default(void);

int eng_conf_save(void);
int eng_conf_load(void);

#endif /* SRC_ENG_CONF_H_ */
