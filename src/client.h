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

extern void client_players_num_set(int players_num);
extern void client_start(void);
extern void client_stop(void);
extern bool client_running(void);

extern bool client_ingame(void);

extern bool client_world_valid(void);
extern void client_world_valid_set(bool valid);

extern void client_handle(void);

#endif /* SRC_CLIENT_H_ */
