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
}vertbuf_s;

typedef struct
{
	int framenum;

	hash_t hash;
	char name[20];
	int activecount;

	int num_boxes;
	vec3_t* absmin;
	vec3_t* absmax;

	int num_brushareas;
	word* brushareas;
}area_s;

typedef struct _surf_s
{
	int itype;
	bool_t sign;
	byte color[3];

	int offset;
	int texture;

	struct _surf_s* next;
	struct _surf_s* nextdetail;
}surf_s;

typedef struct
{
	hash_t hash;
	char name[32];
	glpic_t texture;

	surf_s* chain;
}detail_s;

typedef struct
{
	glpic_t texture;

	char name[32];
	int width, height;
	material_e material;
	material_e detail;//why am I mixing this up???

	surf_s* chain;
}btexture_s;

typedef struct
{
	int framenum;

	int num_surfes;
	surf_s* surfes;

	vec3_t absmin;
	vec3_t absmax;
}brush_s;

typedef struct
{
	int num_brushes;
	int start_brush;
	brush_s* brushes;

	vec3_t mins;
	vec3_t maxs;

	vec3_t absmin;
	vec3_t absmax;

	vec3_t offset;
	vec3_t origin;
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

	vec3_t* box_absmax;
	vec3_t* box_absmin;
}bru_s;

extern bru_s gbru;
extern vertbuf_s gvertbuf;

void bru_area_active(const char* name, bool_t is_active);

bool_t bru_load(const char* name);
void bru_free();

void bru_unload();
void bru_reload();

#endif