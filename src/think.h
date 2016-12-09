/*
 * ctrl.h
 *
 *  Created on: 9 окт. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_THINK_H_
#define SRC_THINK_H_

#include "types.h"
#include "mobjs.h"
#include "weap.h"

//искуственный интеллект
typedef struct
{
	//флаг опастности
	bool danger;
	//флаг опастности
	bool Fdanger;
	bool attack;
	weapontype_t weap;
	//цель
	struct mobj_s * target;
	//счетчик
	long count;
}think_t;

#include "player.h"

void ctrl_AI_init(think_t * brain);
void ctrl_AI_done(think_t * brain);

void think_human(mobj_t * player);
void think_enemy(mobj_t * player);



#endif /* SRC_THINK_H_ */
