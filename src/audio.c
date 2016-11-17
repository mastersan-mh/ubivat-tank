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

#define SND_MIX_SAMPLE_DOUBLE 0
#if SND_MIX_SAMPLE_DOUBLE == 1
typedef float snd_max_sample_t;
#else
typedef double snd_max_sample_t;
#endif


char *sound_files[__SOUND_NUM] =
{
		BASEDIR"/sounds/1fire1.wav",	// s16bit mono 22050
		BASEDIR"/sounds/1fire2.wav",	// s16bit mono 22050
		BASEDIR"/sounds/techno2.wav",	// u8bit mono 11025
		BASEDIR"/sounds/techno3.wav",	// u8bit mono 11025
		BASEDIR"/sounds/start.wav"  	// u8bit mono 22050
};

typedef struct
{
	void * buffer;
	size_t length;
	/* channels: 1 - mono, 2 - stereo*/
	int channels;
} sound_data_t;

sound_data_t sound_table[__SOUND_NUM];

typedef struct
{
	bool play;
	sound_index_t sound_index;

	// global pointer to the audio buffer to be played
	void * pos;
	// remaining length of the sample we have to play
	int32_t len;
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

/**
 * смешать массив buf и записать результат в dest
 */
static snd_max_sample_t __sample_array_mix_laurence(snd_max_sample_t * buf, size_t amount, snd_max_sample_t max)
{
	snd_max_sample_t a;
	snd_max_sample_t b;
	size_t mid;
	size_t size2;
	switch(amount)
	{
		case 0: return 0.0;
		case 1:	return buf[0];
		default:
			mid = amount / 2;
			size2 = amount - mid;
			a = ( mid   == 1 ? buf[0]   : __sample_array_mix_laurence(buf      , mid  , max) );
			b = ( size2 == 1 ? buf[mid] : __sample_array_mix_laurence(&buf[mid], size2, max) );
			return (a + b) / (1 + ( (a / max) * (b / max) ) );
			//return (a + b) / (1 + (a * b) / ( max * max ) );
	}
}

/**
 *
 */
static void __mix_laurence(void * stream, size_t size)
{
	memset(stream, 0, size);

	void * dest = stream;
	size_t pos;
	size_t width = snd_renderbuffer->format.width;

	uint8_t vu8;
	int16_t vs16;

	static snd_max_sample_t samples[SOUND_MIX_AMOUNT];

#define MAX_8  256.0f
#define MAX_16 32768.0f

	snd_max_sample_t max;
	switch(width)
	{
		case 1: max = MAX_8;break;
		case 2: max = MAX_16;break;
	}

	for(pos = 0; pos < size; pos += width)
	{
		size_t i;
		size_t amount = 0;
		for(i = 0; i < SOUND_MIX_AMOUNT; i++)
		{
			sound_play_t *sound_play = &sound_plays[i];
			if(!sound_play->play) continue;

			if(!sound_play->len)
			{
				sound_play->play = false;
				continue;
			}
			switch(width)
			{
				case 1: samples[amount] = *((uint8_t *) sound_play->pos); break;
				case 2: samples[amount] = *((int16_t *) sound_play->pos); break;
			}
			amount++;
			sound_play->pos += width;
			sound_play->len -= width;

		}

		snd_max_sample_t mixed = __sample_array_mix_laurence(samples, amount, max);

		switch(width)
		{
			case 1: vu8  = mixed ; memcpy(dest, &vu8, 1);break;
			case 2: vs16 = mixed ; memcpy(dest, &vs16, 2);break;
		}
		dest += width;
	}
}

// audio callback function
// here you have to copy the data of your audio buffer into the
// requesting audio buffer (stream)
// you should only copy as much as the requested length (len)
static void audio_callback(void * userdata, Uint8 * stream, int len)
{
	//frame size
	unsigned int factor;
	unsigned int RequestedFrames;

	factor = snd_renderbuffer->format.channels * snd_renderbuffer->format.width;

	if ((unsigned int)len % factor != 0)
		game_halt("SDL sound: invalid buffer length passed to Buffer_Callback (%d bytes)\n", len);

	RequestedFrames = (unsigned int)len / factor;

	// http://forum.vingrad.ru/forum/topic-74562.html
	// http://www.muzoborudovanie.ru/equip/studio/seq/cubase5/mixing.php

	//sound_mix_overlap(stream, len);
	//sound_mix_max(stream, len);

	__mix_laurence(stream, len);



}

void sound_play_start(sound_index_t isound)
{
	sound_data_t * sound = &sound_table[isound];
	if(!sound->buffer)return;
	size_t i;
	for(i = 0; i< SOUND_MIX_AMOUNT; i++)
	{
		sound_play_t *sound_play = &sound_plays[i];
		if(!sound_play->play)
		{
			sound_play->sound_index = isound;
			sound_play->len   = sound->length;
			sound_play->pos   = sound->buffer;
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

		sound->buffer = tmpbuf;
		sound->length = tmpbuf_len;
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
		Z_free(sound->buffer);
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
