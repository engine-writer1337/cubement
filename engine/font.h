#ifndef _FONT_H_
#define _FONT_H_

#define FONT_DIMENSION	16
#define FONT_LETTERS	256
#define FONT_VERTS		1024

#define FONT_MAGIC		'TNOF'
#define FONT_FOLDER		"fonts/"

typedef struct
{
	glpic_t texture;

	int height;
	byte starts[FONT_LETTERS];
	byte widths[FONT_LETTERS];
}font_s;

extern ihandle_t gconfont;

int font_height(ihandle_t idx);
int font_len(ihandle_t idx, const char* text);
int font_print(ihandle_t idx, const char* text, int x, int y, render_e render, byte r, byte g, byte b, byte a);

void font_load(const char* name, font_s* out);

#endif