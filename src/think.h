/*
 * ctrl.h
 *
 *  Created on: 9 окт. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_THINK_H_
#define SRC_THINK_H_

#include <types.h>
//искуственный интеллект
typedef struct
{
	//флаг опастности
	bool danger;
	//флаг опастности
	bool Fdanger;
	//0 -нет атаки;[1..3] -атака
	int weapon;
	//цель
	struct player_s * target;
	//счетчик
	long count;
}think_t;

void ctrl_AI_init       (think_t * brain);
void ctrl_AI_done       (think_t * brain);

void think_human(int Pnum, struct player_s * player);
void think_enemy(struct player_s * player);



#endif /* SRC_THINK_H_ */
