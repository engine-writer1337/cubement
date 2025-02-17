#ifndef _BRU_H_
#define _BRU_H_

#define MAP_MAGIC		' URB'

#define MAX_MAP_KEY		32
#define MAX_MAP_VALUE	256

#define MAX_MAP_AREAS		256
#define MAX_MAP_TEXTURES	768
#define MAX_MAP_ENTITIES	2048
#define MAX_MAP_AREABOXES	2048
#define MAX_MAP_RANGE		16384
#define MAX_MAP_BRUSHES		65536
#define MAX_MAP_TEXINFOS	65536
#define MAX_MAP_SURFACES	65536
#define MAX_MAP_BRUSHAREAS	262144
#define MAX_MAP_ENTSTRING	(2 * 1024 * 1024)

#define TEXCOORD_SCALE		1000
#define TEXCOORD_SCALE_INV	(1.f / TEXCOORD_SCALE)

#define TEXCOORD_OFFSET		10
#define TEXCOORD_OFFSET_INV	(1.f / TEXCOORD_OFFSET)

#define BRU_LUMP_SURFACES		0
#define BRU_LUMP_BRUSHES		1
#define BRU_LUMP_TEXINFOS		2
#define BRU_LUMP_MODELS			3
#define BRU_LUMP_TEXTURES		4
#define BRU_LUMP_ENTITIES		5
#define BRU_LUMP_AREAS			7
#define BRU_LUMP_AREABOXES		8
#define BRU_LUMP_BRUSHAREAS		9
#define BRU_LUMP_NUMS			10

typedef enum
{
	SURF_TYPE_X,
	SURF_TYPE_Y,
	SURF_TYPE_Z,
	SURF_TYPE_SX,
	SURF_TYPE_SY,
	SURF_TYPE_SZ,
	SURF_TYPE_BAD
}surftype_e;

typedef unsigned char byte;
typedef unsigned short word;

typedef struct
{
	int ofs, len;
}bru_lump_s;

typedef struct
{
	char name[32];
}bru_texture_s;

typedef struct
{
	int texture;
	short vecs[2][4];
}bru_texinfo_s;

typedef struct
{
	word encode;//type + color
	word texinfo;
}bru_surf_s;

typedef struct
{
	word num_surfaces;
	word start_surface;

	short mins[3];
	short maxs[3];
}bru_brush_s;

typedef struct
{
	word num_brushes;
	word start_brush;

	short mins[3];
	short maxs[3];
	short ofs[3];
}bru_model_s;

typedef struct
{
	short mins[2];
	short maxs[2];
}bru_areabox_s;

typedef struct
{
	char name[18];

	word num_brushareas;
	int start_brusharea;

	word start_box;
	word num_boxes;
}bru_area_s;

typedef struct
{
	int magic;
	bru_lump_s lumps[BRU_LUMP_NUMS];
}bru_header_s;

#endif