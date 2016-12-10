/*
 * ent_exit.h
 *
 *  Created on: 9 дек. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_ENT_EXIT_H_
#define SRC_ENT_EXIT_H_

/**
 * сообщение
 */
typedef struct
{
	char * message;
} ent_exit_t;

#define ENT_EXIT(x) ((ent_exit_t *)(x)->data)

void mobj_exit_init();

#endif /* SRC_ENT_EXIT_H_ */
