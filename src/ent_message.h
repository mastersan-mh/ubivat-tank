/*
 * ent_message.h
 *
 * Сообщение
 *
 *  Created on: 9 дек. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_ENT_MESSAGE_H_
#define SRC_ENT_MESSAGE_H_

#include "entity.h"

typedef struct
{
    ENTITY_COMMON_STRUCT;
    STRING text;
} entity_message_t;

extern void entity_message_init(void);

#endif /* SRC_ENT_MESSAGE_H_ */
