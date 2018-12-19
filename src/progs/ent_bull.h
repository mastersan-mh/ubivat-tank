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
#include "progs_main.h"

entity_t * bull_artillery_spawn(entity_t * parent, const char * spawninfo);
entity_t * bull_missile_spawn(entity_t * parent, const char * spawninfo);
entity_t * bull_mine_spawn(entity_t * parent, const char * spawninfo);

#endif /* SRC_ENT_BULL_H_ */
