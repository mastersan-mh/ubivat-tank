/*
 * ent_player_think.h
 *
 *  Created on: 9 окт. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_ENT_PLAYER_THINK_H_
#define SRC_ENT_PLAYER_THINK_H_

#include "types.h"
#include "entity.h"
#include "ent_weap.h"

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
	entity_t * target;
	//счетчик
	long count;
}think_t;

void ctrl_AI_init(think_t * brain);
void ctrl_AI_done(think_t * brain);

void think_human(entity_t * player);
void think_enemy(entity_t * player);

#endif /* SRC_ENT_PLAYER_THINK_H_ */