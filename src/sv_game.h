/*
 * sv_game.h
 *
 *  Created on: 12 янв. 2017 г.
 *      Author: mastersan
 */

#ifndef SRC_SV_GAME_H_
#define SRC_SV_GAME_H_

extern void sv_game_abort(void);
extern bool sv_game_nextmap(void);

extern void sv_game_message_send(const char * mess);

extern int sv_game_is_custom_game(void);
extern int sv_game_is_first_map(void);
extern void sv_game_win(void);

extern void sv_game_mainTick(void);


#endif /* SRC_SV_GAME_H_ */