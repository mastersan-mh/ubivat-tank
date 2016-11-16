/*
 * audio.c
 *
 *  Created on: 6 нояб. 2016 г.
 *      Author: mastersan
 */

#include "types.h"
#include "audio.h"
#include "game.h"

#include <SDL2/SDL.h>
//#include <SDL2/SDL_mixer.h>


char *sound_files[__SOUND_NUM] =
{
		BASEDIR"/sounds/1fire1.wav",	// s16bit mono 22050
//		BASEDIR"/sounds/1fire2.wav",	// s16bit mono 22050
//		BASEDIR"/sounds/techno2.wav",	// u8bit mono 11025
//		BASEDIR"/sounds/techno3.wav",	// u8bit mono 11025
		BASEDIR"/sounds/start.wav"  	// u8bit mono 22050
};

typedef struct
{
	uint8_t * wav_buffer;
	size_t wav_length;
	/* channels: 1 - mono, 2 - stereo*/
	int channels;
} sound_data_t;

typedef struct
{
	sound_index_t index;
	// global pointer to the audio buffer to be played
	Uint8 * audio_pos;
	// remaining length of the sample we have to play
	int32_t audio_len;

} userdata_t;

sound_data_t sound_table[__SOUND_NUM];


typedef struct
{
	bool play;
	sound_index_t sound_index;
	userdata_t ud;

}sound_play_t;

#define SOUND_MIX_AMOUNT 32
sound_play_t sound_plays[SOUND_MIX_AMOUNT] = {};


typedef struct snd_ringbuffer_s
{
	snd_format_t		format;
	unsigned char*		ring;
	unsigned int		maxframes;	// max size (buffer size), in sample frames
	unsigned int		startframe;	// index of the first frame in the buffer
									// if startframe == endframe, the bufffer is empty
	unsigned int		endframe;	// index of the first EMPTY frame in the "ring" buffer
									// may be smaller than startframe if the "ring" buffer has wrapped
}snd_renderbuffer_t;

snd_renderbuffer_t * snd_renderbuffer = NULL;

/**
 * wav mono 8 bit to 16 bit
 */
static int16_t wav_mono_u8tos16(uint8_t sample8)
{
	return (int16_t) (sample8 - 0x80) << 8;
}

static uint8_t wav_mono_s16tou8(int16_t sample16)
{
	return (uint8_t)(sample16 >> 8) + 0x80;
}

static int audioFormat2width(SDL_AudioFormat format)
{
	switch(format)
	{
		case AUDIO_U8    : return 1;
		case AUDIO_S16SYS: return 2;
	}
	return 0;
}

// audio callback function
// here you have to copy the data of your audio buffer into the
// requesting audio buffer (stream)
// you should only copy as much as the requested length (len)
void audio_callback(void * userdata, Uint8 * stream, int len)
{
	//frame size
	unsigned int factor;
	unsigned int RequestedFrames;

	factor = snd_renderbuffer->format.channels * snd_renderbuffer->format.width;

	if ((unsigned int)len % factor != 0)
		game_halt("SDL sound: invalid buffer length passed to Buffer_Callback (%d bytes)\n", len);

	RequestedFrames = (unsigned int)len / factor;

	memset(stream, 0, len);

	size_t sound_play_len;
	size_t i;
	for(i = 0; i < SOUND_MIX_AMOUNT; i++)
	{
		sound_play_t *sound_play = &sound_plays[i];
		if(!sound_play->play) continue;

		userdata_t * ud = &sound_play->ud;

		if(!ud->audio_len)
		{
			sound_play->play = false;
			continue;
		}

		sound_play_len = ( len > ud->audio_len ? ud->audio_len : len );
		// simply copy from one buffer into the other
		memcpy(stream, ud->audio_pos, sound_play_len);
		//SDL_MixAudio(stream, __ud->audio_pos, len, SDL_MIX_MAXVOLUME);// mix from one buffer into another
		// mix our audio against the silence, at 50% volume.
		//SDL_MixAudioFormat(stream, mixData, deviceFormat, len, SDL_MIX_MAXVOLUME / 2);
		ud->audio_pos += sound_play_len;
		ud->audio_len -= sound_play_len;


	}
}

void sound_play_start(sound_index_t isound)
{
	sound_data_t * sound = &sound_table[isound];
	if(!sound->wav_buffer)return;
	size_t i;
	for(i = 0; i< SOUND_MIX_AMOUNT; i++)
	{
		sound_play_t *sound_play = &sound_plays[i];
		if(!sound_play->play)
		{
			sound_play->ud.index     = isound;
			sound_play->ud.audio_len = sound->wav_length;
			sound_play->ud.audio_pos = sound->wav_buffer;
			sound_play->play = true;
			break;
		}
	}
}

static void * convert(
	const void * __src_buffer,
	int __src_width , int __src_freq , size_t __src_length,
	int __dest_width, int __dest_freq, size_t * __dest_length

)
{
//#error сделать потоковое преобразование

	typedef union
	{
		uint8_t  u8;
		int8_t   s8;
		uint16_t u16;
		int16_t  s16;
		uint16_t data;
	}data_t;
	data_t data_src;
	data_src.data = 0;
	data_t data_dest;
	data_dest.data = 0;

	void * __dest_buffer = NULL;
	size_t src_sh;
	uint8_t * buf = (uint8_t *) __src_buffer;
	size_t dest_sh;
	*__dest_length = 0;

	int i = 0;

	for(src_sh = 0, dest_sh = 0; src_sh < __src_length; src_sh += __src_width)
	{
		memcpy(&data_src, buf, __src_width);
		buf += __src_width;

		// ширина
		switch(__src_width)
		{
			case 1:
				switch(__dest_width)
				{
					case 1: data_dest.u8 = data_src.u8; break;
					case 2: data_dest.s16 = wav_mono_u8tos16(data_src.u8); break;
					default: goto error;
				}
				break;
			case 2:
				switch(__dest_width)
				{
					case 1: data_dest.u8  = wav_mono_s16tou8(data_src.s16); break;
					case 2: data_dest.s16 = data_src.s16; break;
					default: goto error;
				}
				break;
			default: goto error;
		}
		data_src = data_dest;

		// частота
		if(__src_freq == __dest_freq)
		{

			// размер буфера назначения неизвестен, вычислим
			if(!__dest_buffer)
			{
				*__dest_length = __src_length * __dest_width;
				__dest_buffer = Z_malloc(*__dest_length);
				if(!__dest_buffer) goto error;
			}

			memcpy(__dest_buffer + dest_sh, &data_src, __dest_width);
			dest_sh += __dest_width;

		}
		else if(__src_freq < __dest_freq)
		{

			int freq_factor = __dest_freq / __src_freq;

			// размер буфера назначения неизвестен, вычислим
			if(!__dest_buffer)
			{
				*__dest_length = __src_length * __dest_width * freq_factor;
				__dest_buffer = Z_malloc(*__dest_length);
				if(!__dest_buffer) goto error;
			}

			for(int freq_repeat = 0; freq_repeat < freq_factor; freq_repeat++)
			{
				memcpy(__dest_buffer + dest_sh, &data_src, __dest_width);
				dest_sh += __dest_width;
			}

		}
		else
		{
			/* __src_freq > __dest_freq */

			//goto error;

			int freq_factor = __src_freq / __dest_freq;

			// размер буфера назначения неизвестен, вычислим
			if(!__dest_buffer)
			{
				*__dest_length = (__src_length * __dest_width) / freq_factor;
				__dest_buffer = Z_malloc(*__dest_length);
				if(!__dest_buffer) goto error;
			}

			i++;

			if(i % freq_factor)
			{
				memcpy(__dest_buffer + dest_sh, &data_src, __dest_width);
				dest_sh += __dest_width;
			}

		}

	}
	return __dest_buffer;
	error:
	Z_free(__dest_buffer);
	return NULL;

}

/**
 *
 */
void audio_precache()
{
	size_t i;


	for(i = 0; i< __SOUND_NUM; i++)
	{
		SDL_AudioSpec wav_spec;
		Uint8 * wav_buffer;
		Uint32 wav_length;
		sound_data_t * sound = &sound_table[i];
		if(
				SDL_LoadWAV(
					sound_files[i],
					&wav_spec,
					&wav_buffer,
					&wav_length
				)
				== NULL
		)
		{
			game_halt("Unable to load wave file: %s, %s\n", sound_files[i], SDL_GetError());
		}

		int wav_width = audioFormat2width(wav_spec.format);
		int wav_freq = wav_spec.freq;

		int dev_width = snd_renderbuffer->format.width;
		int dev_freq = snd_renderbuffer->format.freq;

		size_t tmpbuf_len;
		void * tmpbuf;

		tmpbuf = convert(
			wav_buffer,
			wav_width, wav_freq, wav_length,
			dev_width, dev_freq, &tmpbuf_len
		);

		SDL_FreeWAV(wav_buffer);
		if(!tmpbuf)
		{
			game_halt("AUDIO: Can not convert");
		}

		sound->wav_buffer = tmpbuf;
		sound->wav_length = tmpbuf_len;
		sound->channels   = wav_spec.channels;
	}
}

snd_renderbuffer_t * snd_buffer_alloc(const snd_format_t * requested)
{
	snd_renderbuffer_t * buf;
	buf = Z_malloc(sizeof(*snd_renderbuffer));
	buf->format = *requested;
	return buf;
}

void snd_buffer_free(snd_renderbuffer_t * buf)
{
	Z_free(buf);
}

/**
 *
 */
void audio_free()
{
	size_t i;
	// shut everything down
	for(i = 0; i< __SOUND_NUM; i++)
	{
		sound_data_t * sound = &sound_table[i];
		Z_free(sound->wav_buffer);
	}
}


void audio_init(const snd_format_t * requested)
{

	if (SDL_Init(SDL_INIT_AUDIO) != 0)
		game_halt("audio init failed");

	/* Open the audio device */
	SDL_AudioSpec wantspec, obtainspec;

	memset(&wantspec, 0, sizeof(wantspec));
	//memset(&have, 0, sizeof(have));


	wantspec.freq = requested->freq;//48000;
	wantspec.format = ((requested->width == 1) ? AUDIO_U8 : AUDIO_S16SYS);
	wantspec.channels = requested->channels;
	wantspec.samples = 4096;
	wantspec.callback = audio_callback;
	wantspec.userdata = NULL;

	// Open the audio device
/*
	const char * dev_name = SDL_GetAudioDeviceName(1, 0);
	SDL_AudioDeviceID dev;
	dev = SDL_OpenAudioDevice(dev_name, 0, &sound->wav_spec, NULL, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
	if (dev == 0)
	{
		game_halt("Failed to open audio: %s\n", SDL_GetError());
	}
*/

	if ( SDL_OpenAudio(&wantspec, &obtainspec) < 0 )
	{
		game_halt("Failed to open the audio device! (%s)\n", SDL_GetError());
	}

	if(
			wantspec.freq != obtainspec.freq ||
			wantspec.format != obtainspec.format ||
			wantspec.channels != obtainspec.channels
	)
	{
		SDL_CloseAudio();
		game_halt("Couldn't open audio: %s\n", SDL_GetError());
	}

	snd_renderbuffer = snd_buffer_alloc(requested);

	// Start playing
	//SDL_PauseAudioDevice(dev, 0);

	SDL_PauseAudio(0);

}

void audio_done()
{
	//SDL_PauseAudioDevice(dev, 1);
	//SDL_CloseAudioDevice(dev);
	SDL_PauseAudio(1);
	snd_buffer_free(snd_renderbuffer);
	SDL_CloseAudio();
}


static Uint8 * audio_pos;
// remaining length of the sample we have to play
static int32_t audio_len;


static int i = 0;
void fill_audio(void *udata, Uint8 *stream, int len)
{
	// Only play if we have data left
	if ( audio_len == 0 )
		return;

	// Mix as much data as possible
	len = ( len > audio_len ? audio_len : len );
	SDL_MixAudio(stream, audio_pos, len, SDL_MIX_MAXVOLUME);
	i++;
	if(i % 2 != 0) return;
	audio_pos += len;
	audio_len -= len;
}
