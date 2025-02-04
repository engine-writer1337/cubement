#ifndef _STUDIO_H_
#define _STUDIO_H_

#define MDL_NAGIC		' LDM'

#define MDL_MAX_GROUPS		8
#define MDL_MAX_BODIES		16
#define MDL_MAX_SEQUENCES	256
#define MDL_MAX_FRAMES		256
#define MDL_MAX_SKINS		256
#define MDL_MAX_TEXTURES	256
#define MDL_MAX_BONES		256
#define MDL_MAX_MESHES		256
#define MDL_MAX_MODELS		256
#define MDL_MAX_VERTS		65536
#define MDL_MAX_TEXCOORDS	65536

#define TEXCOORD_SCALE		32000
#define TEXCOORD_SCALE_INV	(1.f / 32000)

#define TEXFLAG_TRANSPARENT		(1 << 0)
#define TEXFLAG_ADDITIVE		(1 << 1)

typedef float vec2_t[2];
typedef float vec3_t[3];
typedef unsigned char byte;

typedef struct 
{
	int magic;

	int numbones;
	int boneindex;

	int numverts;
	int vertindex;

	int numvertinfos;
	int vertinfoindex;

	int numtexcoords;
	int texcoordsindex;

	int numseq;
	int seqindex;

	int numtextures;
	int textureindex;

	int numskins;
	int numskinfamilies;
	int skinindex;

	int numbodyparts;
	int bodypartindex;

	int nummodels;
	int modelindex;

	int nummeshes;
	int meshindex;
}studiohdr_s;

typedef struct
{
	char name[30];
	short parent;
	float value[6];
	float scale[6];
}mstudiobone_s;

typedef struct
{
	char label[30];

	byte fps;
	byte numframes;

	vec3_t bbmin;
	vec3_t bbmax;

	int animindex;
}mstudioseqdesc_s;

typedef struct
{
	unsigned short offset[6];
}mstudioanim_s;

typedef union
{
	struct
	{
		byte valid;
		byte total;
	}num;
	short value;
}mstudioanimvalue_s;

typedef struct
{
	byte flags;
	char name[23];
}mstudiotexture_s;

//skin families
//byte index[skinfamilies][numtextures]

typedef struct
{
	char name[31];
	byte nummodels;
	byte modelindex;
}mstudiobodyparts_s;

typedef struct
{
	char name[30];
	byte nummeshes;
	byte meshindex;
}mstudiomodel_s;

typedef struct
{
	unsigned short texture;
	unsigned short numcommands;
	int commandindex;
}mstudiomesh_s;

typedef struct
{
	vec3_t v;
}mstudiovert_s;

typedef struct
{
	short s, t;
}mstudiotexcoord_s;

#endif
