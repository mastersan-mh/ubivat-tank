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
	WEAP_ARTILLERY,
	WEAP_MISSILE,
	WEAP_MINE,
	__WEAP_NUM
} weapontype_t;

/*
 * оружие
 */
typedef struct
{
	//название оружия
	char * name;
	//изображение оружия
	image_index_t icon;
} weaponinfo_t;

extern weaponinfo_t wtable[__WEAP_NUM];

#endif /* SRC_WEAP_H_ */
