/*
 * cl_game.h
 *
 *  Created on: 12 янв. 2017 г.
 *      Author: mastersan
 */

#ifndef SRC_CL_GAME_H_
#define SRC_CL_GAME_H_

#include "types.h"

extern bool cl_game_quit_get(void);
extern void cl_game_quit_set(void);
extern void cl_game_draw(void);
extern void cl_game_action_showmenu(void);
extern int cl_game_create(int flags);
extern void cl_game_abort(void);

#endif /* SRC_CL_GAME_H_ */
