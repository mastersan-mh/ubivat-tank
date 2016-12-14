/*
 * ent_player.h
 *
 *  Created on: 10 окт. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_ENT_PLAYER_H_
#define SRC_ENT_PLAYER_H_

#include "weap.h"
#include "map.h"
#include "types.h"
#include "mobjs.h"
#include "ent_items.h"
#include "think.h"

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

typedef struct
{
	int items[__ITEM_NUM];
	vec_t speed;
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
	bool go;
	//направление движения
} move_t;

/*
 * характеристика игрока
 */
typedef struct
{
	//фрагов за пройденые карты
	long fragstotal;
	//фрагов на карте
	long frags;
	//ирок на карте
	bool spawned;
} stat_t;

typedef struct
{
	int level;
	int items[__ITEM_NUM];

	//характеристика
	stat_t charact;
	//для управляемой ракеты
	struct mobj_s * bull;
	//передвижения
	move_t move;
	bool attack;
	weapontype_t weap;
	//время на перезарядку
	long reloadtime_d;

	//флаг
	item_img_t * Iflag;
	// мозг
	think_t brain;
	int soundId_move;
} player_t;

#define ENT_PLAYER(mobj) ((player_t *) (mobj)->data)

extern playerinfo_t playerinfo_table[__PLAYER_LEVEL_NUM];



void mobj_player_init();


void player_moveUp_ON();
void player_moveUp_OFF();
void player_moveDown_ON();
void player_moveDown_OFF();
void player_moveLeft_ON();
void player_moveLeft_OFF();
void player_moveRight_ON();
void player_moveRight_OFF();
void player_attack_weapon1_ON();
void player_attack_weapon1_OFF();
void player_attack_weapon2_ON();
void player_attack_weapon2_OFF();
void player_attack_weapon3_ON();
void player_attack_weapon3_OFF();

void player2_moveUp_ON();
void player2_moveUp_OFF();
void player2_moveDown_ON();
void player2_moveDown_OFF();
void player2_moveLeft_ON();
void player2_moveLeft_OFF();
void player2_moveRight_ON();
void player2_moveRight_OFF();
void player2_attack_weapon1_ON();
void player2_attack_weapon1_OFF();
void player2_attack_weapon2_ON();
void player2_attack_weapon2_OFF();
void player2_attack_weapon3_ON();
void player2_attack_weapon3_OFF();


mobj_t * player_spawn_get();
int player_respawn(mobj_t * player);

void player_checkcode();
void player_item_get(struct mobj_s * player);

void player_spawn_init(mobj_t * player, player_t * pl, const mobj_t * spawn);

void player_draw_status(camera_t * cam, struct mobj_s * player);

void player_getdamage(struct mobj_s * player, struct mobj_s * explode, bool self, float radius);

void player_class_init(mobj_t * player, player_t * pl);


#endif /* SRC_ENT_PLAYER_H_ */
