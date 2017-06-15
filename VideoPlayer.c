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

#include "VideoPlayer.h"

typedef struct
{
	long long usecs; // microseconds
}videoplayertimer;

long get_first_usec(videoplayertimer *t)
{
	long long micros;
    struct timespec spec;

    clock_gettime(CLOCK_REALTIME, &spec);

	micros = spec.tv_sec * 1.0e6 + round(spec.tv_nsec / 1000); // Convert nanoseconds to microseconds
	t->usecs = micros;
	return(0L);
}

long get_next_usec(videoplayertimer *t)
{
    long delta;
    long long micros;
    struct timespec spec;

    clock_gettime(CLOCK_REALTIME, &spec);

    micros = spec.tv_sec * 1.0e6 + round(spec.tv_nsec / 1000); // Convert nanoseconds to microseconds
    delta = micros - t->usecs;
    t->usecs = micros;
    return(delta);
}

gboolean invalidate(gpointer data)
{
	videoplayer *v = (videoplayer*)data;

	GdkWindow *dawin = gtk_widget_get_window(v->dwgarea);
	cairo_region_t *region = gdk_window_get_clip_region(dawin);
	gdk_window_invalidate_region (dawin, region, TRUE);
	//gdk_window_process_updates (dawin, TRUE);
	cairo_region_destroy(region);
	return FALSE;
}

void destroynotify(guchar *pixels, gpointer data)
{
//printf("destroy notify\n");
	free(pixels);
}

void initPixbuf(videoplayer *v)
{
	int i;

	guchar *imgdata = malloc(v->playerWidth*v->playerHeight*4); // RGBA
	for(i=0;i<v->playerWidth*v->playerHeight;i++)
	{
		((unsigned int *)imgdata)[i]=0xFF000000; // ABGR
	}
	g_mutex_lock(&(v->pixbufmutex));
	if (v->pixbuf)
		g_object_unref(v->pixbuf);
    v->pixbuf = gdk_pixbuf_new_from_data(imgdata, GDK_COLORSPACE_RGB, TRUE, 8, v->playerWidth, v->playerHeight, v->playerWidth*4, destroynotify, NULL);
	g_mutex_unlock(&(v->pixbufmutex));
	gdk_threads_add_idle(invalidate, v);
}

void closePixbuf(videoplayer *v)
{
	g_mutex_lock(&(v->pixbufmutex));
	g_object_unref(v->pixbuf);
	v->pixbuf=NULL;
	g_mutex_unlock(&(v->pixbufmutex));
}

void init_videoplayer(videoplayer *v, int width, int height, int vqMaxLength, int aqMaxLength, audiomixer *x)
{
	int ret;

	v->playerWidth = width;
	v->playerHeight = height;
	if (!v->playerHeight)
		v->playerHeight = v->playerWidth * 9 / 16;

	if((ret=pthread_mutex_init(&(v->seekmutex), NULL))!=0 )
		printf("seek mutex init failed, %d\n", ret);

	if((ret=pthread_mutex_init(&(v->framemutex), NULL))!=0 )
		printf("frame mutex init failed, %d\n", ret);

	v->catalog_folder = NULL;

	for(ret=0;ret<4;ret++)
	{
		CPU_ZERO(&(v->cpu[ret]));
		CPU_SET(ret, &(v->cpu[ret]));
	}

	v->vqMaxLength = vqMaxLength;
	v->aqMaxLength = aqMaxLength;

	vq_init(&(v->vpq), vqMaxLength);
	v->stoprequested = 0;

	v->x = x;

	v->pFormatCtx = NULL;
	v->pCodecCtx = NULL;
	v->pCodecCtxA = NULL;
	v->sws_ctx = NULL;
	v->pCodec = NULL;
	v->pCodecA = NULL;
	v->thread_count = 4;
	v->optionsDict = NULL;
	v->optionsDictA = NULL;
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

int open_now_playing(videoplayer *v)
{
    int i;

    /* FFMpeg stuff */
    v->optionsDict = NULL;
    v->optionsDictA = NULL;

    av_register_all();
    avformat_network_init();

	if(avformat_open_input(&(v->pFormatCtx), v->now_playing, NULL, NULL)!=0)
	{
		printf("avformat_open_input()\n");
		return -1; // Couldn't open file
	}

    // Retrieve stream information
	if(avformat_find_stream_info(v->pFormatCtx, NULL)<0)
	{
		printf("avformat_find_stream_info()\n");
		return -1; // Couldn't find stream information
	}
  
    // Dump information about file onto standard error
	//av_dump_format(v->pFormatCtx, 0, v->now_playing, 0);
  
    // Find the first video stream
    v->videoStream=-1;
	for(i=0; i<v->pFormatCtx->nb_streams; i++)
	{
		if (v->pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO)
		{
			v->videoStream=i;
			break;
		}
	}
	if (v->videoStream==-1)
	{
//printf("No video stream found\n");
		v->videoduration = 0;
//		return -1; // Didn't find a video stream
	}
	else
	{
		// Get a pointer to the codec context for the video stream
		v->pCodecCtx=v->pFormatCtx->streams[v->videoStream]->codec;

		// Find the decoder for the video stream
		v->pCodec=avcodec_find_decoder(v->pCodecCtx->codec_id);
		if (v->pCodec==NULL)
		{
			printf("Unsupported video codec\n");
			return -1; // Codec not found
		}

		v->pCodecCtx->thread_count = v->thread_count;
		// Open codec
		if (avcodec_open2(v->pCodecCtx, v->pCodec, &(v->optionsDict))<0)
		{
			printf("Video avcodec_open2()\n");
			return -1; // Could not open video codec
		}

		AVStream *st = v->pFormatCtx->streams[v->videoStream];
		if (st->avg_frame_rate.den)
		{
			v->frame_rate = st->avg_frame_rate.num / (double)st->avg_frame_rate.den;
			v->frametime = 1000000 / v->frame_rate; // usec
		}
		else
		{
			v->frame_rate = 0;
			v->frametime = 1000000; // 1s
		}
//printf("Frame rate = %2.2f\n", frame_rate);
//printf("frametime = %d usec\n", frametime);
//printf("Width : %d, Height : %d\n", pCodecCtx->width, pCodecCtx->height);
		v->videoduration = (v->pFormatCtx->duration / AV_TIME_BASE) * v->frame_rate; // in frames
	}

	v->audioStream = -1;
	for(i=0; i<v->pFormatCtx->nb_streams; i++)
	{
		if (v->pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_AUDIO)
		{
			v->audioStream = i;
			break;
		}
	}
	if (v->audioStream==-1)
	{
//printf("No audio stream found\n");
//		return -1; // Didn't find an audio stream
	}
	else
	{
		// Get a pointer to the codec context for the audio stream
		v->pCodecCtxA=v->pFormatCtx->streams[v->audioStream]->codec;

		// Find the decoder for the audio stream
		v->pCodecA=avcodec_find_decoder(v->pCodecCtxA->codec_id);

		if (v->pCodecA==NULL)
		{
			printf("Unsupported audio codec!\n");
			return -1; // Codec not found
		}

		// Open codec
		if (avcodec_open2(v->pCodecCtxA, v->pCodecA, &(v->optionsDictA))<0)
		{
			printf("Audio avcodec_open2()\n");
			return -1; // Could not open audio codec
		}

		// Set up SWR context once you've got codec information
		v->swr = swr_alloc();
		av_opt_set_int(v->swr, "in_channel_layout",  v->pCodecCtxA->channel_layout, 0);
		av_opt_set_int(v->swr, "out_channel_layout", v->pCodecCtxA->channel_layout,  0);
		av_opt_set_int(v->swr, "in_sample_rate",     v->pCodecCtxA->sample_rate, 0);
		av_opt_set_int(v->swr, "out_sample_rate",    v->pCodecCtxA->sample_rate, 0);
		av_opt_set_sample_fmt(v->swr, "in_sample_fmt",  v->pCodecCtxA->sample_fmt, 0);

		av_opt_set_sample_fmt(v->swr, "out_sample_fmt", AV_SAMPLE_FMT_S16,  0);
		swr_init(v->swr);

		v->sample_rate = v->pCodecCtxA->sample_rate;
		v->audioduration = (v->pFormatCtx->duration / AV_TIME_BASE) * v->sample_rate; // in samples
		//printf("audio frame count %lld\n", audioduration);
/*
		int64_t last_ts = pFormatCtx->duration / AV_TIME_BASE;
		last_ts = (last_ts * (pFormatCtx->streams[audioStream]->time_base.den)) / (pFormatCtx->streams[audioStream]->time_base.num);
		printf("timebase last_ts %lld\n", last_ts);
*/
	}

	if (v->optionsDict)
		av_dict_free(&(v->optionsDict));
	if (v->optionsDictA)
		av_dict_free(&(v->optionsDictA));

	return 0;
}

void request_stop_frame_reader(videoplayer *v)
{
	v->stoprequested = 1;
}

void frame_reader_loop(videoplayer *v, audiojack *aj)
{
	videoplayertimer vt;

    AVFrame *pFrame = NULL;
    pFrame=av_frame_alloc();

    AVPacket *packet;

    packet=av_malloc(sizeof(AVPacket));
    av_init_packet(packet);
    packet->data = NULL;
    packet->size = 0;

	v->now_decoding_frame = v->now_playing_frame = 0;
    int64_t fnum, aperiod=0;

	char *rgba;
	int width = v->pCodecCtx->width;
	int height = v->pCodecCtx->height;

	int frameFinished, ret;

	uint8_t **dst_data;
	int dst_bufsize;
	int line_size;

	get_first_usec(&vt);
    while ((av_read_frame(v->pFormatCtx, packet)>=0) && (!v->stoprequested))
    {
//printf("av_read_frame");
		v->diff1=get_next_usec(&vt); //printf("%lu usec av_read_frame\n", v->diff1);

		//if (!(now_playing_frame%10))
		//	gdk_threads_add_idle(setLevel1, &diff1);

		if (packet->stream_index==v->videoStream) 
		{
//printf("videoStream %lld\n", v->now_decoding_frame);
			get_first_usec(&vt);
			if ((ret=avcodec_decode_video2(v->pCodecCtx, pFrame, &frameFinished, packet))<0) // Decode video frame
				printf("Error decoding video frame %d\n", ret);
			v->diff2=get_next_usec(&vt); //printf("%lu usec avcodec_decode_video2\n", v->diff2);

			//if (!(now_playing_frame%10))
			//	gdk_threads_add_idle(setLevel2, &diff2);

			if (frameFinished)
			{
				if (!v->videoduration) // no video stream
				{
					AVFrame *pFrameRGB = NULL;
					pFrameRGB = av_frame_alloc();
					av_frame_unref(pFrameRGB);

					uint8_t *rgbbuffer = NULL;
					int numBytes = avpicture_get_size(AV_PIX_FMT_RGB32, v->playerWidth, v->playerHeight);
					//printf("numbytes %d\n", numBytes);

					rgbbuffer = (uint8_t *)av_malloc(numBytes * sizeof(uint8_t));
					avpicture_fill((AVPicture *)pFrameRGB, rgbbuffer, AV_PIX_FMT_RGB32, v->playerWidth, v->playerHeight);

					struct SwsContext *sws_ctx = sws_getContext(v->pCodecCtx->width, v->pCodecCtx->height, v->pCodecCtx->pix_fmt,
					v->playerWidth, v->playerHeight, AV_PIX_FMT_RGB32, SWS_BILINEAR, NULL, NULL, NULL);

					get_first_usec(&vt);
					//printf("sws_scale %d %d\n", playerWidth, playerHeight);
					sws_scale(sws_ctx, (uint8_t const* const*)pFrame->data, pFrame->linesize, 0, v->pCodecCtx->height, pFrameRGB->data, pFrameRGB->linesize);
					//printf("sws_scale done\n");
					v->diff4=v->diff2=get_next_usec(&vt);

					//gdk_threads_add_idle(setLevel4, &diff4);

					sws_freeContext(sws_ctx);

					rgba = malloc(pFrameRGB->linesize[0] * v->playerHeight);
					//printf("malloc %d %d\n", pFrameRGB->linesize[0], playerHeight);
					memcpy(rgba, pFrameRGB->data[0], pFrameRGB->linesize[0] * v->playerHeight);
					//printf("memcpy %d\n", pFrameRGB->linesize[0] * playerHeight);

					av_free(rgbbuffer);
					//printf("av_free\n");
					av_frame_unref(pFrameRGB);
					//printf("av_frame_unref\n");
					av_frame_free(&pFrameRGB);
				}
				else
				{
					rgba = malloc(width * height*3/2);
					memcpy(&rgba[0], pFrame->data[0], width*height); //y
					memcpy(&rgba[width*height], pFrame->data[1], width*height/4); //u
					memcpy(&rgba[width*height*5/4], pFrame->data[2], width*height/4); //v
				}

				pthread_mutex_lock(&(v->framemutex));
				if (!v->videoduration)
					fnum = v->now_decoding_frame = 0;
				else
					fnum = v->now_decoding_frame++;
				pthread_mutex_unlock(&(v->framemutex));

				vq_add(&(v->vpq), rgba, fnum); //printf("vq added %lld\n", fnum);

				pthread_mutex_lock(&(v->seekmutex));
				pthread_mutex_unlock(&(v->seekmutex));
				
				av_frame_unref(pFrame);
			}
		}
		else if (packet->stream_index==v->audioStream)
		{
//printf("audioStream %d\n", i);
			if ((ret = avcodec_decode_audio4(v->pCodecCtxA, pFrame, &frameFinished, packet)) < 0)
				printf("Error decoding audio frame %d\n", ret);
			if (frameFinished)
			{
				ret = av_samples_alloc_array_and_samples(&dst_data, &line_size, v->pCodecCtxA->channels, pFrame->nb_samples, AV_SAMPLE_FMT_S16, 0);
//printf("av_samples_alloc_array_and_samples, %d\n", ret);
				ret = swr_convert(v->swr, dst_data, pFrame->nb_samples, (const uint8_t **)pFrame->extended_data, pFrame->nb_samples);
				if (ret<0) printf("swr_convert error %d\n", ret);
				dst_bufsize = av_samples_get_buffer_size(NULL, v->pCodecCtxA->channels, ret, AV_SAMPLE_FMT_S16, 0);
//printf("bufsize=%d\n", p->dst_bufsize);
				if (dst_data)
				{
					if (dst_data[0])
					{
						if (dst_bufsize)
						{
							AudioEqualizer_BiQuadProcess(v->aeq, dst_data[0], dst_bufsize);
							writetojack((char*)dst_data[0], dst_bufsize, aj);
							aperiod++;
						}
						av_freep(&(dst_data[0]));
					}
					av_freep(&dst_data);
				}
				pthread_mutex_lock(&(v->seekmutex));
				pthread_mutex_unlock(&(v->seekmutex));

				av_frame_unref(pFrame);
			}
		}

		av_packet_unref(packet);
		av_free(packet);
		packet=av_malloc(sizeof(AVPacket));
		av_init_packet(packet);
		packet->data = NULL;
		packet->size = 0;

		get_first_usec(&vt);
	}
	av_frame_free(&pFrame);
//printf("av_frame_free\n");
	av_packet_unref(packet);
//printf("av_packet_unref\n");
	av_free(packet);
//printf("av_free packet\n");
	if (v->videoStream!=-1)
	{
		avcodec_flush_buffers(v->pCodecCtx);
		avcodec_close(v->pFormatCtx->streams[v->videoStream]->codec);
		//av_free(pCodecCtx);
	}
	if (v->audioStream!=-1)
	{
		avcodec_flush_buffers(v->pCodecCtxA);
		avcodec_close(v->pFormatCtx->streams[v->audioStream]->codec);
		//av_free(pCodecCtxA);
	}
	avformat_close_input(&(v->pFormatCtx));
	avformat_network_deinit();

	free(v->swr);
}

void frame_player_loop(videoplayer *v)
{
	CUBE_STATE_T state;
	UserData *userData;
	videoqueue *q;
	long diff = 0;
	videoplayertimer vt;

//printf("starting frame player\n");
    bcm_host_init();

    //init_ogl(p_state); // Render to screen
    init_ogl2(&state, v->playerWidth, v->playerHeight); // Render to Frame Buffer
    if (Init(&state) == GL_FALSE)
    {
		printf("Init failed\n");
		return;
	}

	int width = v->pCodecCtx->width;
	int height = v->pCodecCtx->height;
	userData = state.user_data;
    setSize(&state, width/4, height*3/2);

	initPixbuf(v);
	userData->outrgb = (char*)gdk_pixbuf_get_pixels(v->pixbuf);

	GLfloat picSize[2] = { (GLfloat)width, (GLfloat)height*3/2 };
	glUniform2fv(userData->sizeLoc, 1, picSize);
	GLfloat yuv2rgbmatrix[9] = { 1.0, 0.0, 1.5958, 1.0, -0.3917, -0.8129, 1.0, 2.017, 0.0 };
	glUniformMatrix3fv(userData->cmatrixLoc, 1, FALSE, yuv2rgbmatrix);

//printf("player start\n");
	while ((q=vq_remove(&(v->vpq))))
	{
//printf("vq_remove %lld\n", q->label);

		if (v->videoduration)
			v->now_playing_frame = q->label;

		//if (!(q->label%10))
		//	gdk_threads_add_idle(setLevel9, &(v->vpq.vqLength));

		if (diff > 0)
		{
			diff-=v->frametime;
			if (diff<0)
			{
				//printf("usleep %ld\n", 0-diff);
				usleep(0-diff);
				diff = 0;
			}
//printf("skip %lld\n", p->label);
			v->framesskipped++;
			v->diff7 = (float)(v->framesskipped*100)/(float)v->now_playing_frame;
			//gdk_threads_add_idle(setLevel7, &v->diff7);

			free(q->yuv);
			free(q);
			continue;
		}

		if (!v->videoduration) // No video stream || single jpeg album art with mp3
		{
			g_mutex_lock(&(v->pixbufmutex));
			//printf("pixbuf memcpy\n");
			memcpy(userData->outrgb, q->yuv, state.screen_width*state.screen_height*4);
			//printf("pixbuf memcpy %d %d\n", p_state->screen_width, p_state->screen_height);
			g_mutex_unlock(&(v->pixbufmutex));
			v->diff3 = v->diff4 = v->diff5 = 0;
			//gdk_threads_add_idle(invalidate, NULL);
		}
		else
		{
			get_first_usec(&vt);
			texImage2D(&state, q->yuv, width/4, height*3/2);
			v->diff3=get_next_usec(&vt);
//printf("%lu usec glTexImage2D\n", diff3);

			get_first_usec(&vt);
			redraw_scene(&state);
			v->diff4=get_next_usec(&vt);
//printf("%lu usec redraw\n", diff4);

			get_first_usec(&vt);
			g_mutex_lock(&(v->pixbufmutex));
			glReadPixels(0, 0, state.screen_width, state.screen_height, GL_RGBA, GL_UNSIGNED_BYTE, userData->outrgb);
			g_mutex_unlock(&(v->pixbufmutex));
			v->diff5=get_next_usec(&vt);
//printf("%lu usec glReadPixels\n", diff5);

		//checkNoGLES2Error();

			//if (!(v->now_playing_frame%10))
			//{
			//	gdk_threads_add_idle(setLevel3, &(v->diff3));
			//	gdk_threads_add_idle(setLevel4, &(v->diff4));
			//	gdk_threads_add_idle(setLevel5, &(v->diff5));
			//	gdk_threads_add_idle(update_hscale, NULL);
			//}
		}

		diff = v->diff3 + v->diff4 + v->diff5 + 6000;
//printf("%5lu usec frame, frametime %5d\n", diff, frametime);
		diff -= v->frametime;
		gdk_threads_add_idle(invalidate, (void *)v);
		if (diff<0)
		{
			//printf("usleep %ld\n", 0-diff);
			usleep(0-diff);
			diff = 0;
		}
		//vq_next(&vq);
		free(q->yuv);
		free(q);
	}
	close_ogl2(&state);
//printf("closed ogl\n");
}

static void* read_frames(void* args)
{
	int ctype = PTHREAD_CANCEL_ASYNCHRONOUS;
	int ctype_old;
	pthread_setcanceltype(ctype, &ctype_old);

	videoplayer *vp = (videoplayer *)args;

	audiojack jack2;
	connect_audiojack(vp->aqMaxLength, &jack2, vp->x);

	frame_reader_loop(vp, &jack2);

	close_audiojack(&jack2);

//printf("requeststop_videoplayer\n");
	requeststop_videoplayer(vp);
//printf("drain_videoplayer\n");
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

	frame_player_loop(vp);

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
//printf("read_frames->%d\n", 3);
	if ((err = pthread_setaffinity_np(vp->tid[1], sizeof(cpu_set_t), &(vp->cpu[3]))))
		printf("pthread_setaffinity_np error %d\n", err);

	err = pthread_create(&(vp->tid[2]), NULL, &videoPlayFromQueue, (void*)vp);
	if (err)
	{}
//printf("videoPlayFromQueue->%d\n", 2);
	if ((err = pthread_setaffinity_np(vp->tid[2], sizeof(cpu_set_t), &(vp->cpu[2]))))
		printf("pthread_setaffinity_np error %d\n", err);

	int i;
	if ((i=pthread_join(vp->tid[1], NULL)))
		printf("pthread_join error, vp->tid[1], %d\n", i);

	if ((i=pthread_join(vp->tid[2], NULL)))
		printf("pthread_join error, vp->tid[2], %d\n", i);

	close_videoplayer(vp);

//printf("exiting 0, thread0_videoplayer\n");
	vp->retval_thread0 = 0;
	pthread_exit(&(vp->retval_thread0));
}
