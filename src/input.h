#ifndef INPUT_H
#define INPUT_H

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

void input_init(void);
void input_done(void);
void input_key_setState(int __key, bool __state);
void input_key_bind(int __key, actionf_t __press, actionf_t __release);
void input_key_unbind(int __key);
void input_key_unbind_all(void);


#endif // INPUT_H
