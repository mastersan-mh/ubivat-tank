/*
 * client.h
 *
 *  Created on: 8 дек. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_CLIENT_H_
#define SRC_CLIENT_H_

#include "net.h"

#include "client_fsm.h"

#include "menu.h"

#include "entity.h"
#include "map.h"

#define CLIENT_TIMEOUT 30000

extern void client_init(void);
extern void client_done(void);

extern void client_flags_set(int flags);
extern void client_start(void);
extern void client_stop(void);
extern bool client_running(void);

extern bool client_ingame(void);

extern void client_handle(void);

#endif /* SRC_CLIENT_H_ */
