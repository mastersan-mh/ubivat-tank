/*
 * audio.h
 *
 *  Created on: 6 нояб. 2016 г.
 *      Author: mastersan
 */

#ifndef SRC_AUDIO_H_
#define SRC_AUDIO_H_

typedef enum
{
	SOUND_NULL = -1,
	SOUND_FIRE1,
	SOUND_FIRE2,
	SOUND_MUSIC1,
	SOUND_MUSIC2,
	SOUND_START,
	__SOUND_NUM,
}sound_index_t;

typedef struct snd_format_s
{
	unsigned int    freq;
	unsigned short  width;
	unsigned short  channels;
} snd_format_t;

extern void audio_init(const snd_format_t * requested);
extern void audio_done();

extern void audio_precache();
extern void audio_free();

extern void sound_play_start(sound_index_t isound);

#endif /* SRC_AUDIO_H_ */
