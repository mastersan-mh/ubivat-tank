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

extern explodetype_t weapontype_to_explodetype(weapontype_t weapontype);

extern const char * weapontype_to_bullentity(weapontype_t weapontype);
extern explodetype_t bullentity_to_explodetype(entity_t * bull);
const char * explodetype_to_explodeentity(explodetype_t type);

#endif /* SRC_ENT_WEAP_H_ */
