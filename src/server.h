/*
 * server.h
 *
 *  Created on: 27 дек. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_SERVER_H_
#define SRC_SERVER_H_

/* флаги параметров игры */
typedef struct
{
    bool localgame; /**< level-by-level "single player" */
    bool allow_respawn;
} server_gameflags_t;

extern void server_init(void);
extern void server_done(void);



extern void server_start(int flags);
extern void server_stop(void);

extern bool server_running(void);

extern void server_handle(void);

#endif /* SRC_SERVER_H_ */
