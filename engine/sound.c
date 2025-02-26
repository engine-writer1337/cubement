#include "cubement.h"

#define STB_VORBIS_NO_PUSHDATA_API
#include "libs/stb_vorbis.h"

sound_s gsnd;
static dma_s gdma;
static int gscaletable[32][256];
static channel_s gchannels[SND_MAX_CHANNELS];
static samplepair_s grawsamples[SND_PAINT_SIZE];

static struct
{
	int pos;
	bool_t loop;
	bool_t pause;

	FILE* fp;
	stb_vorbis* stream;
}gogg;

static void snd_error(int error)
{
	switch (error)
	{
	default:					con_printf(COLOR_RED, "Sound unknown error %i", error);		break;
	case DSERR_BUFFERLOST:		con_print(COLOR_RED, "Sound error: DSERR_BUFFERLOST");		break;
	case DSERR_INVALIDCALL:		con_print(COLOR_RED, "Sound error: DSERR_INVALIDCALL");		break;
	case DSERR_INVALIDPARAM:	con_print(COLOR_RED, "Sound error: DSERR_INVALIDPARAM");	break;
	case DSERR_PRIOLEVELNEEDED:	con_print(COLOR_RED, "Sound error: DSERR_PRIOLEVELNEEDED");	break;
	}
}

static void snd_begin_painting()
{
	int reps;
	HRESULT	hresult;
	LPVOID pbuf, pbuf2;
	dword size2, status;

	status = 0;
	if (IDirectSoundBuffer_GetStatus(gsnd.p_dsbuf, &status) != DS_OK)
		con_print(COLOR_RED, "Sound error: couldn't get sound buffer status");

	if (status & DSBSTATUS_BUFFERLOST)
		IDirectSoundBuffer_Restore(gsnd.p_dsbuf);

	if (!(status & DSBSTATUS_PLAYING))
		IDirectSoundBuffer_Play(gsnd.p_dsbuf, 0, 0, DSBPLAY_LOOPING);

	reps = 0;
	gdma.buffer = NULL;
	while ((hresult = IDirectSoundBuffer_Lock(gsnd.p_dsbuf, 0, gdma.buf_size, &pbuf, &gdma.lock_size, &pbuf2, &size2, 0)) != DS_OK)
	{
		if (hresult != DSERR_BUFFERLOST)
		{
			snd_error(hresult);
			snd_shutdown();
			return;
		}
		else
			IDirectSoundBuffer_Restore(gsnd.p_dsbuf);

		if (++reps > 4)
			return;
	}

	gdma.buffer = pbuf;
}

static void snd_submit()
{
	if (gdma.buffer)
		IDirectSoundBuffer_Unlock(gsnd.p_dsbuf, gdma.buffer, gdma.lock_size, NULL, 0);
}

static bool_t snd_create_buffers()
{
	DSBCAPS dsbcaps;
	DSBUFFERDESC dsbuf;
	WAVEFORMATEX format;

	if (IDirectSound_SetCooperativeLevel(gsnd.p_ds, gvid.hwnd, DSSCL_EXCLUSIVE) != DS_OK)
	{
		con_print(COLOR_RED, "Sound error: SetCooperativeLevel failed");
		return FALSE;
	}

	memzero(&format, sizeof(format));
	format.wFormatTag = WAVE_FORMAT_PCM;
	format.nChannels = SND_CHANNELS;
	format.wBitsPerSample = SND_BITS;
	format.nSamplesPerSec = SND_SPEED;
	format.nBlockAlign = format.nChannels * (format.wBitsPerSample >> 3);
	format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;

	memzero(&dsbuf, sizeof(dsbuf));
	dsbuf.dwSize = sizeof(DSBUFFERDESC);
	dsbuf.dwFlags = DSBCAPS_PRIMARYBUFFER;

	if (IDirectSound_CreateSoundBuffer(gsnd.p_ds, &dsbuf, &gsnd.p_dspbuf, NULL) == DS_OK)
	{
		if (IDirectSoundBuffer_SetFormat(gsnd.p_dspbuf, &format) != DS_OK)
		{
			con_print(COLOR_RED, "Sound error: setting primary sound format - failed");
			return FALSE;
		}
	}
	else
	{
		con_print(COLOR_RED, "Sound error: creating primary buffer - failed");
		return FALSE;
	}

	memzero(&dsbuf, sizeof(dsbuf));
	dsbuf.dwSize = sizeof(DSBUFFERDESC);
	dsbuf.dwFlags = DSBCAPS_CTRLFREQUENCY | DSBCAPS_LOCSOFTWARE;
	dsbuf.dwBufferBytes = SND_BUFFER_SIZE;
	dsbuf.lpwfxFormat = &format;

	if (IDirectSound_CreateSoundBuffer(gsnd.p_ds, &dsbuf, &gsnd.p_dsbuf, NULL) != DS_OK)
	{
		con_print(COLOR_RED, "Sound error: creating secondary buffer - failed");
		return FALSE;
	}

	if (format.nChannels != SND_CHANNELS || format.wBitsPerSample != SND_BITS || format.nSamplesPerSec != SND_SPEED)
	{
		con_printf(COLOR_RED, "Sound error: %ich, %ibits, %iHz not supported", SND_CHANNELS, SND_BITS, SND_SPEED);
		return FALSE;
	}

	memzero(&dsbcaps, sizeof(dsbcaps));
	dsbcaps.dwSize = sizeof(dsbcaps);
	if (IDirectSoundBuffer_GetCaps(gsnd.p_dsbuf, &dsbcaps) != DS_OK)
	{
		con_print(COLOR_RED, "Sound error: GetCaps failed");
		return FALSE;
	}

	IDirectSoundBuffer_Play(gsnd.p_dsbuf, 0, 0, DSBPLAY_LOOPING);
	gdma.buf_size = dsbcaps.dwBufferBytes;
	gdma.samples = gdma.buf_size / (SND_BITS >> 3);

	snd_begin_painting();
	if (!gdma.buffer)
		return FALSE;

	memzero(gdma.buffer, gdma.samples * (SND_BITS >> 3));
	snd_submit();

	con_printf(COLOR_GREEN, "Sound: %ich, %ibits, %iHz", SND_CHANNELS, SND_BITS, SND_SPEED);
	return TRUE;
}

static void snd_destroy_buffers()
{
	if (gsnd.p_ds)
		IDirectSound_SetCooperativeLevel(gsnd.p_ds, gvid.hwnd, DSSCL_NORMAL);

	if (gsnd.p_dsbuf)
	{
		IDirectSoundBuffer_Stop(gsnd.p_dsbuf);
		IDirectSoundBuffer_Release(gsnd.p_dsbuf);
	}

	if (gsnd.p_dspbuf && (gsnd.p_dsbuf != gsnd.p_dspbuf))
		IDirectSoundBuffer_Release(gsnd.p_dspbuf);

	gsnd.p_dsbuf = NULL;
	gsnd.p_dspbuf = NULL;
	gdma.buffer = NULL;
}

void snd_init()
{
	DSCAPS dscaps;

	gsnd.volume->is_change = TRUE;
	if (DirectSoundCreate(NULL, &gsnd.p_ds, NULL) != DS_OK)
	{
		con_print(COLOR_RED, "Sound error: init failed");
		return;
	}

	dscaps.dwSize = sizeof(dscaps);
	if (IDirectSound_GetCaps(gsnd.p_ds, &dscaps) != DS_OK)
		con_print(COLOR_RED, "Sound error: couldn't get DS caps");

	if (dscaps.dwFlags & DSCAPS_EMULDRIVER)
	{
		con_print(COLOR_RED, "Sound error: no DSound driver founds");
		snd_shutdown();
		return;
	}

	if (!snd_create_buffers())
		snd_shutdown();
}

void snd_shutdown()
{
	if (gsnd.p_ds)
	{
		snd_destroy_buffers();
		IDirectSound_Release(gsnd.p_ds);
	}

	gsnd.p_ds = NULL;
	gsnd.p_dsbuf = NULL;
	gsnd.p_dspbuf = NULL;
	snd_music_stop();
}

static int snd_dma_pos()
{
	MMTIME time;
	dword write;

	time.wType = TIME_SAMPLES;
	IDirectSoundBuffer_GetCurrentPosition(gsnd.p_dsbuf, &time.u.sample, &write);
	return (time.u.sample >> ((SND_BITS >> 3) - 1)) & (gdma.samples - 1);
}

void snd_stop_all()
{
	if (!gsnd.p_ds)
		return;

	memzero(gchannels, sizeof(gchannels));
	snd_begin_painting();
	if (!gdma.buffer)
		return;

	memzero(gdma.buffer, gdma.samples * (SND_BITS >> 3));
	snd_submit();
}

static void snd_resample(wave_s* wav, byte* data)
{
	float stepscale;
	int i, outcount;

	stepscale = (float)wav->speed / SND_SPEED;
	outcount = wav->samples / stepscale;
	wav->samples = outcount;
	wav->speed = SND_SPEED;
	wav->data = util_malloc(outcount * wav->width * wav->channels);

	if (stepscale == 1.0f && wav->width == 1)
	{
		if (wav->channels == 2)
		{
			for (i = 0; i < outcount; i++)
			{
				((char*)wav->data)[i * 2 + 0] = (char)(data[i * 2 + 0] - 128);
				((char*)wav->data)[i * 2 + 1] = (char)(data[i * 2 + 1] - 128);
			}
		}
		else
		{
			for (i = 0; i < outcount; i++)
				((char*)wav->data)[i] = (char)(data[i] - 128);
		}
	}
	else
	{
		int srcsample, samplefrac, fracstep;

		samplefrac = 0;
		fracstep = stepscale * 256;

		if (wav->channels == 2)
		{
			if (wav->width == 2)
			{
				for (i = 0; i < outcount; i++)
				{
					srcsample = samplefrac >> 8;
					samplefrac += fracstep;

					((short*)wav->data)[i * 2 + 0] = ((short*)data)[srcsample * 2 + 0];
					((short*)wav->data)[i * 2 + 1] = ((short*)data)[srcsample * 2 + 1];
				}
			}
			else
			{
				for (i = 0; i < outcount; i++)
				{
					srcsample = samplefrac >> 8;
					samplefrac += fracstep;

					((char*)wav->data)[i * 2 + 0] = (char)(data[srcsample * 2 + 0] - 128);
					((char*)wav->data)[i * 2 + 1] = (char)(data[srcsample * 2 + 1] - 128);
				}
			}
		}
		else
		{
			if (wav->width == 2)
			{
				for (i = 0; i < outcount; i++)
				{
					srcsample = samplefrac >> 8;
					samplefrac += fracstep;
					((short*)wav->data)[i] = ((short*)data)[srcsample];
				}
			}
			else
			{
				for (i = 0; i < outcount; i++)
				{
					srcsample = samplefrac >> 8;
					samplefrac += fracstep;
					((char*)wav->data)[i] = (char)(data[srcsample] - 128);
				}
			}
		}
	}
}

static void snd_load_wav(const char* filename, wave_s* out)
{
	int len, size;
	wav_header_s* head;
	byte* src, * data, * endfile, * next;

	data = util_full(filename, &len);
	if (!data)
	{
		con_printf(COLOR_RED, "%s - not found", filename);
		return;
	}

	head = (wav_header_s*)data;
	if (head->chunk_id != 'FFIR' || head->format != 'EVAW' || head->subchunk1_id != ' tmf' || head->audio_format != WAVE_FORMAT_PCM)
	{
		util_free(data);
		con_printf(COLOR_RED, "%s - bad header", filename);
		return;
	}

	if (head->sample_rate != 44100 && head->sample_rate != 22050 && head->sample_rate != 11025 &&
		head->bits_persample != 16 && head->bits_persample != 8 &&
		head->num_channels != 1 && head->num_channels != 2)
	{
		util_free(data);
		con_printf(COLOR_RED, "%s - bad format", filename);
		return;
	}

	src = NULL;
	endfile = data + len;
	for (next = data + sizeof(wav_header_s); next < endfile; next++)
	{
		if (*(dword*)next == 'atad')
		{
			src = next + sizeof(dword);
			size = *(int*)src;
			src += sizeof(dword);
			break;
		}
	}

	if (!src)
	{
		util_free(data);
		con_printf(COLOR_RED, "%s - no data chunk", filename);
		return;
	}

	out->speed = head->sample_rate;
	out->channels = head->num_channels;
	out->width = head->bits_persample >> 3;
	out->samples = size / (out->width * out->channels);
	snd_resample(out, src);
	util_free(data);
}

static void snd_load_ogg(const char* filename, wave_s* out)
{
	byte* data;
	short* temp;
	int len, sz, res;
	stb_vorbis* head;
	
	data = util_full(filename, &len);
	if (!data)
	{
		con_printf(COLOR_RED, "%s - not found", filename);
		return;
	}

	res = 0;
	head = stb_vorbis_open_memory(data, len, &res, NULL);
	if (res)
	{
		util_free(data);
		con_printf(COLOR_RED, "%s - open memory error", filename);
		return;
	}

	if (head->channels != 1 && head->channels != 2 &&
		head->sample_rate != 11025 && head->sample_rate != 22050 && head->sample_rate != 44100)
	{
		stb_vorbis_close(head);
		util_free(data);
		con_printf(COLOR_RED, "%s - bad format", filename);
		return;
	}

	out->width = 2;
	out->speed = head->sample_rate;
	out->channels = head->channels;
	out->samples = stb_vorbis_stream_length_in_samples(head);

	sz = out->samples * out->channels;
	temp = util_malloc(sz * sizeof(short));
	res = stb_vorbis_get_samples_short_interleaved(head, head->channels, temp, sz);
	if (res > 0)
	{
		if (res != out->samples)
		{
			con_printf(COLOR_RED, "%s - invalid read size", filename);
			out->samples = res;
		}
	}
	else
	{
		stb_vorbis_close(head);
		util_free(temp);
		util_free(data);
		con_printf(COLOR_RED, "%s - something wrong", filename);
		return;
	}

	snd_resample(out, (byte*)temp);
	stb_vorbis_close(head);
	util_free(temp);
	util_free(data);
}

void snd_load(const char* filename, wave_s* out)
{
	int len;
	string_t path = SND_FOLDER;

	strcatn(path, filename);
	len = strlen(path);
	if (*(int*)(path + len - 4) == 'ggo.')
	{
		snd_load_ogg(path, out);
		return;
	}
	if (*(int*)(path + len - 4) == 'vaw.')
	{
		snd_load_wav(path, out);
		return;
	}

	strcpy(path + len, ".ogg");
	if (util_exist(path))
	{
		snd_load_ogg(path, out);
		return;
	}

	strcpy(path + len, ".wav");
	if (util_exist(path))
	{
		snd_load_wav(path, out);
		return;
	}

	con_printf(COLOR_RED, "%s - unknown format", filename);
}

void snd_free(wave_s* wav)
{
	if (wav->data)
		util_free(wav->data);
	wav->data = NULL;
}

void snd_music(const char* filename, bool_t loop)
{
	FILE* fp;
	int res = 0;

	fp = util_open(filename, "rb");
	if (!fp)
	{
		con_printf(COLOR_RED, "%s - not found", filename);
		return;
	}

	snd_music_stop();
	gogg.stream = stb_vorbis_open_file(fp, FALSE, &res, NULL);
	if (res != 0)
	{
		util_close(fp);
		con_printf(COLOR_RED, "%s - is not a valid Ogg Vorbis file (error %i)", filename, res);
		return;
	}

	gogg.fp = fp;
	gogg.pos = 0;
	gogg.loop = loop;
	gogg.pause = FALSE;
}

void snd_music_pause(bool_t pause)
{
	gogg.pause = pause;
}

void snd_music_stop()
{
	if (!gogg.stream)
		return;

	stb_vorbis_close(gogg.stream);
	util_close(gogg.fp);
	gogg.stream = NULL;
}

static channel_s* snd_pick_channel(int entnum)
{//TODO:
	channel_s* ch;
	int i, chosen, life_left;

	chosen = SND_MAX_STATIC;
	life_left = 0x7FFFFFFF;

	for (i = SND_MAX_STATIC; i < SND_MAX_STATIC + SND_MAX_DYNAMICS; i++)
	{
		ch = &gchannels[i];
		if (ch->end - gsnd.painted_time < life_left)
		{
			life_left = ch->end - gsnd.painted_time;
			chosen = i;
		}
	}

	ch = &gchannels[chosen];
	memzero(ch, sizeof(*ch));
	return ch;
}

void snd_play(ihandle_t idx, const entity_s* ent, channel_e chan, float volume, float distance)
{
	wave_s* w;
	channel_s* ch;

	if (!gsnd.p_ds || res_notvalid(idx, RES_SOUND))
		return;

	w = &gres[idx].wav;
	if (!w->data)
		return;

	ch = snd_pick_channel(0);
	vec_copy(ch->origin, ent->origin);

	ch->wav = w;
	ch->loop = FALSE;
	ch->mastervol = volume * 255;
	ch->entnum = 1;
	ch->channel = chan;
	ch->distance = 1.f / distance;
	ch->end = gsnd.painted_time + w->samples;
}

void snd_play_wav(const wave_s* wav, const vec3_t origin, float fvol, int distance, int entnum, enum_t channel, bool_t loop)
{
	int vol;
	channel_s* ch;

	if (!gsnd.p_ds || !wav || !wav->data)
		return;

	vol = fvol * 255;
	vol = clamp(0, vol, 255);
	if (!vol)
		return;

	if (channel < SND_MAX_STATIC)
	{
		ch = &gchannels[channel];
		memzero(ch, sizeof(*ch));
		ch->leftvol = vol;
		ch->rightvol = vol;
	}
	else
		ch = snd_pick_channel(entnum);

	if (origin)
		vec_copy(ch->origin, origin);

	ch->wav = wav;
	ch->loop = loop;
	ch->mastervol = vol;
	ch->entnum = entnum;
	ch->channel = channel;
	ch->distance = 1.f / distance;
	ch->end = gsnd.painted_time + wav->samples;
}

static void snd_spatialize_origin(channel_s* ch)
{
	vec3_t source;
	float dot, dist, lscale, rscale, scale;

	vec_sub(source, gworld.vieworg, ch->origin);
	dist = vec_normalize(source) - SND_FULLVOLUME;
	if (dist < 0)
		dist = 0;

	dist *= ch->distance;
	if (dist > 1)
		dist = 1;

	dot = vec_dot(gworld.v_right, source);
	if (ch->distance < CMP_EPSILON)
	{
		rscale = 1;
		lscale = 1;
	}
	else
	{
		rscale = 0.5f * (1.0f + dot);
		lscale = 0.5f * (1.0f - dot);
	}

	scale = (1.0f - dist) * rscale;
	ch->rightvol = scale * ch->mastervol;
	ch->rightvol = clamp(0, ch->rightvol, 255);

	scale = (1.0f - dist) * lscale;
	ch->leftvol = scale * ch->mastervol;
	ch->leftvol = clamp(0, ch->leftvol, 255);
}

static void snd_update_pos()
{
	int i;
	channel_s* ch;

	for (i = SND_MAX_STATIC; i < SND_MAX_CHANNELS; i++)
	{
		ch = &gchannels[i];
		if (!ch->wav)
			continue;

		if (!ch->entnum)
		{
			ch->leftvol = ch->mastervol;
			ch->rightvol = ch->mastervol;
		}
		else
		{
			snd_spatialize_origin(ch);
			if (!ch->leftvol && !ch->rightvol)
				memzero(ch, sizeof(*ch));
		}
	}
}

static void snd_update_soundtime()
{
	int sample_pos, fullsamples;
	static int buffers, old_samplepos;

	sample_pos = snd_dma_pos();
	fullsamples = gdma.samples / SND_CHANNELS;
	if (sample_pos < old_samplepos)
	{
		buffers++;
		if (gsnd.painted_time > 0x6FFFFFFF)
		{
			buffers = 0;
			gsnd.painted_time = fullsamples;
			snd_stop_all();
		}
	}

	old_samplepos = sample_pos;
	gsnd.sound_time = buffers * fullsamples + sample_pos / SND_CHANNELS;
	if (gsnd.painted_time < gsnd.sound_time)
		gsnd.painted_time = gsnd.sound_time;
}

static void snd_transfer_stereo16(int* samp, int end_time)
{
	short* out;
	int i, val, lpos, lpainted_time, linear_count;

	lpainted_time = gsnd.painted_time;
	while (lpainted_time < end_time)
	{
		lpos = lpainted_time & ((gdma.samples >> 1) - 1);
		out = gdma.buffer + (lpos * 2);

		linear_count = (gdma.samples >> 1) - lpos;
		if (lpainted_time + linear_count > end_time)
			linear_count = end_time - lpainted_time;

		linear_count <<= 1;
		for (i = 0; i < linear_count; i += 2)
		{
			val = samp[i + 0] >> 8;
			out[i + 0] = clamp(-0x7FFF, val, 0x7FFF);

			val = samp[i + 1] >> 8;
			out[i + 1] = clamp(-0x7FFF, val, 0x7FFF);
		}

		samp += linear_count;
		lpainted_time += (linear_count >> 1);
	}
}

static void snd_raw_samples(int samples, int rate, int width, int channels, byte* data)
{
	float scale;
	int intvolume, i, src, dst;

	if (gsnd.rawend < gsnd.painted_time)
		gsnd.rawend = gsnd.painted_time;

	intvolume = gsnd.musicvol->value * 255;
	intvolume = clamp(0, intvolume, 255);
	scale = (float)rate / SND_SPEED;

	if (channels == 2 && width == 2)
	{
		for (i = 0; ; i++)
		{
			src = (int)(i * scale);
			if (src >= samples)
				break;

			dst = gsnd.rawend & (SND_PAINT_SIZE - 1);
			gsnd.rawend++;

			grawsamples[dst].left = ((short*)data)[src * 2] * intvolume;
			grawsamples[dst].right = ((short*)data)[src * 2 + 1] * intvolume;
		}
	}
	else if (channels == 1 && width == 2)
	{
		for (i = 0; ; i++)
		{
			src = (int)(i * scale);
			if (src >= samples)
				break;

			dst = gsnd.rawend & (SND_PAINT_SIZE - 1);
			gsnd.rawend++;

			grawsamples[dst].left = ((short*)data)[src] * intvolume;
			grawsamples[dst].right = ((short*)data)[src] * intvolume;
		}
	}
	else if (channels == 2 && width == 1)
	{
		intvolume *= 255;
		for (i = 0; ; i++)
		{
			src = (int)(i * scale);
			if (src >= samples)
				break;

			dst = gsnd.rawend & (SND_PAINT_SIZE - 1);
			gsnd.rawend++;

			grawsamples[dst].left = (((byte*)data)[src * 2] - 128) * intvolume;
			grawsamples[dst].right = (((byte*)data)[src * 2 + 1] - 128) * intvolume;
		}
	}
	else if (channels == 1 && width == 1)
	{
		intvolume *= 255;
		for (i = 0; ; i++)
		{
			src = (int)(i * scale);

			if (src >= samples)
			{
				break;
			}

			dst = gsnd.rawend & (SND_PAINT_SIZE - 1);
			gsnd.rawend++;

			grawsamples[dst].left = (((byte*)data)[src] - 128) * intvolume;
			grawsamples[dst].right = (((byte*)data)[src] - 128) * intvolume;
		}
	}
}

static void snd_music_stream()
{
	short samples[SND_PAINT_SIZE / 2] = { 0 };
	int read_samples = stb_vorbis_get_samples_short_interleaved(gogg.stream, gogg.stream->channels, samples, sizeof(samples) / sizeof(short));

	if (read_samples > 0)
	{
		gogg.pos += read_samples;
		snd_raw_samples(read_samples, gogg.stream->sample_rate, sizeof(short), gogg.stream->channels, (byte*)samples);
	}
	else
	{
		if (gogg.loop)
		{
			gogg.pos = 0;
			stb_vorbis_seek_start(gogg.stream);

			read_samples = stb_vorbis_get_samples_short_interleaved(gogg.stream, gogg.stream->channels, samples, sizeof(samples) / sizeof(short));
			if (read_samples > 0)
			{
				gogg.pos += read_samples;
				snd_raw_samples(read_samples, gogg.stream->sample_rate, sizeof(short), gogg.stream->channels, (byte*)samples);
			}
			else
				snd_music_stop();
		}
		else
			snd_music_stop();
	}
}

static void snd_paint_channel_from8(channel_s* ch, samplepair_s* samp, int count)
{
	byte* sfx;
	int* lscale, * rscale, i;

	lscale = gscaletable[ch->leftvol >> 3];
	rscale = gscaletable[ch->rightvol >> 3];
	sfx = ch->wav->data + ch->pos;
	ch->pos += count;

	if (ch->wav->channels == 2)
	{
		count <<= 1;
		for (i = 0; i < count; i += 2, samp++)
		{
			samp->left += lscale[sfx[i + 0]];
			samp->right += rscale[sfx[i + 1]];
		}
	}
	else
	{
		int data;

		for (i = 0; i < count; i++, samp++)
		{
			data = sfx[i];
			samp->left += lscale[data];
			samp->right += rscale[data];
		}
	}
}

static void snd_paint_channel_from16(channel_s* ch, samplepair_s* samp, int count)
{
	short* sfx;
	int i, left, right;

	left = ch->leftvol * gsnd.ivolume;
	right = ch->rightvol * gsnd.ivolume;
	sfx = (short*)ch->wav->data + ch->pos;
	ch->pos += count;

	if (ch->wav->channels == 2)
	{
		count <<= 1;
		for (i = 0; i < count; i += 2, samp++)
		{
			samp->left += (left * sfx[i + 0]) >> 8;
			samp->right += (right * sfx[i + 1]) >> 8;
		}
	}
	else
	{
		int data;

		for (i = 0; i < count; i++, samp++)
		{
			data = sfx[i];
			samp->left += (left * data) >> 8;
			samp->right += (right * data) >> 8;
		}
	}
}

static void snd_paint_channels(int end_time)
{
	channel_s* ch;
	int i, ltime, count, end;
	static samplepair_s paint_buffer[SND_PAINT_SIZE];

	while (gsnd.painted_time < end_time)
	{
		end = end_time;
		if (end_time - gsnd.painted_time > SND_PAINT_SIZE)
			end = gsnd.painted_time + SND_PAINT_SIZE;

		memzero(paint_buffer, (end - gsnd.painted_time) * sizeof(samplepair_s));
		for (i = 0; i < SND_MAX_CHANNELS; i++)
		{
			ch = &gchannels[i];
			if (!ch->wav || (!ch->leftvol && !ch->rightvol))
				continue;

			ltime = gsnd.painted_time;
			while (ltime < end)
			{
				count = end - ltime;
				if (ch->end - ltime < count)
					count = ch->end - ltime;

				if (count > 0)
				{
					if (ch->wav->width == 1)
						snd_paint_channel_from8(ch, &paint_buffer[ltime - gsnd.painted_time], count);
					else
						snd_paint_channel_from16(ch, &paint_buffer[ltime - gsnd.painted_time], count);
					ltime += count;
				}

				if (ltime >= ch->end)
				{
					if (ch->loop)
					{
						ch->pos = 0;
						ch->end = ltime + ch->wav->samples;
					}
					else
					{
						ch->wav = NULL;
						break;
					}
				}
			}
		}

		if (gsnd.rawend >= gsnd.painted_time)
		{
			int stop;

			stop = (end < gsnd.rawend) ? end : gsnd.rawend;
			for (i = gsnd.painted_time; i < stop; i++)
			{
				int s;

				s = i & (SND_PAINT_SIZE - 1);
				paint_buffer[i - gsnd.painted_time].left += grawsamples[s].left;
				paint_buffer[i - gsnd.painted_time].right += grawsamples[s].right;
			}
		}

		snd_transfer_stereo16((int*)paint_buffer, end);
		gsnd.painted_time = end;
	}
}

void snd_update()
{
	int end_time, samps;

	if (!gsnd.p_ds)
		return;

	snd_begin_painting();
	if (!gdma.buffer)
		return;

	snd_update_pos();
	snd_update_soundtime();
	while (gogg.stream && !gogg.pause && ((gsnd.painted_time + SND_PAINT_SIZE - 2048) > gsnd.rawend))
		snd_music_stream();

	samps = gdma.samples >> (SND_CHANNELS - 1);
	end_time = gsnd.sound_time + SND_MIX_AHEAD * SND_SPEED;
	if (end_time - gsnd.sound_time > samps)
		end_time = gsnd.sound_time + samps;

	snd_paint_channels(end_time);
	snd_submit();
}

void snd_set_param()
{
	if (gsnd.volume->is_change)
	{
		int i, j, scale;

		gsnd.volume->value = clamp(SND_MIN_VOL, gsnd.volume->value, SND_MAX_VOL);
		gsnd.ivolume = gsnd.volume->value * 255;
		gsnd.ivolume = clamp(0, gsnd.ivolume, 255);
		gsnd.volume->is_change = FALSE;

		for (i = 0; i < 32; i++)
		{
			scale = i * 8 * 255 * gsnd.volume->value;
			for (j = 0; j < 256; j++)
				gscaletable[i][j] = ((char)j) * scale;
		}
	}
}