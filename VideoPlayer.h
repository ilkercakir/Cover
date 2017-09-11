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
	int codedWidth, codedHeight, codecWidth, codecHeight, lineWidth;
	char *now_playing;
	char *catalog_folder;

	pthread_mutex_t seekmutex;
	pthread_cond_t pausecond;
	pthread_mutex_t framemutex;
	GtkWidget *drawingarea;

	pthread_t tid[4];
	int retval_thread0, retval_thread1, retval_thread2, retval_thread3;
	cpu_set_t cpu[4];

	videoplayerqueue vpq;

	int vqMaxLength, aqMaxLength;
	audiomixer *x;
	audioequalizer *aeq;
	int aqLength, vqLength;

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
	YUVformats yuvfmt;

	int frametime, decodevideo, spk_samplingrate;
	double frame_rate, sample_rate;
	int64_t now_playing_frame, now_decoding_frame, videoduration, audioduration, audioframe;
	long diff1, diff2, diff3, diff4, diff7, framesskipped;
	void *vpwp; int hscaleupd;
	oglidle oi;
}videoplayer;

void init_videoplayer(videoplayer *v, int width, int height, int vqMaxLength, int aqMaxLength, audiomixer *x, int thread_count);
void close_videoplayer(videoplayer *v);
void requeststop_videoplayer(videoplayer *v);
void signalstop_videoplayer(videoplayer *v);
void drain_videoplayer(videoplayer *v);
int open_now_playing(videoplayer *v);
void request_stop_frame_reader(videoplayer *v);
gpointer thread0_videoplayer(void* args);
gboolean update_hscale(gpointer data); // defined inside VideoPlayerWidgets.c
gboolean set_upper(gpointer data); // defined inside VideoPlayerWidgets.c
/*
gboolean setLevel1(gpointer data); // defined inside VideoPlayerWidgets.c
gboolean setLevel2(gpointer data); // defined inside VideoPlayerWidgets.c
gboolean setLevel3(gpointer data); // defined inside VideoPlayerWidgets.c
gboolean setLevel4(gpointer data); // defined inside VideoPlayerWidgets.c
gboolean setLevel7(gpointer data); // defined inside VideoPlayerWidgets.c
gboolean setLevel8(gpointer data); // defined inside VideoPlayerWidgets.c
gboolean setLevel9(gpointer data); // defined inside VideoPlayerWidgets.c
void resetLevels(videoplayer *v); // defined inside VideoPlayerWidgets.c
*/
#endif
