/*
 * ctrl.h
 *
 *  Created on: 9 окт. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_CTRL_H_
#define SRC_CTRL_H_

#include <defs.h>
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
}TAIbrain;

void ctrl_human         (int Pnum, struct player_s * player);
void ctrl_AI_init       (TAIbrain * brain);
void ctrl_AI_done       (TAIbrain * brain);
void ctrl_AI_checkdanger(struct player_s * player);
void ctrl_AI_attack     (struct player_s * player, struct player_s * target);
void ctrl_AI_findenemy  (struct player_s * player, struct player_s * target);


#endif /* SRC_CTRL_H_ */
