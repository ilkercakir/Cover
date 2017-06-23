#ifndef BiQuadH
#define BiQuadH

#define _GNU_SOURCE 

#include <stdint.h>
#include <math.h>
#include <pthread.h>
#include <alsa/asoundlib.h>

//#define M_LN2 0.69314718055994530942

/* filter types */
typedef enum
{
   LPF, /* low pass filter */
   HPF, /* High pass filter */
   BPF, /* band pass filter */
   NOTCH, /* Notch Filter */
   PEQ, /* Peaking band EQ filter */
   LSH, /* Low shelf filter */
   HSH /* High shelf filter */
}filtertype;

typedef struct
{
	int type;
	float dbGain;
	float freq, srate, bandwidth;

	float a0, a1, a2, a3, a4;
	float x1, x2, y1, y2;
}biquad;

typedef struct
{
	int bands;
	float *freqs;
	char **labels;
	filtertype *filtertypes;
	float *dbGains;
	snd_pcm_format_t format;
	unsigned int rate;
	unsigned int channels;
	biquad *bq;
	float octave;
	pthread_mutex_t *eqmutex; // = PTHREAD_MUTEX_INITIALIZER;
	int enabled;
	int autoleveling;
	float effgain;
	float volume;
}audioequalizer;

typedef struct
{
	float eqfreqs[10];
	char eqlabels[10][20];
	filtertype filtertypes[10];
	float dbGains[10];
}eqdefaults;

inline float BiQuad(biquad *b, float sample);
void BiQuad_init(biquad *b, filtertype type, float dbGain, float freq, float srate, float bandwidth);
void BiQuad_close(biquad *b);

void AudioEqualizer_calcEffectiveGain(audioequalizer *eq);
void AudioEqualizer_init(audioequalizer *eq, int eqbands, float eqoctave, int eqenabled, int eqautoleveling, snd_pcm_format_t format, unsigned int rate, unsigned int channels, eqdefaults *eqd);
void AudioEqualizer_setEffectiveGain(audioequalizer *eq, float dbGain);
void AudioEqualizer_setGain(audioequalizer *eq, int eqband, float dbGain);
void AudioEqualizer_setVolume(audioequalizer *eq, float vol);
void AudioEqualizer_setEnabled(audioequalizer *eq, int enabled);
void AudioEqualizer_setAutoLeveling(audioequalizer *eq, int autoleveling);
void AudioEqualizer_close(audioequalizer *eq);
void AudioEqualizer_BiQuadProcess(audioequalizer *eq, uint8_t *buf, int bufsize);
void set_eqdefaults(eqdefaults *d);
void saveto_eqdefaults(eqdefaults *d, audioequalizer *eq);
#endif
