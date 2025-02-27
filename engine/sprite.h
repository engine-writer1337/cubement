#ifndef _SPRITE_H_
#define _SPRITE_H_

#define SPR_FOLDER		"sprites/"

typedef struct
{
	int x, y, w, h;
}sframe_s;

typedef struct
{
	glpic_t texture;

	int flags;
	int numframes;
	sframe_s* frames;
}sprite_s;

void spr_load_pic(const char* filename, sprite_s* spr);
void spr_load(const char* filename, sprite_s* spr);
void spr_free(sprite_s* spr, bool_t is_reload);

#endif