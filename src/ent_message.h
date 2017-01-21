/*
 * ent_message.h
 *
 *  Created on: 9 дек. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_ENT_MESSAGE_H_
#define SRC_ENT_MESSAGE_H_

/**
 * сообщение
 */
typedef struct
{
	char * message;
} ent_message_t;

#define ENT_MESSAGE(x) ((ent_message_t *)(x)->data)

extern void entity_message_init(void);

#endif /* SRC_ENT_MESSAGE_H_ */
