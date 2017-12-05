/*
 * progs_main.h
 *
 *  Created on: 27 нояб. 2017 г.
 *      Author: mastersan
 */

#ifndef SRC_PROGS_PROGS_MAIN_H_
#define SRC_PROGS_PROGS_MAIN_H_

#include "progs.h"

typedef enum
{
    WEAP_ARTILLERY,
    WEAP_MISSILE,
    WEAP_MINE,
    WEAP_NUM
} weapontype_t;

typedef struct
{
    //название оружия
    char * name;
    //изображение оружия
    image_index_t icon;



    char * entityname;
    sound_index_t sound_index;
} weaponinfo_t;

typedef struct
{
    ENTITY_VARS_COMMON_STRUCT;
} bull_vars_t;

typedef struct
{
    ENTITY_VARS_COMMON_STRUCT;
} explode_vars_t;

typedef struct
{
    ENTITY_VARS_COMMON_STRUCT;
    STRING text;
} message_vars_t;

typedef struct
{
    ENTITY_VARS_COMMON_STRUCT;
    INTEGER item_scores;
    INTEGER item_health;
    INTEGER item_armor ;
    INTEGER item_ammo_missile;
    INTEGER item_ammo_mine   ;
} spawn_vars_t;

typedef struct
{
    ENTITY_VARS_COMMON_STRUCT;
    STRING text;
} exit_vars_t;

typedef struct
{
    ENTITY_VARS_COMMON_STRUCT;
    INTEGER amount;
} item_vars_t;
extern var_descr_t item_vars_descr[];

typedef struct
{
    //скорость движения игрока
    vec_t speed;
    //игрок движется
    bool prev_go;
    bool go;
    //направление движения
} move_t;

//искуственный интеллект
typedef struct
{
    //флаг опастности
    bool danger;
    //флаг опастности
    bool Fdanger;
    bool attack;
    weapontype_t weap;
    //цель
    ENTITY target;
    //счетчик
    long count;
} player_ai_t;

typedef struct
{
    ENTITY_VARS_COMMON_STRUCT;

    INTEGER fragstotal; /* фрагов за пройденые карты */
    INTEGER frags;      /* фрагов за карту */
    INTEGER scores;     /* набрано очков */
    INTEGER level;      /* уровень игрока */

    INTEGER item_health;
    INTEGER item_armor;
    INTEGER item_ammo_artillery;
    INTEGER item_ammo_missile;
    INTEGER item_ammo_mine;

    //для управляемой ракеты
    ENTITY bull;

    //передвижения
    move_t move;
    bool attack;
    weapontype_t weap;
    //время на перезарядку
    long reloadtime_d;

    // мозг
    player_ai_t brain;

} player_vars_t;

ENTITY spawn_entity_by_class(const char * classname, const char * info, ENTITY parent);

game_exports_t * progs_init(void);

#endif /* SRC_PROGS_PROGS_MAIN_H_ */
