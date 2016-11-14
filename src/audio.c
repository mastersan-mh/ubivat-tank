/*
 * audio.c
 *
 *  Created on: 6 нояб. 2016 г.
 *      Author: mastersan
 */

#include "audio.h"
#include "types.h"
#include "game.h"

#include <SDL2/SDL.h>
//#include <SDL2/SDL_mixer.h>


char *sound_files[__SOUND_NUM] =
{
		BASEDIR"/sounds/1fire1.wav",
		BASEDIR"/sounds/1fire2.wav",
		BASEDIR"/sounds/techno2.wav",
		BASEDIR"/sounds/techno3.wav"
};

typedef struct
{
	// global pointer to the audio buffer to be played
	Uint8 * audio_pos;
	// remaining length of the sample we have to play
	int32_t audio_len;

} userdata_t;
typedef struct
{
	// length of our sample
	Uint32 wav_length;
	// buffer containing our audio file
	Uint8 *wav_buffer;
	// the specs of our piece of music
	SDL_AudioSpec wav_spec;

} sound_data_t;

sound_data_t sound_table[__SOUND_NUM];


typedef struct
{
	bool play;
	sound_index_t sound_index;
	userdata_t ud;

}sound_play_t;

#define SOUND_MIX_AMOUNT 8
sound_play_t sound_plays[SOUND_MIX_AMOUNT] = {};

static int chan = 0;

typedef struct snd_format_s
{
	unsigned int	speed;
	unsigned short	width;
	unsigned short	channels;
} snd_format_t;

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

snd_renderbuffer_t * snd_renderbuffer;

// audio callback function
// here you have to copy the data of your audio buffer into the
// requesting audio buffer (stream)
// you should only copy as much as the requested length (len)
void my_audio_callback(void * userdata, Uint8 * stream, int len)
{

	//frame size
	unsigned int factor;
	unsigned int RequestedFrames;

	factor = snd_renderbuffer->format.channels * snd_renderbuffer->format.width;

	if ((unsigned int)len % factor != 0)
		game_halt("SDL sound: invalid buffer length passed to Buffer_Callback (%d bytes)\n", len);

	RequestedFrames = (unsigned int)len / factor;

	chan++;
	//if(chan % 2 != 0)
	//{
		memset(stream, 0, len);
	//}

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
	size_t i;
	for(i = 0; i< SOUND_MIX_AMOUNT; i++)
	{
		sound_play_t *sound_play = &sound_plays[i];
		if(!sound_play->play)
		{
			sound_play->ud.audio_len = sound->wav_length;
			sound_play->ud.audio_pos = sound->wav_buffer;
			sound_play->play = true;
			break;
		}
	}
}

void audio_precache()
{
	size_t i;
	for(i = 0; i< __SOUND_NUM; i++)
	{
		sound_data_t * sound = &sound_table[i];
		if(
				SDL_LoadWAV(
					sound_files[i],
					&sound->wav_spec,
					&sound->wav_buffer,
					&sound->wav_length
				)
				== NULL
		)
		{
			game_halt("Unable to load wave file: %s, %s\n", sound_files[i], SDL_GetError());
		}
	}
}

void audio_free()
{
	size_t i;
	// shut everything down
	for(i = 0; i< __SOUND_NUM; i++)
	{
		sound_data_t * sound = &sound_table[i];
		SDL_FreeWAV(sound->wav_buffer);
	}
}


void audio_init()
{

	if (SDL_Init(SDL_INIT_AUDIO) != 0)
		game_halt("audio init failed");


	audio_precache();

	// set the callback function

	//wav_spec.freq = 22050;
	//wav_spec.format = AUDIO_S16;
	//wav_spec.channels = 2;    /* 1 = mono, 2 = stereo */
	//wav_spec.samples = 1024;  /* Good low-latency value for callback */
	//wav_spec.callback = fill_audio;
	//wav_spec.userdata = NULL;

	//sound->wav_spec.channels = 1;    /* 1 = mono, 2 = stereo */

	//sound->wav_spec.callback = my_audio_callback;
	//sound->wav_spec.userdata = &sound->ud;
	// set our global static variables

	/* Open the audio device */


	SDL_AudioSpec wantspec, obtainspec;
	memset(&wantspec, 0, sizeof(wantspec));
	//memset(&have, 0, sizeof(have));


	int freq = 11025;
	int width = 1;
	int channels = 1;

	wantspec.freq = freq;//48000;
	wantspec.format = ((width == 1) ? AUDIO_U8 : AUDIO_S16SYS);
	wantspec.channels = channels;
	wantspec.samples = 4096;
	wantspec.callback = my_audio_callback;
	wantspec.userdata = NULL;             /**< Userdata passed to callback (ignored for NULL callbacks). */





	snd_renderbuffer = malloc(sizeof(*snd_renderbuffer));
	snd_renderbuffer->format.speed    = freq;
	snd_renderbuffer->format.width    = width;
	snd_renderbuffer->format.channels = channels;

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

	// Start playing
	//SDL_PauseAudioDevice(dev, 0);
	SDL_PauseAudio(0);
	sound_play_start(SOUND_MUSIC1);

	// wait until we're don't playing
	int i = 0;
	while ( 1 )
	{
		SDL_Delay(1000);
		if(i == 3)
		{
			sound_play_start(SOUND_MUSIC2);
		}
		i++;
	}

	//SDL_PauseAudioDevice(dev, 1);
	SDL_PauseAudio(1);

	//SDL_CloseAudioDevice(dev);

	audio_free();



}

void audio_done()
{
	free(snd_renderbuffer);

	//SDL_CloseAudioDevice(dev);
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


void audio_test()
{

	if (SDL_Init(SDL_INIT_AUDIO) != 0)
		game_halt("audio init failed");

	// length of our sample
	static Uint32 wav_length;
	// buffer containing our audio file
	static Uint8 *wav_buffer;
	// the specs of our piece of music
	static SDL_AudioSpec wav_spec;


	// Load the WAV
	// the specs, length and buffer of our wav are filled
	if( SDL_LoadWAV(sound_files[SOUND_MUSIC1], &wav_spec, &wav_buffer, &wav_length) == NULL )
	{
		game_halt("SDL_LoadWAV failed: %s\n", SDL_GetError());
	}
	// set the callback function

	//wav_spec.freq = 22050;
	//wav_spec.format = AUDIO_S16;
	//wav_spec.channels = 2;    // 1 = mono, 2 = stereo
	//wav_spec.samples = 1024;  // Good low-latency value for callback
	//wav_spec.callback = fill_audio;
	//wav_spec.userdata = NULL;

	wav_spec.channels = 1;    // 1 = mono, 2 = stereo

	wav_spec.callback = fill_audio;
	wav_spec.userdata = NULL;
	// set our global static variables
	audio_pos = wav_buffer; // copy sound buffer
	audio_len = wav_length; // copy file length

	if ( SDL_OpenAudio(&wav_spec, NULL) < 0 )
	{
		game_halt("Couldn't open audio: %s\n", SDL_GetError());
	}

	// Start playing
	SDL_PauseAudio(0);

	// wait until we're don't playing
	while ( audio_len > 0 )
	{
		SDL_Delay(100);
	}

	SDL_PauseAudio(1);
	// shut everything down
	SDL_FreeWAV(wav_buffer);
/**/
	exit(0);

}


