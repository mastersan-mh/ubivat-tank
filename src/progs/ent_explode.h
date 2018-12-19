/*
 * ent_explode.h
 *
 * Взрыв
 *
 *  Created on: 29 нояб. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_ENT_EXPLODE_H_
#define SRC_ENT_EXPLODE_H_

#include "progs.h"
#include "progs_main.h"

typedef enum
{
	EXPLODE_ARTILLERY,
	EXPLODE_MISSILE,
	EXPLODE_MINE,
	EXPLODE_NUM
} explodetype_t;

typedef struct
{
	//повреждение
	int damage;
	//повреждение
	int selfdamage;
	/* маска тексур стен, которые можно разрушить взрывом */
	char wall;
	sound_index_t soundIndex;
} explodeinfo_t;

extern explodeinfo_t explodeinfo_table[EXPLODE_NUM];

entity_t * explode_artillery_spawn(entity_t * parent, const char * spawninfo);
entity_t * explode_missile_spawn(entity_t * parent, const char * spawninfo);
entity_t * explode_mine_spawn(entity_t * parent, const char * spawninfo);

#endif /* SRC_ENT_EXPLODE_H_ */
