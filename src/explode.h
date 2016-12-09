/*
 * explode.h
 *
 *  Created on: 29 нояб. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_EXPLODE_H_
#define SRC_EXPLODE_H_

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
	//игрок, выпустивший пулю
	explodetype_t type;
	struct mobj_s * owner;
	int state;
	float frame;
} explode_t;

#define EXPLODE(x) ((explode_t *)(x)->data)

extern explodeinfo_t explodeinfo_table[__EXPLODE_NUM];

void mobj_explode_init();

void explode_draw(camera_t * cam, mobj_t * explode);

#endif /* SRC_EXPLODE_H_ */
