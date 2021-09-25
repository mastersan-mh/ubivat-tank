/*
 * audio.h
 *
 *  Created on: 6 нояб. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_SOUND_H_
#define SRC_SOUND_H_

typedef enum
{
	SOUND_NULL = -1,

	/* music */
	SOUND_MUSIC1,
	SOUND_MUSIC2,
	SOUND_MUSIC3,
	SOUND_MUSIC4,
	/* menu */
	SOUND_MENU_MOVE,
	SOUND_MENU_ENTER,
	SOUND_MENU_SELECT,
	/* walls */
	SOUND_WORLD_METALHIT1,
	SOUND_WORLD_METALHIT2,
	SOUND_WORLD_WATER,
	SOUND_WORLD_WIND,
	/* players */
	SOUND_PLAYER_CHANGE,
	SOUND_PLAYER_TANKMOVE,
	/* explodes */
	SOUND_EXPLODE_ARTILLERY,
	SOUND_EXPLODE_MISSILE,
	SOUND_EXPLODE_GRENADE,
	SOUND_EXPLODE_FIREBURST,
	/* weapons */
	SOUND_WEAPON_ARTILLERY_1,
	SOUND_WEAPON_ARTILLERY_2,
	SOUND_WEAPON_MISSILE_1,
	SOUND_WEAPON_MISSILE_2,
	__SOUND_NUM,
}sound_index_t;

typedef struct snd_format_s
{
	unsigned int    freq;
	unsigned short  width;
	unsigned short  channels;
} snd_format_t;

extern void sound_init(const snd_format_t * requested);
extern void sound_done(void);

extern void sound_precache(void);
extern void sound_precache_free(void);

extern int sound_play_start(sound_index_t sound_index, int loops);
extern void sound_play_pause(int playId, bool pause);
extern void sound_play_stop(int playId);
extern void sound_play_stop_all(void);

#endif /* SRC_SOUND_H_ */
