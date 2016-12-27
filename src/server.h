/*
 * server.h
 *
 *  Created on: 27 дек. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_SERVER_H_
#define SRC_SERVER_H_

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
	struct entity_s * entity;
	client_storedata_t storedata;
	void * userstoredata;
}client_t;

#include "entity.h"

int server_client_connect();

void server_disconnect_clients();

int server_client_num_get();

client_t * server_client_get(int id);

int server_spawn_client(int id);

void server_unspawn_client(int id);

void server_store_clients_info();
void server_restore_clients_info();

void server_events_pump();

#endif /* SRC_SERVER_H_ */
