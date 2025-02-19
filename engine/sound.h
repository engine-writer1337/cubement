#ifndef _SOUND_H_
#define _SOUND_H_

#include <dsound.h>

#define SND_CHANNELS	2
#define SND_BITS		16
#define SND_SPEED		44100
#define SND_BUFFER_SIZE	0x10000
#define	SND_FULLVOLUME	80
#define SND_MIX_AHEAD	0.125f
#define	SND_PAINT_SIZE	8192
#define SND_FOLDER		"sound/"

#define CHAN_TALK	0
#define CHAN_STEP	1
#define CHAN_PAIN	2
#define CHAN_ENTITY	3

#define	SND_MAX_STATIC		1
#define	SND_MAX_DYNAMICS	24
#define SND_MAX_CHANNELS	256
#define	SND_MAX_AMBIENTS	(SND_MAX_CHANNELS - SND_MAX_DYNAMICS - SND_MAX_STATIC)

#define SND_DEF_VOL	0.7f
#define SND_MIN_VOL	0.0f
#define SND_MAX_VOL	1.0f

typedef struct
{
	dword chunk_id;
	dword chunk_size;
	dword format;
	dword subchunk1_id;
	dword subchunk1_size;
	word audio_format;
	word num_channels;
	dword sample_rate;
	dword byte_rate;
	word block_align;
	word bits_persample;
}wav_header_s;

typedef struct
{
	int left;
	int right;
}samplepair_s;

typedef struct _wave_s
{
	int speed;
	int width;
	int channels;
	int samples;
	byte* data;
}wave_s;

typedef struct
{
	const wave_s* wav;
	int leftvol;
	int rightvol;
	int mastervol;

	int entnum;
	int pos, end;
	enum_t channel;

	vec3_t origin;
	float distance;

	bool_t loop;
}channel_s;

typedef struct
{
	int samples;
	short* buffer;

	dword buf_size;
	dword lock_size;
}dma_s;

typedef struct
{
	LPDIRECTSOUND p_ds;
	LPDIRECTSOUNDBUFFER p_dsbuf;
	LPDIRECTSOUNDBUFFER p_dspbuf;

	int rawend;
	int sound_time;
	int painted_time;

	int ivolume;
	convar_s* volume;
}sound_s;

extern sound_s gsnd;

void snd_init();
void snd_shutdown();

void snd_music_stop();
void snd_music(const char* filename);

void snd_stop_all();
void snd_load(const char* name, wave_s* out);
void snd_free(wave_s* wav);

void snd_play(ihandle_t idx, const entity_s* ent, channel_e chan, float volume, float distance);
void snd_play_wav(const wave_s* wav, const vec3_t origin, float fvol, int distance, int entnum, enum_t channel, bool_t loop);

void snd_update();
void snd_set_param();

#endif