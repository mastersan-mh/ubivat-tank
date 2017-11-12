/*
 * ent_items.h
 *
 *  Created on: 3 дек. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_ENT_ITEMS_H_
#define SRC_ENT_ITEMS_H_

#include "entity.h"

typedef struct
{
    ENTITY_COMMON_STRUCT;
    INTEGER amount;
} entity_item_t;

extern void entity_items_init(void);

#endif /* SRC_ENT_ITEMS_H_ */
