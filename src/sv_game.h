/*
 * sv_game.h
 *
 *  Created on: 12 янв. 2017 г.
 *      Author: mastersan
 */

#ifndef SRC_SV_GAME_H_
#define SRC_SV_GAME_H_

extern void sv_game_abort();
extern bool sv_game_nextmap();

extern void sv_game_message_send(const char * mess);

extern int sv_game_is_custom_game();
extern int sv_game_is_first_map();
extern void sv_game_win();

extern void sv_game_mainTick();


#endif /* SRC_SV_GAME_H_ */
