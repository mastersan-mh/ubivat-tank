#include "cl_input.h"
#include "client.h"
#include "game.h"
#include "sound.h"


#include <string.h>

typedef struct
{
	/* хэш клавиш клавиатуры */
	game_keyHash_t key[KEYS_HASH_NUM];

}game_input_t;
static game_input_t game_input;

static void * __xrealloc(void * __ptrmem, size_t __newsize, size_t __oldsize)
{
	if(__newsize==__oldsize)
	{
		return __ptrmem;
	}

	void *__new;
	__new = malloc(__newsize);

	if(__newsize > __oldsize)
	{
		memcpy(__new, __ptrmem, __oldsize);
		memset(__new+__oldsize, 0, __newsize-__oldsize);
	}
	else
	{
		memcpy(__new, __ptrmem, __newsize);
	}
	if(__ptrmem)
		free(__ptrmem);
	return __new;
}

/*
 * вставка новой коллизии
 */
static int __extendHash(game_keyHash_t * keyHash)
{
	int size;
	int oldsize;
	/* индекс нового элемента коллизии */
	int newIndex;

	/* расширим массив коллизий */
	oldsize = keyHash->amount * sizeof(game_key_t);
	newIndex = keyHash->amount;
	keyHash->amount++;
	size = keyHash->amount*sizeof(game_key_t);
	keyHash->collision = __xrealloc(keyHash->collision, size, oldsize);

	return newIndex;
}

/*
 * @description найти коллизию key
 * @return
 *   = -1 - не нашли
 *   = 0...n - индекс
 */
static int __findHashCollision(game_keyHash_t *__keyHash, int __key)
{
	for(size_t i = 0; i < __keyHash->amount; i++)
	{
		if(__keyHash->collision[i].key == __key)
			return i;
	}
	return -1;
}

/*
 *
 */
void input_init()
{
	memset(&game_input, 0, sizeof(game_input));
}

/*
 *
 */
void input_done()
{
	input_key_unbind_all();
}

/*
 *
 */
void input_key_setState(int __key, bool __state)
{
	int collisionIndex;
	actionf_t func = NULL;
	char * action;
	game_keyHash_t * __keyHash = &(game_input.key[__key%KEYS_HASH_NUM]);


	collisionIndex = __findHashCollision(__keyHash, __key);

	if(collisionIndex < 0)
	{
		collisionIndex = __extendHash(__keyHash);
		__keyHash->collision[collisionIndex].key = __key;
	}


	int clientId = 0;

	if(__state)
	{
		if(game.state == GAMESTATE_INGAME)
		{
			action = __keyHash->collision[collisionIndex].action_press;
			func = __keyHash->collision[collisionIndex].press;
		}
		else if(game.state == GAMESTATE_MISSION_BRIEF)
		{
			sound_play_start(SOUND_MENU_ENTER, 1);
			game.state = GAMESTATE_INGAME;
		}
		else if(game.state == GAMESTATE_INTERMISSION)
		{
			sound_play_start(SOUND_MENU_ENTER, 1);
			// *imenu = MENU_GAME_SAVE;
			game.show_menu = true;
		}
	}
	else
	{

		if(game.state == GAMESTATE_INGAME)
		{
			action = __keyHash->collision[collisionIndex].action_release;
			func = __keyHash->collision[collisionIndex].release;
		}
	}

	if(func)
		func();

	if(action)
		client_event_control_send(clientId, action);


}

void input_key_bind_act(int __key, const char * action)
{
	game_keyHash_t * __keyHash = &(game_input.key[__key % KEYS_HASH_NUM]);

	int collisionIndex = __findHashCollision(__keyHash, __key);

	if(collisionIndex < 0)
	{
		collisionIndex = __extendHash(__keyHash);
	}
	__keyHash->collision[collisionIndex].key = __key;
	__keyHash->collision[collisionIndex].action_press = strdup(action);
	if(action[0] == '+')
	{
		__keyHash->collision[collisionIndex].action_release = strdup(action);
		__keyHash->collision[collisionIndex].action_release[0] = '-';
	}
}

/*
 * привязать функцию к клавише
 */
void input_key_bind(int __key, actionf_t __press, actionf_t __release)
{
	game_keyHash_t * __keyHash = &(game_input.key[__key % KEYS_HASH_NUM]);

	int collisionIndex = __findHashCollision(__keyHash, __key);

	if(collisionIndex < 0)
	{
		collisionIndex = __extendHash(__keyHash);
	}
	__keyHash->collision[collisionIndex].key = __key;
	__keyHash->collision[collisionIndex].press = __press;
	__keyHash->collision[collisionIndex].release = __release;

}

void input_key_bindAction(int __key, action_t __action)
{
	input_key_bind(__key, __action.press, __action.release);
}

/*
 * отвязать функцию от клавиши
 */
void input_key_unbind(int __key)
{
	int collisionIndex;
	game_keyHash_t *__keyHash = &(game_input.key[__key%KEYS_HASH_NUM]);

	collisionIndex=__findHashCollision(__keyHash, __key);

	if(collisionIndex >= 0)
	{
		__keyHash->collision[collisionIndex].press = NULL;
		__keyHash->collision[collisionIndex].release = NULL;
		if(__keyHash->collision[collisionIndex].action_press)
			free(__keyHash->collision[collisionIndex].action_press);
		if(__keyHash->collision[collisionIndex].action_release)
			free(__keyHash->collision[collisionIndex].action_release);
	}

}

/*
 * отвязать функцию от клавиши
 */
void input_key_unbind_all()
{
	for(int i=0; i < KEYS_HASH_NUM; i++)
	{
		game_input.key[i].amount = 0;
		for(int j; j < game_input.key[i].amount ; j++)
		{
			if(game_input.key[i].collision[j].action_press)
				free(game_input.key[i].collision[j].action_press);
			if(game_input.key[i].collision[j].action_release)
				free(game_input.key[i].collision[j].action_release);
		}
		free(game_input.key[i].collision);
		game_input.key[i].collision = NULL;
	}
}
