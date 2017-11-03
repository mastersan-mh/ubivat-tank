/*
 * serve_fsm.h
 *
 *  Created on: 3 нояб. 2017 г.
 *      Author: mastersan
 */

#ifndef SRC_SERVER_FSM_H_
#define SRC_SERVER_FSM_H_

#include "net.h"
#include "client_requests.h"

void server_client_fsm(const net_addr_t * sender, const game_client_request_t * req);


#endif /* SRC_SERVER_FSM_H_ */
