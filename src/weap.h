/*
 * weap.h
 *
 *  Created on: 10 окт. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_WEAP_H_
#define SRC_WEAP_H_

#include <defs.h>

/*
 * оружие
 */
typedef struct
{
	//название оружия
	char name[17];
	//повреждение
	int damage;
	//повреждение
	int selfdamage;
	//макс кол-во боеприпасов(0 - бесконечно)
	int ammo;
	//радиус действия
	int radius;
	//дальность
	int range;
	//начальная скорость пули
	int bullspeed;
	//bodybox
	int bullbox;
	//изображение оружия
	item_img_t * icon;
} weapon_info_t;

/*
 * пуля
 */
typedef struct bull_s
{
	struct bull_s * next;
	//координаты
	pos_t orig;
	//игрок, выпустивший пулю
	struct player_s * player;
	//тип пули(оружие, из которого выпущена пуля)
	int _weap_;
	//направление движения
	//0 - вверх;1 - вниз;2 - влево;3 - вправо
	int dir;
	//изменение расстояния
	float delta_s;
	//время
	Ttime time;
	float frame;
	//изображение пули
	item_img_t * image;
} bull_t;

/*
 * взрыв
 */
typedef struct Texplode
{
	struct Texplode * next;
	//координаты
	pos_t orig;
	//игрок, выпустивший пулю
	struct player_s * player;
	int _weap_;
	//время
	Ttime time;
	//№ кадра
	float frame;
	//изображение взрыва
	item_img_t * image;
} explode_t;

extern weapon_info_t wtable[];
extern bull_t * bullList;
extern explode_t * explList;

void bull_add();
void bull_remove(bull_t ** bull);
void bull_removeall();
void bull_draw(camera_t * cam, bull_t * bull, bool play);
void bull_control();
void explode_add(bull_t * bull, float Xexpl, float Yexpl);
void explode_remove(explode_t ** explode);
void explode_removeall();
void explode_draw(camera_t * cam, explode_t * explode, bool play);
void explode_control();

#endif /* SRC_WEAP_H_ */
