#ifndef VideoPlayerH
#define VideoPlayerH

#define _GNU_SOURCE

#include <stdio.h>
#include <pthread.h>

#include <gtk/gtk.h>

#include <libavcodec/avcodec.h>
#include <libavutil/pixfmt.h>
#include <libavutil/avutil.h>
#include <libavutil/samplefmt.h>
#include <libavutil/opt.h>
#include <libavformat/avformat.h>
#include <libavresample/avresample.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>

#include "VideoQueue.h"
#include "YUV420RGBgl.h"
#include "BiQuad.h"
#include "AudioEffects.h"
#include "AudioMixer.h"

enum
{
  COL_ID = 0,
  COL_FILEPATH,
  NUM_COLS
};

typedef struct
{
	int playerWidth, playerHeight, stoprequested;
	char *now_playing;
	char *catalog_folder;

	pthread_mutex_t seekmutex;
	pthread_mutex_t framemutex;
	GtkWidget *dwgarea;
	GdkPixbuf *pixbuf;
	GMutex pixbufmutex;

	pthread_t tid[4];
	int retval_thread0, retval_thread1, retval_thread2, retval_thread3;
	cpu_set_t cpu[4];

	videoplayerqueue vpq;

	int vqMaxLength, aqMaxLength;
	audiomixer *x;
	audioequalizer *aeq;

	AVFormatContext *pFormatCtx;
	AVCodecContext *pCodecCtx;
	int videoStream;
	AVCodecContext *pCodecCtxA;
	int audioStream;
	struct SwsContext *sws_ctx;
	AVCodec *pCodec;
	AVCodec *pCodecA;
	int thread_count;
	SwrContext *swr;
    AVDictionary *optionsDict;
    AVDictionary *optionsDictA;

	int frametime;
	double frame_rate, sample_rate;
	int64_t now_playing_frame, now_decoding_frame, videoduration, audioduration;
	long diff1, diff2, diff3, diff4, diff5, diff7, framesskipped;
}videoplayer;

void initPixbuf(videoplayer *v);
void closePixbuf(videoplayer *v);
void init_videoplayer(videoplayer *v, int width, int height, int vqMaxLength, int aqMaxLength, audiomixer *x);
void close_videoplayer(videoplayer *v);
void requeststop_videoplayer(videoplayer *v);
void signalstop_videoplayer(videoplayer *v);
void drain_videoplayer(videoplayer *v);
int open_now_playing(videoplayer *v);
void request_stop_frame_reader(videoplayer *v);
void* thread0_videoplayer(void* args);
#endif
