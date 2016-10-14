/*
 * plr.h
 *
 *  Created on: 10 окт. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_PLR_H_
#define SRC_PLR_H_

#include <defs.h>
#include <weap.h>
#include <map.h>

typedef struct
{
	//кол-во боеприпасов(-1 - не используется)
	long ammo[3];
	//атака
	char attack;
	//время на перезарядку
	long reloadtime_d;
} TINVweap;

/*
 * перемещения игрока
 */
typedef struct
{
	//координаты
	pos_t orig;
	//скорость движения игрока
	long speed;
	//игрок движется
	bool go;
	//направление движения
	//0 - вверх;1 - вниз;2 - влево;3 - вправо
	int dir;
} Tmove;

/*
 * характеристика игрока
 */
typedef struct
{
	//очки
	long scores;
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
	TAIbrain brain;
	//время
	struct time_s time;
} player_t;

extern player_t * playerList;

void player_checkcode();
void player_item_get(player_t * player);
void player_obj_check(player_t * player);
void player_draw(camera_t * cam, player_t * player, bool play);
void player_control(player_t * player);
player_t * player_find(int status);
int  player_connect(int status);
void player_disconnect_monsters();
void player_disconnect_all();
void player_spawn(player_t * player, TDATAspawn * DATA);
int player_spawn_player(player_t * player);
int player_spawn_enemy();
void player_spawn_all();

void player_class_init(player_t * player);

void player_draw_status(camera_t * cam, player_t * player);

void player_getdamage(player_t * player, explode_t * explode, bool self, float radius);


#endif /* SRC_PLR_H_ */
