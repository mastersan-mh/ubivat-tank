/*
 * client.h
 *
 *  Created on: 8 дек. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_CLIENT_H_
#define SRC_CLIENT_H_

typedef struct client_storedata_s
{
	//фрагов за пройденые карты
	int fragstotal;
	//фрагов на карте
	int frags;
	int level;
	int scores;
} client_storedata_t;

typedef struct
{
	struct mobj_s * mobj;
	client_storedata_t storedata;
	void * userstoredata;
}client_t;

#include "mobjs.h"

int client_connect();

void client_disconnect_all();

int client_num_get();

client_t * client_get(int id);

void client_spawn(int id);

void client_unspawn(int id);

void client_store_all();
void client_restore_all();

#endif /* SRC_CLIENT_H_ */
