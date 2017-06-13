#ifndef AudioMicH
#define AudioMicH

#include <pthread.h>
#include <alsa/asoundlib.h>
#include "AudioEffects.h"

#define mono 1
#define stereo 2

typedef struct
{
	pthread_mutex_t *haasmutex;
	int haasenabled;
	float millisec;
	snd_pcm_format_t format;
	unsigned int rate;
	char* delayed;
	int delaybytes;
	signed short *delayedbuffer;
	int inbuffersize, inbufferframes;

	int sbuffersize;
	char *sbuffer, *mbuffer;
	signed short *stereobuffer, *monobuffer;
	soundmod sndmod; 
}michaas;

typedef enum
{
	MC_RUNNING = 0,
	MC_STOPPED
}micstatus;

typedef struct
{
	char device[32];
	snd_pcm_format_t format;
	unsigned int rate;
	unsigned int channels;
	snd_pcm_t *capture_handle;
	snd_pcm_hw_params_t *hw_params;
	micstatus status;
	char *buffer;
	int buffersize, bufferframes, buffersamples;
	michaas mh;
}microphone;

int init_audio_hw_mic(microphone *m);
void close_audio_hw_mic(microphone *m);
void haas_reinit(michaas *h, float millisec);
void haas_init(michaas *h, int enabled, float millisec, snd_pcm_format_t format, unsigned int rate, int inbuffersize, char *inbuffer, int modenabled, float modrate, float moddepth);
void haas_add(michaas *h);
void haas_close(michaas *h);
void init_mic(microphone *m, char* device, snd_pcm_format_t format, unsigned int rate, int outframes, int haasenabled, float haasms, int modenabled, float modrate, float moddepth);
int read_mic(microphone *m);
void signalstop_mic(microphone *m);
void close_mic(microphone *m);

#endif
