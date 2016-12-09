/*
 * weap.h
 *
 *  Created on: 10 окт. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_WEAP_H_
#define SRC_WEAP_H_

#include "types.h"
#include "img.h"
#include "mobjs.h"

#include "bull.h"
#include "explode.h"

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

bulltype_t mobj_weapon_type_to_bull_type(weapontype_t type);
explodetype_t mobj_bull_type_to_explode_type(bulltype_t bull_type);
mobj_type_t mobl_byulltype_to_mobj(bulltype_t bulltype);
mobj_type_t mobj_explodetype_to_mobjtype(explodetype_t type);

#endif /* SRC_WEAP_H_ */
