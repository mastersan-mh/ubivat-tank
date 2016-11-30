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

//оружие не используется
#define PLAYER_WEAP_NOTACCESSIBLE (-1)
//ускорение игрока, м/с^2
#define PLAYER_ACCEL       (0.05f * MPS2_TO_PIXPS2)
//#define PLAYER_ACCEL       12
/* торможение: м/с^2 */
#define PLAYER_DECEL       (0.05f * MPS2_TO_PIXPS2)
//скорость проигрывания кадров: бег
#define PLAYER_FPS_RUN      20


typedef struct
{
	//кол-во боеприпасов(-1 - не используется)
	int ammo[3];
	//атака
	weapon_t attack;
	//время на перезарядку
	long reloadtime_d;
} TINVweap;

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
	//очки
	int scores;
	//номер
	int id;
	//статус игрока
	char status;
	//здоровье max
	int healthmax;
	//здоровье
	int health;
	//броня max
	int armormax;
	//броня
	int armor;
	//фрагов за пройденые карты
	long fragstotal;
	//фрагов на карте
	long frags;
	//максимальная скорость при ходьбе
	long speed;
	//ирок на карте
	bool spawned;
} Tcharacter;

typedef struct player_s
{
	struct player_s * next;
	//характеристика
	Tcharacter charact;
	//для управляемой ракеты
	mobj_t * bull;
	//передвижения
	Tmove move;
	//оружия
	TINVweap w;
	//флаг
	item_img_t * Iflag;
	//база
	item_img_t * Ibase;
	//№ кадра(база)
	float Fbase;
	// мозг
	int soundId_move;
	think_t brain;
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
