/*
 * weap.h
 *
 *  Created on: 10 окт. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_ENT_WEAP_H_
#define SRC_ENT_WEAP_H_

#include "ent_bull.h"
#include "ent_explode.h"

#include "types.h"
#include "img.h"
#include "entity.h"

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

bulltype_t entity_weapon_type_to_bull_type(weapontype_t type);
explodetype_t entity_bull_type_to_explode_type(bulltype_t bull_type);
const char * entity_bulltype_to_mobj(bulltype_t bulltype);
const char * entity_explodetype_to_mobjtype(explodetype_t type);

#endif /* SRC_ENT_WEAP_H_ */
