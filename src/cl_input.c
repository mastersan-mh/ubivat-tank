
#include "game.h"
#include "sound.h"

#include "cl_input.h"
#include "client.h"
#include "client_private.h"

#include <stdlib.h>
#include <string.h>

/* количество клавиш */
#define KEYS_HASH_NUM 256

typedef struct
{
    /* количество ключей в коллизии */
    size_t amount;
    /* коллизии клавиш */
    client_keybind_t *collision;
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
    oldsize = keyhash->amount * sizeof(client_keybind_t);
    newIndex = keyhash->amount;
    keyhash->amount++;
    size = keyhash->amount * sizeof(client_keybind_t);
    keyhash->collision = xrealloc(keyhash->collision, size, oldsize);

    return newIndex;
}

/**
 * @brief найти коллизию key
 * @return
 *   = -1 - не нашли
 *   = 0...n - индекс
 */
static ssize_t hash_find_collision(keyhash_t * keyhash, int key)
{
    for(ssize_t i = 0; i <  keyhash->amount; i++)
    {
        if( keyhash->collision[i].key == key)
            return i;
    }
    return -1;
}

static const client_keybind_t * key_to_inputkey(int key)
{
    ssize_t cindex;
    keyhash_t * keyhash = &(keyhashs[key % KEYS_HASH_NUM]);
    cindex = hash_find_collision(keyhash, key);

    if(cindex < 0)
    {
        cindex = hash_extend(keyhash);
        keyhash->collision[cindex].key = key;
    }
    return &keyhash->collision[cindex];
}


/*
 *
 */
void client_input_init()
{
    memset(&keyhashs, 0, sizeof(keyhashs));
}

/*
 *
 */
void client_input_done()
{
    client_key_unbind_all();
}



/**
 * установка действия.
 * Для действия '+', на отжатие клавиши автоматически устанавливается действие '-'
 * Если действие уже назначено, оно переписывается.
 */
int client_key_bind(int playerId, int key, const char * action)
{
    keyhash_t * keyhash = &keyhashs[key % KEYS_HASH_NUM];

    ssize_t cindex = hash_find_collision(keyhash, key);

    if(cindex < 0)
        cindex = hash_extend(keyhash);

    client_keybind_t * keybind = &keyhash->collision[cindex];
    keybind->playerId = playerId;
    keybind->key = key;
    if(keybind->action_press)
        free(keybind->action_press);
    if(keybind->action_release)
        free(keybind->action_release);
    keybind->action_press = strdup(action);
    keybind->action_release = NULL;
    if(action[0] == '+')
    {
        keybind->action_release = strdup(action);
        keybind->action_release[0] = '-';
    }
    return 0;
}

int client_key_binded_get(int playerId, const char * action)
{
    size_t i;
    for(i = 0; i < KEYS_HASH_NUM; i++)
    {
        keyhash_t * keyhash = &keyhashs[i];
        for(size_t cindex = 0; cindex < keyhash->amount ; cindex++)
        {
            client_keybind_t * keybind = &keyhash->collision[cindex];
            if(
                    keybind->playerId == playerId &&
                    (
                            ACTIONS_EQ(keybind->action_press, action) ||
                            ACTIONS_EQ(keybind->action_release, action)
                    )
            )
                return keybind->key;
        }
    }
    return -1;
}

/*
 * отвязать функцию от клавиши
 */
void client_key_unbind(int key)
{
    keyhash_t *keyhash = &keyhashs[key%KEYS_HASH_NUM];

    ssize_t cindex = hash_find_collision(keyhash, key);

    if(cindex < 0)
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
        client_keybind_t * tmp = malloc(newamount * sizeof(client_keybind_t));
        free(keyhash->collision[cindex].action_press);
        free(keyhash->collision[cindex].action_release);

        size_t i;
        for( i = 0; i < cindex; i++)
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

/**
 * @brief отвязать функцию от клавиши
 */
void client_key_unbind_all(void)
{
    for(size_t i = 0; i < KEYS_HASH_NUM; i++)
    {
        keyhash_t * keyhash = &keyhashs[i % KEYS_HASH_NUM];
        for(size_t cindex = 0; cindex < keyhash->amount; cindex++)
        {
            free(keyhash->collision[cindex].action_press);
            free(keyhash->collision[cindex].action_release);
        }
        free(keyhash->collision);
        keyhash->collision = NULL;
        keyhash->amount = 0;
    }
}

void client_key_unbind_action(int playerId, const char * action)
{
    for(size_t i = 0; i < KEYS_HASH_NUM; i++)
    {
        keyhash_t * keyhash = &keyhashs[i];
        size_t newamount = 0;
        for(size_t cindex = 0; cindex < keyhash->amount ; cindex++)
        {
            client_keybind_t * keybind = &keyhash->collision[cindex];
            if(
                    !(
                            keybind->playerId == playerId &&
                            (
                                    ACTIONS_EQ(keybind->action_press, action) ||
                                    ACTIONS_EQ(keybind->action_release, action)
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
                size_t newsize = newamount * sizeof(client_keybind_t);
                client_keybind_t * newcollision = malloc(newsize);
                size_t ci = 0;
                for(size_t cindex = 0; cindex < keyhash->amount ; cindex++)
                {
                    client_keybind_t * keybind = &keyhash->collision[cindex];
                    if(
                            (
                                    keybind->playerId == playerId &&
                                    (
                                            ACTIONS_EQ(keybind->action_press, action) ||
                                            ACTIONS_EQ(keybind->action_release, action)
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

void client_key_press(int key)
{
    const client_keybind_t * inputkey = key_to_inputkey(key);
    const char * action_str = inputkey->action_press;
    const game_action_t * game_action = game_action_find(action_str);
    /* Если клавиша перекрыта локальным действием, выполняем его без отправки серверу */
    if(game_action)
    {
        game_action->actionf_press(action_str);
        return;
    }
    if(action_str)
        client_req_send_player_action(inputkey->playerId, action_str);
}


void client_key_release(int key)
{
    const client_keybind_t * inputkey = key_to_inputkey(key);
    const char * action_str = inputkey->action_release;
    const game_action_t * game_action = game_action_find(action_str);
    if(game_action)
    {
        game_action->actionf_release(action_str);
        return;
    }
    if(action_str)
        client_req_send_player_action(inputkey->playerId, action_str);
}

/**
 * @brief Проход по всем привязаным клавишам
 */
int client_foreachkey(int (*callback)(const client_keybind_t * keybind, void * userdata), void * userdata )
{
    for(size_t i = 0; i < KEYS_HASH_NUM; i++)
    {
        keyhash_t * keyhash = &keyhashs[i];
        for(size_t cindex = 0; cindex < keyhash->amount ; cindex++)
        {
            client_keybind_t * keybind = &keyhash->collision[cindex];
            int ret = (*callback)(keybind, userdata);
            if(ret)
                return ret;
        }
    }
    return 0;
}

