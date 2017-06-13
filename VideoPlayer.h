#ifndef VideoPlayerH
#define VideoPlayerH

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
#include "AudioMixer.h"

typedef enum
{
	ASPECTRATIO_16_9 = 0,
	ASPECTRATIO_4_3 = 1
}aspectratio;

enum
{
  COL_ID = 0,
  COL_FILEPATH,
  NUM_COLS
};

typedef struct
{
	aspectratio ar;
	int playerWidth, playerHeight;
	char *now_playing;
	char *catalog_folder;

	pthread_mutex_t seekmutex; 
	pthread_mutex_t framemutex;
	pthread_t tid[3];
	int retval_thread0, retval_thread1, retval_thread2;
	cpu_set_t cpu[4];

	videoplayerqueue vpq;

	int vqMaxLength, aqMaxLength;
	audiomixer *x;

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

	int frametime;
	double frame_rate, sample_rate;
	int64_t now_playing_frame, now_decoding_frame, videoduration, audioduration;
}videoplayer;

void init_videoplayer(videoplayer *v, int width, aspectratio ar, int vqMaxLength, int aqMaxLength, audiomixer *x);
void close_videoplayer(videoplayer *v);
void requeststop_videoplayer(videoplayer *v);
void signalstop_videoplayer(videoplayer *v);
void drain_videoplayer(videoplayer *v);
void* thread0_videoplayer(void* args);
#endif
