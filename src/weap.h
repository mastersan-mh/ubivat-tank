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


extern weapon_info_t wtable[];

#endif /* SRC_WEAP_H_ */
