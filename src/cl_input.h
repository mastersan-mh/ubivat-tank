#ifndef SRC_CL_INPUT_H_
#define SRC_CL_INPUT_H_

#include "types.h"
#include "actions.h"

#include <stdlib.h>

/* количество клавиш */
#define KEYS_HASH_NUM 256


typedef struct
{
	/* код клавиши */
	int key;
	/* функции обработки нажатий/отпусканий */
	actionf_t press;
	actionf_t release;
	char * action_press;
	char * action_release;
}
game_key_t;

typedef struct
{
	/* количество ключей в коллизии */
	size_t amount;
	/* коллизии клавиш */
	game_key_t *collision;
}
game_keyHash_t;

void input_init();
void input_done();
void input_key_setState(int __key, bool __state);
void input_key_bind_act(int __key, const char * action);
void input_key_bind(int __key, actionf_t __press, actionf_t __release);
void input_key_bindAction(int __key, action_t __action);
void input_key_unbind(int __key);
void input_key_unbind_all();


#endif /* SRC_CL_INPUT_H_ */
