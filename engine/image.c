#include "cubement.h"
#include "libs/turbojpeg.h"

image_s gimg;
static pic_s gpics[IMG_MAX_PICS];
extern const char* minipng_load(const byte* buffer, int filesize, byte* rgba, int* has_alpha);
extern const char* minipng_size(const byte* buffer, int filesize, int* out_width, int* out_height);

void img_bind(glpic_t t)
{
	if (t == gimg.current)
		return;

	gimg.current = t;
	if (t)
		glBindTexture(GL_TEXTURE_2D, t);
	else
		glBindTexture(GL_TEXTURE_2D, gimg.null);
}

static ihandle_t img_alloc_pic()
{
	int i;
	pic_s* p;

	for (i = 0; i < IMG_MAX_PICS; i++)
	{
		p = gpics + i;
		if (!p->name[0])
			return i;
	}

	con_print(COLOR_RED, "IMG_MAX_PICS");
	return BAD_HANDLE;
}

static void img_free_pic(ihandle_t idx)
{
	pic_s* p = gpics + idx;
	if (!p->name[0])
		return;

	util_tex_free(p->t);
	p->name[0] = '\0';
	p->t = 0;
}

static ihandle_t img_find_pic(const char* name)
{
	int i;
	pic_s* p;
	hash_t hash;

	hash = util_hash_str(name);
	for (i = 0; i < IMG_MAX_PICS; i++)
	{
		p = gpics + i;
		if (!p->name[0] || p->hash != hash)
			continue;

		if (strcmpi(p->name, name))
			return i;
	}

	return BAD_HANDLE;
}

void img_set_filter(glpic_t pic)
{
	gimg.current = pic;
	glBindTexture(GL_TEXTURE_2D, pic);
	if (gimg.mipmap)
	{
		if (gimg.aniso->value && gimg.max_aniso > 1)
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, gimg.max_aniso);

		if (gimg.nearest)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
	}
	else
	{
		if (gimg.nearest)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
	}

	if (gimg.clamp)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}
}

static void img_mipmap(byte* in, int width, int height, int format)
{
	int i, j;
	byte* out;

	out = in;
	width <<= 2;
	height >>= 1;
	if (format == GL_RGB)
	{
		for (i = 0; i < height; i++, in += width)
		{
			for (j = 0; j < width; j += 8, out += 4, in += 8)
			{
				out[0] = (in[0] + in[4] + in[width + 0] + in[width + 4]) >> 2;
				out[1] = (in[1] + in[5] + in[width + 1] + in[width + 5]) >> 2;
				out[2] = (in[2] + in[6] + in[width + 2] + in[width + 6]) >> 2;
			}
		}
	}
	else
	{
		for (i = 0; i < height; i++, in += width)
		{
			for (j = 0; j < width; j += 8, out += 4, in += 8)
			{
				out[0] = (in[0] + in[4] + in[width + 0] + in[width + 4]) >> 2;
				out[1] = (in[1] + in[5] + in[width + 1] + in[width + 5]) >> 2;
				out[2] = (in[2] + in[6] + in[width + 2] + in[width + 6]) >> 2;
				out[3] = (in[3] + in[7] + in[width + 3] + in[width + 7]) >> 2;
			}
		}
	}
}

glpic_t img_upload(byte* data, int width, int height, int format)
{
	glpic_t retval;

	gimg.out_width = width;
	gimg.out_height = height;

	retval = util_tex_gen();
	img_set_filter(retval);
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	if (gimg.mipmap)
	{
		int mipmap = 1;

		while (width > 1 || height > 1)
		{
			img_mipmap(data, width, height, format);

			width >>= 1;
			height >>= 1;

			if (width < 1) width = 1;
			if (height < 1) height = 1;

			glTexImage2D(GL_TEXTURE_2D, mipmap++, format, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
	}

	util_free(data);
	return retval;
}

static glpic_t img_load_jpg(const char* filename)
{
	tjhandle jpeg;
	byte* file, * data;
	int len, w, h, samps;

	file = util_full(filename, &len);
	if (!file)
	{
		con_printf(COLOR_RED, "%s - not found", filename);
		return 0;
	}

	jpeg = tjInitDecompress();
	if (tjDecompressHeader2(jpeg, file, len, &w, &h, &samps))
	{
		util_free(file);
		tjDestroy(jpeg);
		con_printf(COLOR_RED, "%s - bad format", filename);
		return 0;
	}

	if (w != ((w >> 2) << 2) || w * h > IMG_MAX_SIZE)
	{
		util_free(file);
		tjDestroy(jpeg);
		con_printf(COLOR_RED, "%s - bad size %i x %i", filename, w, h);
		return 0;
	}

	data = util_malloc((w * h) << 2);
	if (tjDecompress2(jpeg, file, len, data, w, 0, h, TJPF_RGBX, TJFLAG_FASTDCT))
	{
		util_free(data);
		util_free(file);
		tjDestroy(jpeg);
		con_printf(COLOR_RED, "%s - couldn't decompress", filename);
		return 0;
	}

	tjDestroy(jpeg);
	util_free(file);
	return img_upload(data, w, h, GL_RGB);
}

static glpic_t img_load_png(const char* filename)
{
	const char* error;
	byte* file, * data;
	int has_alpha, w, h, len;

	file = util_full(filename, &len);
	if (!file)
	{
		con_printf(COLOR_RED, "%s - not found", filename);
		return 0;
	}

	if (error = minipng_size(file, len, &w, &h))
	{
		util_free(file);
		con_printf(COLOR_RED, "%s - %s", filename, error);
		return 0;
	}

	if (w != ((w >> 2) << 2) || w * h > IMG_MAX_SIZE)
	{
		util_free(file);
		con_printf(COLOR_RED, "%s - bad size %i x %i", filename, w, h);
		return 0;
	}

	data = util_malloc((w * h) << 2);
	if (error = minipng_load(file, len, data, &has_alpha))
	{
		util_free(data);
		util_free(file);
		con_printf(COLOR_RED, "%s - %s", filename, error);
		return 0;
	}

	util_free(file);
	return img_upload(data, w, h, has_alpha ? GL_RGBA : GL_RGB);
}

glpic_t img_load(const char* filename)
{
	int len;

	len = (int)strlen(filename);
	if (*(int*)(filename + len - 4) == 'gpj.')
		return img_load_jpg(filename);
	else if (*(int*)(filename + len - 4) == 'gnp.')
		return img_load_png(filename);
	else
	{
		string_t fullname;

		strcpy(fullname, filename);
		strcpy(fullname + len, ".jpg");
		if (util_exist(fullname))
			return img_load_jpg(fullname);
		else
		{
			strcpy(fullname + len, ".png");
			if (util_exist(fullname))
				return img_load_png(fullname);
		}
	}

	con_printf(COLOR_RED, "%s - unknown format", filename);
	return 0;
}

ihandle_t img_precache_pic(const char* filename, int flags)
{
	pic_s* p;
	ihandle_t idx;

	if (!ghost.load_is_allow)
	{
		con_printf(COLOR_RED, "%s - precache is not allowed");
		return BAD_HANDLE;
	}

	if (strnull(filename))
		return BAD_HANDLE;

	idx = img_find_pic(filename);
	if (idx != BAD_HANDLE)
		return idx;

	idx = img_alloc_pic();
	if (idx == BAD_HANDLE)
		return BAD_HANDLE;

	gimg.mipmap = FALSE;
	gimg.clamp = (flags & IMG_CLAMP);
	gimg.nearest = (flags & IMG_NEAREST);
	
	p = gpics + idx;
	p->t = img_load(filename);
	if (!p->t)
		return BAD_HANDLE;

	p->width = gimg.out_width;
	p->height = gimg.out_height;
	strcpyn(p->name, filename);
	p->hash = util_hash_str(p->name);
	p->is_temp = ghost.load_as_temp;
	return idx;
}

void img_scrshot_cmd(const char* arg1, const char* arg2)
{
	FILE* fp;
	tjhandle jpeg;
	string_t name;
	int i, pitch, size = 0;
	byte* buffer, * output, * buffer2;

	strcpy(name, gbru.name);
	strcat(name, util_get_timestamp());
	strcat(name, ".jpg");
	fp = util_open(name, "wb");
	if (!fp)
	{
		con_printf(COLOR_RED, "%s - save failed", name);
		return;
	}

	pitch = gvid.width * 3;
	buffer = util_malloc(pitch * gvid.height);
	buffer2 = util_malloc(pitch * gvid.height);
	glReadPixels(0, 0, gvid.width, gvid.height, GL_RGB, GL_UNSIGNED_BYTE, buffer2);
	for (i = 0; i < gvid.height; i++)
		memcpy(buffer + i * pitch, buffer2 + ((gvid.height - 1) - i) * pitch, pitch);

	jpeg = tjInitCompress();
	tjCompress2(jpeg, buffer, gvid.width, 0, gvid.height, TJPF_RGB, &output, &size, TJSAMP_444, 95, TJFLAG_FASTDCT);
	tjDestroy(jpeg);
	util_free(buffer);
	util_free(buffer2);

	fwrite(output, sizeof(byte), size, fp);
	util_close(fp);
	tjFree(output);
}

void img_pic_draw(ihandle_t idx, int x, int y, render_e render, byte r, byte g, byte b, byte a, const prect_s* rect)
{//TODO: rect
	pic_s* p;
	vec2_t verts[4];
	vec2_t texcoords[4];

	if ((unsigned)idx >= IMG_MAX_PICS)
		return;

	p = gpics + idx;
	if (!p->t)
		return;

	vec2_set(verts[0], x, y);
	vec2_set(verts[1], x + p->width, y);
	vec2_set(verts[2], x + p->width, y + p->height);
	vec2_set(verts[3], x, y + p->height);

	vec2_set(texcoords[0], 0, 0);
	vec2_set(texcoords[1], 1, 0);
	vec2_set(texcoords[2], 1, 1);
	vec2_set(texcoords[3], 0, 1);

	vid_rendermode(render);
	img_bind(p->t);
	glColor4ub(r, g, b, a);
	glVertexPointer(2, GL_FLOAT, 0, verts);
	glTexCoordPointer(2, GL_FLOAT, 0, texcoords);
	glDrawArrays(GL_QUADS, 0, 4);
}

void img_init()
{
	int x, y,* data;

	gimg.current = 0;
	glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &gimg.max_aniso);
	if (gimg.max_aniso > 8)
		gimg.max_aniso = 8;

	data = util_malloc(16 * 16 * sizeof(int));
	for (y = 0; y < 16; y++)
	{
		for (x = 0; x < 16; x++)
		{
			if ((y < 8) ^ (x < 8))
				data[y * 16 + x] = 0xFFFFFFFF;
			else
				data[y * 16 + x] = 0xFF000000;
		}
	}

	gimg.mipmap = TRUE;
	gimg.clamp = FALSE;
	gimg.nearest = TRUE;
	gimg.null = img_upload((byte*)data, 16, 16, GL_RGB);
}

void img_free_all()
{
	int i;

	util_tex_free(gimg.null);
	for (i = 0; i < IMG_MAX_PICS; i++)
		util_tex_free(gpics[i].t);
}

void img_free_temp()
{
	int i;

	for (i = 0; i < IMG_MAX_PICS; i++)
	{
		if (gpics[i].is_temp)
			img_free_pic(i);
	}
}

void img_set_param()
{
	sky_set_param();

	if (gimg.nofilter->is_change || gimg.aniso->is_change)
	{
		gimg.mipmap = TRUE;
		gimg.clamp = FALSE;
		gimg.nearest = gimg.nofilter->value;

		if (gworld.is_load)
		{
			int i;

			for (i = 0; i < gbru.num_textures; i++)
			{
				if (gbru.textures[i].t)
					img_set_filter(gbru.textures[i].t);
			}
		}

		gimg.nofilter->is_change = FALSE;
		gimg.aniso->is_change = FALSE;
	}
}