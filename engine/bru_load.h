#ifndef _BRU_LOAD_H_
#define _BRU_LOAD_H_

#include "tools/make_bru/bru.h"

#define BRU_FOLDER		"maps/"
#define TEXTURE_FOLDER	"textures/"

typedef struct
{
	int texture;
	vec4_t vecs[2];
}tmp_texinfo_s;

typedef struct
{
	int count;
	vec2_t* st;
	vec3_t* xyz;

	glbuf_t buffer;
}vertbuf_s;

typedef struct
{
	int framenum;

	char name[20];
	int activecount;

	int num_boxes;
	vec2_t* mins;
	vec2_t* maxs;

	int num_brushareas;
	word* brushareas;
}area_s;

typedef struct _surf_s
{
	int offset;
	int texture;
	byte color[4];
	surftype_e type;
	struct _surf_s* next;
}surf_s;

typedef struct
{
	glpic_t t;
	char name[32];
	int width, height;

	surf_s* chain;
}btexture_s;

typedef struct
{
	int framenum;

	int num_surfes;
	surf_s* surfes;

	vec3_t mins;
	vec3_t maxs;
}brush_s;

typedef struct
{
	int num_brushes;
	brush_s* brushes;

	vec3_t mins;
	vec3_t maxs;
}brushmodel_s;

typedef struct
{
	name_t name;

	int num_models;
	brushmodel_s* models;

	int num_textures;
	btexture_s* textures;

	int num_brushes;
	brush_s* brushes;

	int num_surfes;
	surf_s* surfes;

	int num_areas;
	area_s* areas;

	word* brushareas;

	vec2_t* box_maxs;
	vec2_t* box_mins;
}bru_s;

extern bru_s gbru;
extern vertbuf_s gvertbuf;

bool_t bru_load(const char* name);
void bru_free();

void bru_unload();
void bru_reload();

#endif