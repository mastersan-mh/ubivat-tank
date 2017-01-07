#include "cl_input.h"
#include "client.h"
#include "game.h"
#include "sound.h"

#include <stdlib.h>
#include <string.h>

/* количество клавиш */
#define KEYS_HASH_NUM 256

#define CL_INPUT_ACTION_LEN (63)

/* сравнение действий: true / false */
#define ACTIONCMP(act1, act2) \
	( act1 && act2 && !strncmp(act1, act2, CL_INPUT_ACTION_LEN) )

typedef struct
{
	/* количество ключей в коллизии */
	size_t amount;
	/* коллизии клавиш */
	input_key_t *collision;
} keyhash_t;

/* хэш клавиш клавиатуры */
static keyhash_t keyhashs[KEYS_HASH_NUM] = {};

static void * xrealloc(void * ptrmem, size_t newsize, size_t oldsize)
{
	if(newsize == oldsize)
	{
		return ptrmem;
	}

	void * newmem;
	newmem = malloc(newsize);

	if(newsize > oldsize)
	{
		memcpy(newmem, ptrmem, oldsize);
		memset(newmem + oldsize, 0, newsize - oldsize);
	}
	else
	{
		memcpy(newmem, ptrmem, newsize);
	}
	if(ptrmem)
		free(ptrmem);
	return newmem;
}

/*
 * вставка новой коллизии
 */
static size_t hash_extend(keyhash_t * keyhash)
{
	size_t size;
	size_t oldsize;
	/* индекс нового элемента коллизии */
	size_t newIndex;

	/* расширим массив коллизий */
	if(keyhash->amount == 0)
		keyhash->collision = NULL;
	oldsize = keyhash->amount * sizeof(input_key_t);
	newIndex = keyhash->amount;
	keyhash->amount++;
	size = keyhash->amount * sizeof(input_key_t);
	keyhash->collision = xrealloc(keyhash->collision, size, oldsize);

	return newIndex;
}

/*
 * @description найти коллизию key
 * @return
 *   = -1 - не нашли
 *   = 0...n - индекс
 */
static int hash_find_collision(keyhash_t * keyhash, int key)
{
	for(size_t i = 0; i <  keyhash->amount; i++)
	{
		if( keyhash->collision[i].key == key)
			return i;
	}
	return -1;
}

/*
 *
 */
void input_init()
{
	memset(&keyhashs, 0, sizeof(keyhashs));
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
void input_key_setState(int key, bool state)
{
	int collisionIndex;
	actionf_t func = NULL;
	char * action;
	keyhash_t * keyhash = &(keyhashs[key % KEYS_HASH_NUM]);


	collisionIndex = hash_find_collision(keyhash, key);

	if(collisionIndex < 0)
	{
		collisionIndex = hash_extend(keyhash);
		keyhash->collision[collisionIndex].key = key;
	}


	int clientId = 0;

	input_actionsrc_t actionsrc = keyhash->collision[collisionIndex].actionsrc;

	if(state)
	{
		if(game.state == GAMESTATE_INGAME)
		{
			switch(actionsrc)
			{
			case INPUT_ACTIONSRC_STR:  action = keyhash->collision[collisionIndex].action.str.press; break;
			case INPUT_ACTIONSRC_FUNC: func = keyhash->collision[collisionIndex].action.func.press; break;
			}
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
			switch(actionsrc)
			{
			case INPUT_ACTIONSRC_STR : action = keyhash->collision[collisionIndex].action.str.release; break;
			case INPUT_ACTIONSRC_FUNC: func   = keyhash->collision[collisionIndex].action.func.release; break;
			}
		}
	}

	if(func)
		func();

	if(action)
		client_event_control_send(clientId, action);


}

int input_key_get(int clientId, const char * action)
{
	size_t i;
	for(i = 0; i < KEYS_HASH_NUM; i++)
	{
		keyhash_t * keyhash = &keyhashs[i];
		for(size_t cindex = 0; cindex < keyhash->amount ; cindex++)
		{
			input_key_t * keybind = &keyhash->collision[cindex];
			if(
					keybind->clientId == clientId &&
					keybind->actionsrc == INPUT_ACTIONSRC_STR &&
					(
							ACTIONCMP(keybind->action.str.press, action) ||
							ACTIONCMP(keybind->action.str.release, action)
					)
			)
				return keybind->key;
		}
	}
	return -1;
}

int input_foreverykey(int (*callback)(const input_key_t * keybind, void * userdata), void * userdata )
{
	for(size_t i = 0; i < KEYS_HASH_NUM; i++)
	{
		keyhash_t * keyhash = &keyhashs[i];
		for(size_t cindex = 0; cindex < keyhash->amount ; cindex++)
		{
			input_key_t * keybind = &keyhash->collision[cindex];
			int ret = (*callback)(keybind, userdata);
			if(ret)
				return ret;
		}
	}
	return 0;
}

/*
 * установка действия. Для действия '+', на отжатие клавиши автоматически устанавливается действие '-'
 */
int input_key_bind_act(int clientId, int key, const char * action)
{
	keyhash_t * keyhash = &keyhashs[key % KEYS_HASH_NUM];

	int collisionIndex = hash_find_collision(keyhash, key);

	if(collisionIndex >= 0)
		return -1;
	collisionIndex = hash_extend(keyhash);
	input_key_t * keybind = &keyhash->collision[collisionIndex];
	keybind->clientId = clientId;
	keybind->key = key;
	keybind->actionsrc = INPUT_ACTIONSRC_STR;
	keybind->action.str.press = strdup(action);
	keybind->action.str.release = NULL;
	if(action[0] == '+')
	{
		keybind->action.str.release = strdup(action);
		keybind->action.str.release[0] = '-';
	}
	return 0;
}

/*
 * привязать функцию к клавише
 */
int input_key_bind(int key, actionf_t press, actionf_t release)
{
	keyhash_t * keyhash = &(keyhashs[key % KEYS_HASH_NUM]);

	int collisionIndex = hash_find_collision(keyhash, key);

	if(collisionIndex >= 0)
		return -1;
	collisionIndex = hash_extend(keyhash);
	input_key_t * keybind = &keyhash->collision[collisionIndex];
	keybind->actionsrc = INPUT_ACTIONSRC_FUNC;
	keybind->key = key;
	keybind->action.func.press = press;
	keybind->action.func.release = release;
	return 0;
}

int input_key_bindAction(int key, action_t action)
{
	return input_key_bind(key, action.press, action.release);
}

/*
 * отвязать функцию от клавиши
 */
void input_key_unbind(int clientId, int key)
{
	int collisionIndex;
	keyhash_t *keyhash = &keyhashs[key%KEYS_HASH_NUM];

	collisionIndex = hash_find_collision(keyhash, key);

	if(collisionIndex < 0)
		return;

	if(clientId != CLIENT_ALL && keyhash->collision[collisionIndex].clientId != clientId)
		return;

	size_t newamount = keyhash->amount - 1;
	if(newamount == 0)
	{
		keyhash->amount = 0;
		free(keyhash->collision);
		keyhash->collision = NULL;
	}
	else
	{
		input_key_t * tmp = malloc(newamount * sizeof(input_key_t));
		switch(keyhash->collision[collisionIndex].actionsrc)
		{
		case INPUT_ACTIONSRC_STR:
			free(keyhash->collision[collisionIndex].action.str.press);
			free(keyhash->collision[collisionIndex].action.str.release);
			break;
		case INPUT_ACTIONSRC_FUNC:
			break;
		}

		size_t i;
		for( i = 0; i < collisionIndex; i++)
		{
			tmp[i] = keyhash->collision[i];
		}

		for( ; i < newamount; i++)
		{
			tmp[i] = keyhash->collision[i + 1];
		}
		keyhash->amount = newamount;
		free(keyhash->collision);
		keyhash->collision = tmp;

	}
}

/*
 * отвязать функцию от клавиши
 */
void input_key_unbind_all()
{
	for(size_t i = 0; i < KEYS_HASH_NUM; i++)
	{
		keyhash_t * keyhash = &keyhashs[i % KEYS_HASH_NUM];
		for(size_t cindex = 0; cindex < keyhash->amount; cindex++)
		{

			switch(keyhash->collision[cindex].actionsrc)
			{
			case INPUT_ACTIONSRC_STR:
				free(keyhash->collision[cindex].action.str.press);
				free(keyhash->collision[cindex].action.str.release);
				break;
			case INPUT_ACTIONSRC_FUNC:
				break;
			}
		}
		free(keyhash->collision);
		keyhash->collision = NULL;
		keyhash->amount = 0;
	}
}

void input_action_unbind(int clientId, const char * action)
{
	for(size_t i = 0; i < KEYS_HASH_NUM; i++)
	{
		keyhash_t * keyhash = &keyhashs[i];
		size_t newamount = 0;
		for(size_t cindex = 0; cindex < keyhash->amount ; cindex++)
		{
			input_key_t * keybind = &keyhash->collision[cindex];
			if(
					!(
							keybind->clientId == clientId &&
							keybind->actionsrc == INPUT_ACTIONSRC_STR &&
							(
									ACTIONCMP(keybind->action.str.press, action) ||
									ACTIONCMP(keybind->action.str.release, action)
							)
					)
			)
				newamount++;
		}

		if(newamount < keyhash->amount)
		{
			if(newamount == 0)
			{
				keyhash->amount = newamount;
				free(keyhash->collision);
				keyhash->collision = NULL;
			}
			else
			{
				size_t newsize = newamount * sizeof(input_key_t);
				input_key_t * newcollision = malloc(newsize);
				size_t ci = 0;
				for(size_t cindex = 0; cindex < keyhash->amount ; cindex++)
				{
					input_key_t * keybind = &keyhash->collision[cindex];
					if(
							(
									keybind->clientId == clientId &&
									keybind->actionsrc == INPUT_ACTIONSRC_STR &&
									(
											ACTIONCMP(keybind->action.str.press, action) ||
											ACTIONCMP(keybind->action.str.release, action)
									)
							)
					)
					{
						newcollision[ci++] = *keybind;
					}
				}
				keyhash->amount = newamount;
				free(keyhash->collision);
				keyhash->collision = newcollision;
			}

		}
	}
}