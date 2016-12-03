/*
 * plr.h
 *
 *  Created on: 10 окт. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_PLAYER_H_
#define SRC_PLAYER_H_

#include <weap.h>
#include <map.h>
#include <types.h>

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
//скорость проигрывания кадров: бег
#define PLAYER_FPS_RUN      20


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
	coord_t speed;
	image_index_t imageindex;
} playerinfo_t;

/*
 * перемещения игрока
 */
typedef struct
{
	//координаты
	pos_t pos;
	//скорость движения игрока
	coord_t speed;
	//игрок движется
	bool go;
	//направление движения
	//0 - вверх;1 - вниз;2 - влево;3 - вправо
	direction_t dir;
} Tmove;

/*
 * характеристика игрока
 */
typedef struct
{
	//номер
	int id;
	//статус игрока
	char status;
	//фрагов за пройденые карты
	long fragstotal;
	//фрагов на карте
	long frags;
	//ирок на карте
	bool spawned;
} Tcharacter;

typedef struct player_s
{
	struct player_s * next;
	int level;
	int items[__ITEM_NUM];

	//характеристика
	Tcharacter charact;
	//для управляемой ракеты
	mobj_t * bull;
	//передвижения
	Tmove move;
	bool attack;
	weapontype_t weap;
	//время на перезарядку
	long reloadtime_d;

	//флаг
	item_img_t * Iflag;
	//база
	item_img_t * Ibase;
	//№ кадра(база)
	float Fbase;
	// мозг
	think_t brain;
	int soundId_move;
} player_t;

extern player_t * playerList;

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


void player_checkcode();
void player_item_get(player_t * player);
void player_obj_check(player_t * player);
void player_draw_all(camera_t * cam);

player_t * player_find(int status);
int  player_connect(int status);
void player_disconnect_monsters();
void player_disconnect_all();
void player_spawn(player_t * player, mobj_t * spawn);
int player_spawn_player(player_t * player);
int player_spawn_enemy();
void player_spawn_all();

void player_class_init(player_t * player);

void player_draw_status(camera_t * cam, player_t * player);

void player_getdamage(player_t * player, mobj_t * explode, bool self, float radius);

void players_control();


#endif /* SRC_PLAYER_H_ */
