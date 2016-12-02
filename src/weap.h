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
	WEAP_MISSILE,
	WEAP_MINE,
	__WEAP_NUM
} weapon_t;

//оружие бесконечно
#define WEAP_AMMO_INFINITE 0


/*
 * оружие
 */
typedef struct
{
	//название оружия
	char * name;
	//повреждение
	int damage;
	//повреждение
	int selfdamage;
	//макс кол-во боеприпасов(0 - бесконечно)
	int ammo;
	//дальность
	coord_t range;
	//начальная скорость пули
	coord_t bullspeed;
	//bodybox
	coord_t bullbox;
	//изображение оружия
	image_index_t icon;
} weapon_info_t;

extern weapon_info_t wtable[3];

#endif /* SRC_WEAP_H_ */
