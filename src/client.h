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
#include "menu.h"

#include "entity.h"
#include "map.h"

#include "client_request.h"

#define CLIENT_TIMEOUT 30000





extern void client_init(void);
extern void client_done(void);

extern void client_start(int flags);
extern void client_stop(void);
extern bool client_running(void);

extern gamestate_t client_gamestate_get(void);

extern void client_event_key_input(int key, bool state);

extern void client_events_pump(void);

extern void client_handle(void);

#endif /* SRC_CLIENT_H_ */
