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

typedef struct
{
    ENTITY_VARS_COMMON_STRUCT;
} bull_vars_t;

void entity_bull_init(void);

#endif /* SRC_ENT_BULL_H_ */
