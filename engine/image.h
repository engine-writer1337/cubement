#ifndef _IMAGE_H_
#define _IMAGE_H_

#define IMG_MAX_SIZE		1024 * 1024

typedef struct
{
	glpic_t t;

	int width;
	int height;
	int flags;
}pic_s;

typedef struct
{
	glpic_t null;
	glpic_t current;
	int max_aniso;

	int out_width;
	int out_height;

	bool_t clamp;
	bool_t mipmap;
	bool_t nearest;

	convar_s* aniso;
	convar_s* nofilter;
}image_s;

extern image_s gimg;

void img_bind(glpic_t t);

void img_set_filter(glpic_t pic);
glpic_t img_upload(int width, int height, int format);

glpic_t img_load(const char* filename);
void img_scrshot_cmd(const char* arg1, const char* arg2);
void img_pic_draw(ihandle_t idx, int frame, int x, int y, render_e render, byte r, byte g, byte b, byte a);

void img_init();
void img_free();
void img_set_param();

#endif