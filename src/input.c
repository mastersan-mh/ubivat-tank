#include "input.h"
#include <string.h>

typedef struct
{
	/* хэш клавиш клавиатуры */
	game_keyHash_t key[KEYS_HASH_NUM];

}game_input_t;
game_input_t game_input;

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
 * найти коллизию key
 * return:
 * = -1 - не нашли
 * = 0...n - индекс
 */
static int __findHashCollision(game_keyHash_t *__keyHash, int __key)
{
	for(size_t i = 0; i < __keyHash->amount; i++)
	{
		if(__keyHash->collision[i].key == __key)
		{
			return i;
		}
	}
	return -1;
}

/*
 *
 */
void input_init()
{
	memset(&game_input,0,sizeof(game_input));
}

/*
 *
 */
void input_done()
{
	for(int i=0;i<KEYS_HASH_NUM;i++)
	{
		free(game_input.key[i].collision);
	}
}

/*
 *
 */
void input_key_setState(int __key, bool __state)
{
	int collisionIndex;
	actionf_t func;
	game_keyHash_t * __keyHash = &(game_input.key[__key%KEYS_HASH_NUM]);


	collisionIndex = __findHashCollision(__keyHash, __key);

	if(collisionIndex < 0)
	{
		collisionIndex = __extendHash(__keyHash);
		__keyHash->collision[collisionIndex].key = __key;
	}

	if(__state)
	{
		func = __keyHash->collision[collisionIndex].press;
	}
	else
	{
		func = __keyHash->collision[collisionIndex].release;
	}

	if(func)
		func();
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
	}

}
