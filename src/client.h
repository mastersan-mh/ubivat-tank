/*
 * client.h
 *
 *  Created on: 8 дек. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_CLIENT_H_
#define SRC_CLIENT_H_

#include "mobjs.h"

typedef struct
{
	mobj_t * mobj;
	//фрагов за пройденые карты
	long fragstotal;
	//фрагов на карте
	long frags;
	int items[__ITEM_NUM];
}client_t;

int client_connect();
void client_disconnect_all();
client_t * client_get(int id);
void client_spawn(int id);
void client_unspawn(int id);


#endif /* SRC_CLIENT_H_ */
