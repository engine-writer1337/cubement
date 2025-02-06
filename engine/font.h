#ifndef _FONT_H_
#define _FONT_H_

#define FONT_DIMENSION	16
#define FONT_LETTERS	256
#define FONT_VERTS		1024

#define FONT_MAGIC		'TNOF'
#define FONT_FOLDER		"fonts/"

#define MAX_FONTS		10

typedef struct
{
	name_t name;
	glpic_t texture;

	int height;
	int starts[FONT_LETTERS];
	int widths[FONT_LETTERS];

	bool_t is_temp;
}font_s;

extern ihandle_t gconfont;

int font_height(ihandle_t idx);
int font_len(ihandle_t idx, const char* text);
int font_print(ihandle_t idx, const char* text, int x, int y, render_e render, byte r, byte g, byte b, byte a);

ihandle_t font_precache(const char* fontname);

void font_init();
void font_free_all();
void font_free_temp();

#endif