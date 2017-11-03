/*
 * server_reply.h
 *
 *  Created on: 3 нояб. 2017 г.
 *      Author: mastersan
 */

#ifndef SRC_SERVER_REPLY_H_
#define SRC_SERVER_REPLY_H_

extern void server_reply_send(server_client_t * client, const game_server_event_t * req);

extern void server_reply_send_info(server_client_t * client);
extern void server_reply_send_connection_accepted(server_client_t * client);
extern void server_reply_send_player_join_awaiting(server_client_t * client);
extern void server_event_send_win(void);
extern void server_reply_send_cliententity(server_client_t * client);
extern void server_reply_send_gamestate(server_client_t * client, gamestate_t state);


#endif /* SRC_SERVER_REPLY_H_ */
