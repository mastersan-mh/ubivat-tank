/*
 * ent_player_think.h
 *
 *  Created on: 9 окт. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_ENT_PLAYER_THINK_H_
#define SRC_ENT_PLAYER_THINK_H_

#include "progs.h"

#include "ent_weap.h"

extern void ctrl_AI_init(player_ai_t * brain);
extern void think_enemy(entity_t * player);

#endif /* SRC_ENT_PLAYER_THINK_H_ */
