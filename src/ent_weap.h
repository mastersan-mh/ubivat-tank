/*
 * ent_weap.h
 *
 * Оружие
 *
 *  Created on: 10 окт. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_ENT_WEAP_H_
#define SRC_ENT_WEAP_H_

#include "types.h"
#include "img.h"
#include "sound.h"
#include "entity.h"

typedef enum
{
	WEAP_ARTILLERY,
	WEAP_MISSILE,
	WEAP_MINE,
	WEAP_NUM
} weapontype_t;

typedef struct
{
	//название оружия
	char * name;
	//изображение оружия
	image_index_t icon;
	char * entityname;
	sound_index_t sound_index;
} weaponinfo_t;

extern weaponinfo_t weaponinfo_table[WEAP_NUM];

#endif /* SRC_ENT_WEAP_H_ */
