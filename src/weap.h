/*
 * weap.h
 *
 *  Created on: 10 окт. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_WEAP_H_
#define SRC_WEAP_H_

#include <img.h>
#include <types.h>

typedef enum
{
	WEAP_NONE,
	WEAP_BULL,
	WEAP_ROCKET,
	WEAP_MINE,
} weapon_t;


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
	coord_t radius;
	//дальность
	coord_t range;
	//начальная скорость пули
	coord_t bullspeed;
	//bodybox
	coord_t bullbox;
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
	pos_t pos;
	//игрок, выпустивший пулю
	struct player_s * player;
	//тип пули(оружие, из которого выпущена пуля)
	int _weap_;
	//направление движения
	//0 - вверх;1 - вниз;2 - влево;3 - вправо
	int dir;
	//изменение расстояния
	coord_t delta_s;
	//время
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
	pos_t pos;
	//игрок, выпустивший пулю
	struct player_s * player;
	int _weap_;
	//№ кадра
	float frame;
	//изображение взрыва
	item_img_t * image;
} explode_t;

extern weapon_info_t wtable[];
extern bull_t * bullList;
extern explode_t * explList;

void bull_add(void);
void bull_remove(bull_t ** bull);
void bull_removeall(void);
void bull_control(void);
void bull_draw_all(camera_t * cam);
void explode_add(bull_t * bull, coord_t Xexpl, coord_t Yexpl);
void explode_remove(explode_t ** explode);
void explode_removeall(void);
void explode_control(void);
void explode_draw_all(camera_t * cam);

#endif /* SRC_WEAP_H_ */
