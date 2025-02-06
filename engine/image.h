#ifndef _IMAGE_H_
#define _IMAGE_H_

#define IMG_MAX_PICS		512
#define IMG_MAX_SIZE		1024 * 1024

typedef struct
{
	glpic_t t;

	int width;
	int height;

	hash_t hash;
	name_t name;

	bool_t is_temp;
}pic_s;

typedef struct
{
	glpic_t null;
	glpic_t current;
	int max_aniso;

	int out_width;
	int out_height;

	byte* rgba;

	bool_t clamp;
	bool_t mipmap;
	bool_t nearest;

	cvar_s* aniso;
	cvar_s* nofilter;
}image_s;

extern image_s gimg;

void img_start();
void img_end();
void img_bind(glpic_t t);

glpic_t img_upload(int width, int height, int format);

glpic_t img_load(const char* filename);
ihandle_t img_precache_pic(const char* filename, int flags);
void img_scrshot_cmd(const char* arg1, const char* arg2);
void img_pic_draw(ihandle_t idx, int x, int y, render_e render, byte r, byte g, byte b, byte a, const prect_s* rect);

void img_init();
void img_free_all();
void img_set_param();

#endif