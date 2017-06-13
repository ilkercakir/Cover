/*
 * AudioMic.c
 * 
 * Copyright 2017  <pi@raspberrypi>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */
#include "AudioMic.h"

int init_audio_hw_mic(microphone *m)
{
	int err;

	snd_pcm_hw_params_alloca(&(m->hw_params));
	//snd_pcm_sw_params_alloca(&sw_params);

	if ((err = snd_pcm_open(&(m->capture_handle), m->device, SND_PCM_STREAM_CAPTURE, 0)) < 0)
	{
		printf("cannot open audio device %s (%s)\n", m->device, snd_strerror(err));
		return(err);
	}

	if ((err = snd_pcm_hw_params_any(m->capture_handle, m->hw_params)) < 0)
	{
		printf("cannot initialize hardware parameter structure (%s)\n", snd_strerror(err));
		return(err);
  }

	if ((err = snd_pcm_hw_params_set_access(m->capture_handle, m->hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0)
	{
		printf("cannot set access type (%s)\n", snd_strerror(err));
		return(err);
	}

	if ((err = snd_pcm_hw_params_set_format(m->capture_handle, m->hw_params, m->format)) < 0)
	{
		printf("cannot set sample format (%s)\n", snd_strerror(err));
		return(err);
	}

	if ((err = snd_pcm_hw_params_set_rate_near(m->capture_handle, m->hw_params, &(m->rate), 0)) < 0)
	{
		printf("cannot set sample rate (%s)\n", snd_strerror(err));
		return(err);
	}

	if ((err = snd_pcm_hw_params_set_channels(m->capture_handle, m->hw_params, m->channels)) < 0)
	{
		printf("cannot set channel count (%s)\n", snd_strerror(err));
		return(err);
	}

	if ((err = snd_pcm_hw_params(m->capture_handle, m->hw_params)) < 0)
	{
		printf("cannot set parameters (%s)\n", snd_strerror(err));
		return(err);
	}

	if ((err = snd_pcm_prepare(m->capture_handle)) < 0)
	{
		printf("cannot prepare audio interface for use (%s)\n", snd_strerror(err));
		return(err);
	}

//printf("initialized format %d rate %d channels %d\n", m->format, m->rate, m->channels);
	return(err);
}

void close_audio_hw_mic(microphone *m)
{
	snd_pcm_close(m->capture_handle);
}

void haas_reinit(michaas *h, float millisec)
{
	free(h->delayed);
	h->millisec = millisec;
	int delayframes = h->millisec / 1000.0 * h->rate;
	h->delaybytes = delayframes * ( snd_pcm_format_width(h->format) / 8 );
	h->delayed = malloc((h->inbufferframes+delayframes) * ( snd_pcm_format_width(h->format) / 8 ));
	memset(h->delayed+h->inbuffersize, 0, h->delaybytes);
	h->delayedbuffer = (signed short *)h->delayed;
	//printf("%f, %d, %d\n", millisec, delayframes, delaybytes);
}

void haas_init(michaas *h, int enabled, float millisec, snd_pcm_format_t format, unsigned int rate, int inbuffersize, char *inbuffer, int modenabled, float modrate, float moddepth)
{
	int ret;

	h->haasenabled = enabled;
	h->millisec = millisec;
	h->format = format;
	h->rate = rate;

	h->haasmutex = malloc(sizeof(pthread_mutex_t));
	if((ret=pthread_mutex_init(h->haasmutex, NULL))!=0 )
		printf("mutex init failed, %d\n", ret);

	h->inbuffersize = inbuffersize;
	h->inbufferframes = h->inbuffersize / snd_pcm_format_width(h->format) * 8;
	int delayframes = h->millisec / 1000.0 * h->rate;
	h->delaybytes = delayframes * ( snd_pcm_format_width(h->format) / 8 );
	h->delayed = malloc((h->inbufferframes+delayframes) * ( snd_pcm_format_width(h->format) / 8 ));
	memset(h->delayed+h->inbuffersize, 0, h->delaybytes);
	h->delayedbuffer = (signed short *)h->delayed;
	//printf("%f, %d, %d\n", millisec, delayframes, delaybytes);

	h->sbuffersize = h->inbufferframes * stereo * ( snd_pcm_format_width(h->format) / 8 );
	h->sbuffer = malloc(h->sbuffersize);
	h->stereobuffer = (signed short *)h->sbuffer;

	h->mbuffer = inbuffer;
	h->monobuffer = (signed short *)h->mbuffer;

	h->sndmod.enabled = modenabled;
	soundmod_init(modrate, moddepth, 0, format, rate, mono, &(h->sndmod));
}
/*
void haas_init(michaas *h, int enabled, float millisec, snd_pcm_format_t format, unsigned int rate, int inbuffersize, char *inbuffer, int modenabled, float modrate, float moddepth)
{
	int ret;

	h->haasenabled = enabled;
	h->millisec = millisec;
	h->format = format;
	h->rate = rate;

	h->haasmutex = malloc(sizeof(pthread_mutex_t));
	if((ret=pthread_mutex_init(h->haasmutex, NULL))!=0 )
		printf("mutex init failed, %d\n", ret);

	h->inbuffersize = inbuffersize;
	h->inbufferframes = h->inbuffersize / snd_pcm_format_width(format) * 8;
	int delayframes = millisec / 1000.0 * rate;
	h->delaybytes = delayframes * ( snd_pcm_format_width(format) / 8 );
	h->delayed = malloc((h->inbufferframes+delayframes) * ( snd_pcm_format_width(format) / 8 ));
	memset(h->delayed+inbuffersize, 0, h->delaybytes);
	h->delayedbuffer = (signed short *)h->delayed;
	//printf("%f, %d, %d\n", millisec, delayframes, delaybytes);

	h->sbuffersize = h->inbufferframes * stereo * ( snd_pcm_format_width(h->format) / 8 );
	h->sbuffer = malloc(h->sbuffersize);
	h->stereobuffer = (signed short *)h->sbuffer;

	h->mbuffer = inbuffer;
	h->monobuffer = (signed short *)h->mbuffer;

	h->sndmod.enabled = modenabled;
	soundmod_init(modrate, moddepth, format, rate, mono, &(h->sndmod));
}
*/

void haas_add(michaas *h)
{
	int i, j;

	pthread_mutex_lock(h->haasmutex);
	if (h->haasenabled)
	{
		memcpy(h->delayed, h->delayed+h->inbuffersize, h->delaybytes); // Delay R
		memcpy(h->delayed+h->delaybytes, (char *)h->monobuffer, h->inbuffersize);

		soundmod_add(h->delayed, h->inbuffersize, &(h->sndmod)); // Modulate R

		for(i=j=0;i<h->inbufferframes;i++)
		{
			// Haas
			h->stereobuffer[j++] = h->monobuffer[i] * 0.7; // L
			h->stereobuffer[j++] = h->delayedbuffer[i];
		}
	}
	else
	{
		for(i=j=0;i<h->inbufferframes;i++)
		{
			// Dry, mono -> mono on both sides of stereo
			h->stereobuffer[j++] = h->monobuffer[i]; // L
			h->stereobuffer[j++] = h->monobuffer[i]; // R
		}
	}
	pthread_mutex_unlock(h->haasmutex);
}

void haas_close(michaas *h)
{
	//soundmod_close(&sndmod);
	free(h->delayed);
	h->delayed = NULL;
	h->delaybytes = 0;
	h->delayedbuffer = NULL;
	free(h->sbuffer);
	h->sbuffer = NULL;
	h->stereobuffer = NULL;
	h->monobuffer = NULL;
	pthread_mutex_destroy(h->haasmutex);
	free(h->haasmutex);
	soundmod_close(&(h->sndmod));
}

void init_mic(microphone *m, char* device, snd_pcm_format_t format, unsigned int rate, int outframes, int haasenabled, float haasms, int modenabled, float modrate, float moddepth)
{
	strcpy(m->device, device);

	m->format = format; // SND_PCM_FORMAT_S16_LE;
	m->rate = rate;
	m->channels = mono;
	m->status = MC_RUNNING;
	m->bufferframes = outframes;
	m->buffersamples = m->bufferframes * mono;
	m->buffersize = m->buffersamples * ( snd_pcm_format_width(m->format) / 8 );
	m->buffer = malloc(m->buffersize);
	memset(m->buffer, 0, m->buffersize);

	haas_init(&(m->mh), haasenabled, haasms, format, rate, m->buffersize, m->buffer, modenabled, modrate, moddepth);
}

int read_mic(microphone *m)
{
	int err;
	if ((err = snd_pcm_readi(m->capture_handle, m->buffer, m->bufferframes)) != m->bufferframes)
	{
		printf("snd_pcm_readi error: %s\n", snd_strerror(err));
		return(0);
	}
	return(1);
}

void signalstop_mic(microphone *m)
{
	m->status = MC_STOPPED;
}

void close_mic(microphone *m)
{
	free(m->buffer);
	haas_close(&(m->mh));
}
