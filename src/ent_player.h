/*
 * ent_player.h
 *
 *  Created on: 10 окт. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_ENT_PLAYER_H_
#define SRC_ENT_PLAYER_H_

#include "map.h"
#include "types.h"
#include "entity.h"
#include "ent_items.h"
#include "ent_player_think.h"
#include "ent_weap.h"
#include "ent_explode.h"

/*
	pix/s -> m/s -> km/h
	16p = 6m
	10 m/s = 10*8 p/s
	90 km/h = 90 * 1000/3600 * 8 p/s

	km/h * SPEEDSCALE = pix/s
*/
/* пикселов на метр */
#define PIXPM (16.0f/6.0f)
/* метров на пиксел */
#define MPPIX (1.0f/PIXPM)

#define SPEEDSCALE (( PIXPM * 1000.0f)/(3600.0f))
/* множитель скорости: пикселов/с */
#define SPEEDSCALE_PIXPS SPEEDSCALE
/* множитель скорости: пикселов/мс */
#define SPEEDSCALE_PIXPMS (PIXPM /(1000.f * 1000.f))

#define MPS2_TO_KMPH2 ( (3600.0f * 3600.0f) / 1000.f)
/* pix/(s^2) */
#define MPS2_TO_PIXPS2 ( 1.0f / MPPIX)
/* pix/(ms^2) */
#define MPS2_TO_PIXPMS2 ( (0.001f * 0.001f) / MPPIX)

//ускорение игрока, м/с^2
#define PLAYER_ACCEL       (0.05f * MPS2_TO_PIXPS2)
//#define PLAYER_ACCEL       12
/* торможение: м/с^2 */
#define PLAYER_DECEL       (0.05f * MPS2_TO_PIXPS2)

/* предмет не используется */
#define PLAYER_ITEM_AMOUNT_NA  (0)
/* бесконечно */
#define PLAYER_ITEM_AMOUNT_INF (-1)

//очки
//очки за 1 убитый танк
#define PLAYER_SCORES_PER_ENEMY 5
//сколько очков нужно набрать для повышения уровня танка
#define PLAYER_SCOREPERCLASS 200
//макс. кол-во очков
#define PLAYER_SCORES_MAX     2000000

enum
{
	PLAYER_LEVEL1,
	PLAYER_LEVEL2,
	PLAYER_LEVEL3,
	PLAYER_LEVEL4,
	PLAYER_LEVEL5,
	PLAYER_LEVEL_BOSS,
	__PLAYER_LEVEL_NUM
};

/* инвентарь */
typedef enum
{
    ITEM_SCORES,
    ITEM_HEALTH,
    ITEM_ARMOR,
    ITEM_AMMO_ARTILLERY,
    ITEM_AMMO_MISSILE,
    ITEM_AMMO_MINE,
    ITEM_NUM
} player_invitemtype_t;

typedef struct
{
	INTEGER items[ITEM_NUM];
	VECTOR1 speed;
	char * modelname;
} playerinfo_t;

/*
 * перемещения игрока
 */
typedef struct
{
	//скорость движения игрока
	vec_t speed;
	//игрок движется
	bool prev_go;
	bool go;
	//направление движения
} move_t;

typedef struct
{
    ENTITY_COMMON_STRUCT;

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
    struct entity_s * bull;

    //передвижения
    move_t move;
    bool attack;
    weapontype_t weap;
    //время на перезарядку
    long reloadtime_d;

    // мозг
    think_t brain;

} player_vars_t;

extern playerinfo_t playerinfo_table[__PLAYER_LEVEL_NUM];

extern void player_spawn_init(void * player , void * spawn);


extern void entity_player_init(void);

extern void player_getdamage(void * player, void * explode, bool self, vec_t distance, const explodeinfo_t * explodeinfo);

extern void player_class_init(void * player, player_vars_t * pl);


#endif /* SRC_ENT_PLAYER_H_ */
