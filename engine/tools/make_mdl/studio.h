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

#define TEXFLAG_TRANSPARENT		(1 << 0)
#define TEXFLAG_ADDITIVE		(1 << 1)

typedef float vec2_t[2];
typedef float vec3_t[3];
typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned char bool_t;

typedef struct 
{
	int magic;

	int numtris;

	int numbones;
	int ofsbones;
	
	int numverts;
	int ofsverts;
	int ofsvertbones;
	
	int numtexcoords;
	int ofstexcoords;
	
	int numnormals;
	int ofsnormals;
	int ofsnormbones;
	
	int numseq;
	int ofsseq;
	
	int numtextures;
	int ofstextures;
	
	int numskins;
	int ofsskins;
	
	int numbodyparts;
	int ofsbodyparts;

	int nummodels;
	int ofsmodels;
	
	int nummeshes;
	int ofsmeshes;
}studiohdr_s;

typedef struct
{
	char name[32];
	
	short parent;
	bool_t is_gait;
	byte hitbox;
	
	float value[6];
	float scale[6];
	
	vec3_t mins;
	vec3_t maxs;
}mstudiobone_s;

typedef struct
{
	char label[31];
	byte numframes;

	vec3_t bbmin;
	vec3_t bbmax;

	int ofsframes;
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
	char name[31];
}mstudiotexture_s;

//skin families
//byte index[skinfamilies][numtextures]

typedef struct
{
	char name[30];
	
	byte nummodels;
	byte start_model;
}mstudiobodyparts_s;

typedef struct
{
	char name[30];
	
	byte nummeshes;
	byte start_mesh;
}mstudiomodel_s;

typedef struct
{
	word texture;
	word numcommands;
	int ofscommands; //normal (word) + vert index (word) + texcoord (word)
}mstudiomesh_s;

typedef struct
{
	vec2_t uv;
}mstudiotexcoord_s;

typedef struct
{
	vec3_t norm;
}mstudionorm_s;

typedef struct
{
	vec3_t v;
}mstudiovert_s;

#endif
