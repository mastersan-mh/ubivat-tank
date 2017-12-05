/*
 * ent_bull.h
 *
 * Пули
 *
 *  Created on: 30 нояб. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_ENT_BULL_H_
#define SRC_ENT_BULL_H_

#include "progs.h"

ENTITY bull_artillery_spawn(ENTITY parent, const char * spawninfo);
ENTITY bull_missile_spawn(ENTITY parent, const char * spawninfo);
ENTITY bull_mine_spawn(ENTITY parent, const char * spawninfo);

#endif /* SRC_ENT_BULL_H_ */
