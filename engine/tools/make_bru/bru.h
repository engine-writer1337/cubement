#ifndef _BRU_H_
#define _BRU_H_

#define MAP_MAGIC		' URB'

#define MAX_MAP_KEY		32
#define MAX_MAP_VALUE	256

#define MAX_MAP_AREAS		256
#define MAX_MAP_TEXTURES	1000
#define MAX_MAP_ENTITIES	2000
#define MAX_MAP_RANGE		16384
#define MAX_MAP_BRUSHES		64000
#define MAX_MAP_TEXINFOS	64000
#define MAX_MAP_SURFACES	65536
#define MAX_MAP_SECTORS		65536
#define MAX_MAP_BRUSHSECTOR	512000
#define MAX_MAP_ENTSTRING	(2 * 1024 * 1024)

#define TEXCOORD_SCALE		1000
#define TEXCOORD_SCALE_INV	(1.f / TEXCOORD_SCALE)

#define TEXCOORD_OFFSET		10
#define TEXCOORD_OFFSET_INV	(1.f / TEXCOORD_OFFSET)

#define TEXINFO_CLIP	65535

#define SECTOR_SIZE		512

#define BRU_LUMP_SURFACES		0
#define BRU_LUMP_BRUSHES		1
#define BRU_LUMP_TEXINFOS		2
#define BRU_LUMP_MODELS			3
#define BRU_LUMP_TEXTURES		4
#define BRU_LUMP_ENTITIES		5
#define BRU_LUMP_AREAS			7
#define BRU_LUMP_SECTORS		8
#define BRU_LUMP_BRUSHSECTORS	9
#define BRU_LUMP_NUMS			10

#define SURF_TYPE_X		0
#define SURF_TYPE_Y		1
#define SURF_TYPE_Z		2
#define SURF_TYPE_SX	3
#define SURF_TYPE_SY	4
#define SURF_TYPE_SZ	5
#define SURF_TYPE_BAD	6

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
}bru_model_s;

//word brushsectors[MAX_MAP_BRUSHSECTOR];

typedef struct
{
	int start_brushsector;
	int num_brushsectors;
}bru_sector_s;

typedef struct
{
	short mins[2];
	short maxs[2];

	word start_sector;
	word num_sectors;
}bru_area_s;

typedef struct
{
	int magic;
	bru_lump_s lumps[BRU_LUMP_NUMS];
}bru_header_s;

#endif