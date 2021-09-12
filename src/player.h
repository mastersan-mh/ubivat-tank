/**
 * @file player.h
 *
 *  Created on: 10 окт. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_PLAYER_H_
#define SRC_PLAYER_H_

#include "defs.h"
#include <weap.h>
#include <map.h>
#include <types.h>

#define SPEEDSCALE ((1.0f/27.0f) / (GAME_TICK_PERIOD))

#define TANK_LEVEL0_SPEED (40.0 * SPEEDSCALE)
#define TANK_LEVEL1_SPEED (50.0 * SPEEDSCALE)
#define TANK_LEVEL2_SPEED (60.0 * SPEEDSCALE)
#define TANK_LEVEL3_SPEED (70.0 * SPEEDSCALE)
#define TANK_LEVEL4_SPEED (90.0 * SPEEDSCALE)


//оружие не используется
#define PLAYER_WEAP_NOTACCESSIBLE (-1)
/* ускорение игрока, м/с^2 */
#define PLAYER_ACCEL       (7.0 / GAME_TICK_PERIOD)
/* торможение: м/с^2 */
#define PLAYER_DECEL       (7.0 / GAME_TICK_PERIOD)
/* скорость проигрывания кадров */
#define PLAYER_FPS_RUN      (2.0 / GAME_TICK_PERIOD)

typedef enum
{
	DIR_UP,
	DIR_DOWN,
	DIR_LEFT,
	DIR_RIGHT
} direction_t;

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
	coord_t speed;
	//ирок на карте
	bool spawned;
} Tcharacter;

typedef struct player_s
{
	struct player_s * next;
	//характеристика
	Tcharacter charact;
	//для управляемой ракеты
	struct bull_s * bull;
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

void player_getdamage(player_t * player, explode_t * explode, bool self, float radius);

void players_control();


#endif /* SRC_PLAYER_H_ */
