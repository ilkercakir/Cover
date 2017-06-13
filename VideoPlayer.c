/*
 * VideoPlayer.c
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

#define _GNU_SOURCE

#include <stdio.h>
#include <pthread.h>
#include "VideoPlayer.h"

void init_videoplayer(videoplayer *v, int width, aspectratio ar, int vqMaxLength, int aqMaxLength, audiomixer *x)
{
	int ret;

	v->playerWidth = width;
	v->ar = ar;
	switch (ar)
	{
		case ASPECTRATIO_16_9:
			v->playerHeight = v->playerWidth * 9 / 16;
			break;
		case ASPECTRATIO_4_3:
			v->playerHeight = v->playerWidth * 3 / 4;
			break;
	}

	if((ret=pthread_mutex_init(&(v->seekmutex), NULL))!=0 )
		printf("seek mutex init failed, %d\n", ret);

	if((ret=pthread_mutex_init(&(v->framemutex), NULL))!=0 )
		printf("frame mutex init failed, %d\n", ret);

	v->now_playing = NULL;
	v->catalog_folder = NULL;

	for(ret=0;ret<4;ret++)
	{
		CPU_ZERO(&(v->cpu[ret]));
		CPU_SET(ret, &(v->cpu[ret]));
	}

	v->vqMaxLength = vqMaxLength;
	v->aqMaxLength = aqMaxLength;

	vq_init(&(v->vpq), vqMaxLength);

	v->x = x;

	v->pFormatCtx = NULL;
	v->pCodecCtx = NULL;
	v->pCodecCtxA = NULL;
	v->sws_ctx = NULL;
	v->pCodec = NULL;
	v->pCodecA = NULL;
	v->thread_count = 4;
}

void close_videoplayer(videoplayer *v)
{
	vq_destroy(&(v->vpq));
	pthread_mutex_destroy(&(v->framemutex));
	pthread_mutex_destroy(&(v->seekmutex));
}

void requeststop_videoplayer(videoplayer *v)
{
	vq_requeststop(&(v->vpq));
}

void signalstop_videoplayer(videoplayer *v)
{
	vq_signalstop(&(v->vpq));
}

void drain_videoplayer(videoplayer *v)
{
	vq_drain(&(v->vpq));
}

static void* read_frames(void* args)
{
	int ctype = PTHREAD_CANCEL_ASYNCHRONOUS;
	int ctype_old;
	pthread_setcanceltype(ctype, &ctype_old);

	videoplayer *vp = (videoplayer *)args;

	audiojack jack2;
	connect_audiojack(vp->aqMaxLength, &jack2, vp->x);

	sleep(3);

	close_audiojack(&jack2);

	requeststop_videoplayer(vp);
	drain_videoplayer(vp);

//printf("exiting 1, read_frames\n");
	vp->retval_thread1 = 0;
	pthread_exit(&(vp->retval_thread1));
}

static void* videoPlayFromQueue(void* args)
{
	int ctype = PTHREAD_CANCEL_ASYNCHRONOUS;
	int ctype_old;
	pthread_setcanceltype(ctype, &ctype_old);

	videoplayer *vp = (videoplayer *)args;

	sleep(4);

	signalstop_videoplayer(vp);

//printf("exiting 2, videoPlayFromQueue\n");
	vp->retval_thread2 = 0;
	pthread_exit(&(vp->retval_thread2));
}

void* thread0_videoplayer(void* args)
{
	int ctype = PTHREAD_CANCEL_ASYNCHRONOUS;
	int ctype_old;
	pthread_setcanceltype(ctype, &ctype_old);

	videoplayer *vp = (videoplayer *)args;

	int err;
	err = pthread_create(&(vp->tid[1]), NULL, &read_frames, (void*)vp);
	if (err)
	{}
	err = pthread_setaffinity_np(vp->tid[1], sizeof(cpu_set_t), &(vp->cpu[3]));
	if (err)
	{}

	err = pthread_create(&(vp->tid[2]), NULL, &videoPlayFromQueue, (void*)vp);
	if (err)
	{}
	err = pthread_setaffinity_np(vp->tid[2], sizeof(cpu_set_t), &(vp->cpu[2]));
	if (err)
	{}

	int i;
	if ((i=pthread_join(vp->tid[1], NULL)))
		printf("pthread_join error, tid[1], %d\n", i);

	if ((i=pthread_join(vp->tid[2], NULL)))
		printf("pthread_join error, tid[2], %d\n", i);

//printf("exiting 0, thread0_videoplayer\n");
	vp->retval_thread0 = 0;
	pthread_exit(&(vp->retval_thread0));
}

/*
int create_thread0_videoplayer(videoplayer *v)
{
	int err;

	err = pthread_create(&(v->tid[0]), NULL, &thread0_videoplayer, (void *)v);
	if (err)
	{}
	err = pthread_setaffinity_np(v->tid[0], sizeof(cpu_set_t), &(v->cpu[0]));
	if (err)
	{}

	return(0);
}

void terminate_thread0_videoplayer(videoplayer *v)
{
	int i;

	close_videoplayer(v);
	if ((i=pthread_join(v->tid[0], NULL)))
		printf("pthread_join error, tid[0], %d\n", i);
}
*/
