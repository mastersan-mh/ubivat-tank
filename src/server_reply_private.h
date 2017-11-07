/*
 * server_private_reply.h
 *
 *  Created on: 4 нояб. 2017 г.
 *      Author: mastersan
 */

#ifndef SRC_SERVER_REPLY_PRIVATE_H_
#define SRC_SERVER_REPLY_PRIVATE_H_

#include "types.h"
#include "server_private.h"
#include "server_reply.h"

extern void server_reply_send_info(server_client_t * client);
extern void server_reply_send_connection_accepted(server_client_t * client);
extern void server_reply_send_connection_close(server_client_t * client);
extern void server_reply_send_players_entity_set(server_client_t * client);
extern void server_reply_send_game_endmap(server_client_t * client, bool win);

#endif /* SRC_SERVER_REPLY_PRIVATE_H_ */
