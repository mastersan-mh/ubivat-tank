/*
 * ent_exit.h
 *
 * Выход с карты
 *
 *  Created on: 9 дек. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_ENT_EXIT_H_
#define SRC_ENT_EXIT_H_

#include "entity.h"

typedef struct
{
    ENTITY_COMMON_STRUCT;
    STRING text;
} entity_exit_t;

extern void entity_exit_init(void);

#endif /* SRC_ENT_EXIT_H_ */
