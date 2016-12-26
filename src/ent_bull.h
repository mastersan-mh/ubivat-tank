/*
 * ent_bull.h
 *
 *  Created on: 30 нояб. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_ENT_BULL_H_
#define SRC_ENT_BULL_H_

#include "types.h"

typedef enum
{
	BULL_ARTILLERY,
	BULL_MISSILE,
	BULL_MINE,
	__BULL_NUM
} bulltype_t;

typedef struct
{
	//повреждение
	int damage;
	//повреждение
	int selfdamage;
	//дальность
	vec_t range;
	//начальная скорость пули
	vec_t speed;
	//bodybox
	vec_t bodybox;
} bullinfo_t;

/*
 * пуля
 */
typedef struct
{
	//тип пули
	bulltype_t type;
	//изменение расстояния
	vec_t delta_s;
} bull_t;

#define ENT_BULL(x) ((bull_t *)(x)->data)

extern bullinfo_t bullinfo_table[__BULL_NUM];

void entity_bull_init();

#endif /* SRC_ENT_BULL_H_ */
