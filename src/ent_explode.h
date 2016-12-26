/*
 * ent_explode.h
 *
 *  Created on: 29 нояб. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_ENT_EXPLODE_H_
#define SRC_ENT_EXPLODE_H_

#include "vec.h"

typedef enum
{
	EXPLODE_ARTILLERY,
	EXPLODE_MISSILE,
	EXPLODE_MINE,
	__EXPLODE_NUM
} explodetype_t;

typedef struct
{
	//повреждение
	int damage;
	//повреждение
	int selfdamage;
	//радиус действия
	vec_t radius;
} explodeinfo_t;

/*
 * взрыв
 */
typedef struct
{
	explodetype_t type;
	//игрок, выпустивший пулю
	struct entlink_s * owner;
} explode_t;

#define EXPLODE(x) ((explode_t *)(x)->data)

extern explodeinfo_t explodeinfo_table[__EXPLODE_NUM];

void entity_explode_init();

#endif /* SRC_ENT_EXPLODE_H_ */
