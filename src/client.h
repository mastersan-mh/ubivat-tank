/*
 * client.h
 *
 *  Created on: 8 дек. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_CLIENT_H_
#define SRC_CLIENT_H_

#include "net.h"
#include "g_events.h"

#include "entity.h"

#define CLIENT_TIMEOUT 30000

typedef struct client_s
{
	struct client_s * prev;
	struct client_s * next;

	/* ассоциированный сокет */
	net_socket_t * ns;
	enum
	{
		CLIENT_AWAITING_CONNECTION,
		CLIENT_LISTEN
	} state;

	/* время последнего получения сообщения */
	unsigned long time;

	entity_t * entity;
	camera_t cam;

} client_t;

extern int client_connect();
extern void clients_initcams();

extern void client_event_send(client_t * client, gclientevent_t * event);

extern void client_event_control_send(int clientId, const char * action_name);

extern void client();
extern void client_draw();


#endif /* SRC_CLIENT_H_ */
