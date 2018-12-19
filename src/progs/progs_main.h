/*
 * progs_main.h
 *
 *  Created on: 27 нояб. 2017 г.
 *      Author: mastersan
 */

#ifndef SRC_PROGS_PROGS_MAIN_H_
#define SRC_PROGS_PROGS_MAIN_H_

#include "progs.h"

#include "../server_private.h"

extern const game_imports_t * gi;

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
    int /* sound_index_t */ sound_index;
} weaponinfo_t;

typedef struct
{
    //скорость движения игрока
    vec_t speed;
    //игрок движется
    bool prev_go;
    bool go;
    //направление движения
} move_t;

typedef struct entity_s entity_t;

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
    entity_t * target;
    //счетчик
    long count;
} player_ai_t;

struct entity_s
{
    entity_common_t c;

    BOOL used;

    entity_t * owner;

    const STRING classname;

    BOOL spawned;

    /* message, exit */
    STRING text;

    /* player, spawner */
    INTEGER item_scores;
    INTEGER item_health;
    INTEGER item_armor;
    INTEGER item_ammo_artillery;
    INTEGER item_ammo_missile;
    INTEGER item_ammo_mine;

    /* item */
    INTEGER amount;

    /* player */
    INTEGER fragstotal; /* фрагов за пройденые карты */
    INTEGER frags;      /* фрагов за карту */
    INTEGER scores;     /* набрано очков */
    INTEGER level;      /* уровень игрока */


    //для управляемой ракеты
    entity_t * bull;

    //передвижения
    move_t move;
    bool attack;
    weapontype_t weap;
    //время на перезарядку
    long reloadtime_d;

    void (*think)(entity_t * self);
    void (*touch)(entity_t * self, entity_t * other);

    // мозг
    player_ai_t brain;

    /* статистика */
    /* пройденое расстояние */
    FLOAT stat_traveled_distance;

};

extern size_t g_entities_num;
extern entity_t * g_entities;

entity_t * spawn_entity_by_class(const char * classname, const char * info, entity_t * parent);

game_exports_t * progs_init(const game_imports_t * gi);

#endif /* SRC_PROGS_PROGS_MAIN_H_ */
