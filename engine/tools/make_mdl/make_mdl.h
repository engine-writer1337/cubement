#ifndef _MAKE_MDL_H_
#define _MAKE_MDL_H_

#include "studio.h"

#define STUDIO_X		0x0001
#define STUDIO_Y		0x0002
#define STUDIO_Z		0x0004
#define STUDIO_XR		0x0008
#define STUDIO_YR		0x0010
#define STUDIO_ZR		0x0020
#define STUDIO_LX		0x0040
#define STUDIO_LY		0x0080
#define STUDIO_LZ		0x0100
#define STUDIO_AX		0x0200
#define STUDIO_AY		0x0400
#define STUDIO_AZ		0x0800
#define STUDIO_AXR		0x1000
#define STUDIO_AYR		0x2000
#define STUDIO_AZR		0x4000
#define STUDIO_TYPES	0x7FFF

#define STUDIO_LOOPING	0x0001

#ifndef EXTERN
#define EXTERN extern
#endif

EXTERN	int		gnumhitbones;
EXTERN	int		ghitboneidx[MDL_MAX_BONES];
EXTERN	char	ghitbonenames[MDL_MAX_BONES][64];

EXTERN	int		gnumgaitbones;
EXTERN	char	ggaitbonenames[MDL_MAX_BONES][64];

EXTERN	char		outname[1024];
EXTERN  qboolean	cdset;
EXTERN	char		cdpartial[256];
EXTERN	char		cddir[256];
EXTERN	int			cdtextureset;
EXTERN	char		cdtexture[16][256];

EXTERN	char		pivotname[32][256];	// names of the pivot points

EXTERN	float		default_scale;
EXTERN	float		scale_up;
EXTERN  float		defaultzrotation;
EXTERN	float		zrotation;


EXTERN	char		defaulttexture[16][256];
EXTERN	char		sourcetexture[16][256];

EXTERN	int			numrep;

EXTERN	int			tag_reversed;
EXTERN	int			tag_normals;
EXTERN	int			flip_triangles;
EXTERN	float		normal_blend;
EXTERN	int			dump_hboxes;
EXTERN	int			ignore_warnings;

EXTERN	vec3_t		eyeposition;
EXTERN	int			gflags;
EXTERN	vec3_t		bbox[2];
EXTERN	vec3_t		cbox[2];

EXTERN	int			maxseqgroupsize;

EXTERN	int			split_textures;
EXTERN	int			clip_texcoords;

#define ROLL	2
#define PITCH	0
#define YAW		1

extern vec_t Q_rint(vec_t in);

extern void WriteFile(void);
void* kalloc(int num, int size);

typedef struct {
	int					vertindex;
	int					normindex;		// index into normal array
	int					s, t;
	float				u, v;
} s_trianglevert_t;

typedef struct
{
	int					bone;		// bone transformation index
	vec3_t				org;		// original position
} s_vertex_t;

typedef struct
{
	int					skinref;
	int					bone;		// bone transformation index
	vec3_t				org;		// original position
} s_normal_t;

//============================================================================
// dstudiobone_t bone[MAXSTUDIOBONES];
typedef struct
{
	vec3_t	worldorg;
	float m[3][4];
	float im[3][4];
	float length;
} s_bonefixup_t;
EXTERN	s_bonefixup_t bonefixup[MDL_MAX_BONES];

int numbones;
typedef struct
{
	char			name[32];	// bone name for symbolic links
	int		 		parent;		// parent bone
	int				bonecontroller;	// -1 == 0
	int				flags;		// X, Y, Z, XR, YR, ZR
	// short		value[6];	// default DoF values
	vec3_t			pos;		// default pos
	vec3_t			posscale;	// pos values scale
	vec3_t			rot;		// default pos
	vec3_t			rotscale;	// rotation values scale
	int				group;		// hitgroup
	vec3_t			bmin, bmax;	// bounding box
} s_bonetable_t;
EXTERN	s_bonetable_t bonetable[MDL_MAX_BONES];

int numrenamedbones;
typedef struct
{
	char			from[32];
	char			to[32];
} s_renamebone_t;
EXTERN s_renamebone_t renamedbone[MDL_MAX_BONES];

int numhitboxes;
typedef struct
{
	char			name[32];	// bone name
	int				bone;
	int				group;		// hitgroup
	int				model;
	vec3_t			bmin, bmax;	// bounding box
} s_bbox_t;
EXTERN s_bbox_t hitbox[MDL_MAX_BONES];

int numhitgroups;
typedef struct
{
	int				models;
	int				group;
	char			name[32];	// bone name
} s_hitgroup_t;
EXTERN s_hitgroup_t hitgroup[MDL_MAX_BONES];


typedef struct
{
	char	name[32];
	int		bone;
	int		type;
	int		index;
	float	start;
	float	end;
} s_bonecontroller_t;

s_bonecontroller_t bonecontroller[MDL_MAX_BONES];
int numbonecontrollers;

typedef struct
{
	char	name[32];
	char	bonename[32];
	int		index;
	int		bone;
	int		type;
	vec3_t	org;
} s_attachment_t;

s_attachment_t attachment[MDL_MAX_BONES];
int numattachments;

typedef struct
{
	char			name[64];
	int				parent;
	int				mirrored;
} s_node_t;

EXTERN char mirrored[MDL_MAX_BONES][64];
EXTERN int nummirrored;

EXTERN	int numani;
typedef struct
{
	char			name[64];
	int				startframe;
	int				endframe;
	int				flags;
	int				numbones;
	s_node_t		node[MDL_MAX_BONES];
	int				bonemap[MDL_MAX_BONES];
	int				boneimap[MDL_MAX_BONES];
	vec3_t* pos[MDL_MAX_BONES];
	vec3_t* rot[MDL_MAX_BONES];
	int				numanim[MDL_MAX_BONES][6];
	mstudioanimvalue_s* anim[MDL_MAX_BONES][6];
} s_animation_t;
EXTERN	s_animation_t* panimation[MDL_MAX_FRAMES];

typedef struct
{
	int				event;
	int				frame;
	char			options[64];
} s_event_t;

typedef struct
{
	int				index;
	vec3_t			org;
	int				start;
	int				end;
} s_pivot_t;

EXTERN	int numseq;
typedef struct
{
	int				motiontype;
	vec3_t			linearmovement;

	char			name[64];
	int				flags;
	float			fps;
	int				numframes;

	int				activity;
	int				actweight;

	int				frameoffset; // used to adjust frame numbers

	int numblends;
	s_animation_t* panim[MDL_MAX_GROUPS];

	float			blendtype[2];
	float			blendstart[2];
	float			blendend[2];

	vec3_t			automovepos[MDL_MAX_FRAMES];
	vec3_t			automoveangle[MDL_MAX_FRAMES];

	int				seqgroup;
	int				animindex;

	vec3_t 			bmin;
	vec3_t			bmax;

	int				entrynode;
	int				exitnode;
	int				nodeflags;
} s_sequence_t;
EXTERN	s_sequence_t sequence[MDL_MAX_SEQUENCES];

EXTERN int numseqgroups;
typedef struct {
	char	label[32];
	char	name[64];
} s_sequencegroup_t;
EXTERN s_sequencegroup_t sequencegroup[MDL_MAX_SEQUENCES];

EXTERN int numxnodes;
EXTERN int xnode[100][100];

typedef struct {
	byte r, g, b;
} rgb_t;
typedef struct {
	byte b, g, r, x;
} rgb2_t;

EXTERN int gnumtextures;
EXTERN mstudiotexture_s gtexture[MDL_MAX_TEXTURES];

EXTERN  float gamma;
EXTERN	int numskinref;
EXTERN  int numskinfamilies;
EXTERN  int skinref[MDL_MAX_SKINS][MDL_MAX_SKINS]; // [skin][skinref], returns texture index
EXTERN	int numtexturegroups;
EXTERN	int numtexturelayers[32];
EXTERN	int numtexturereps[32];
EXTERN  int texturegroup[32][32][32];

typedef struct
{
	int alloctris;
	int numtris;
	s_trianglevert_t(*triangle)[3];

	int skinref;
	int numnorms;
} s_mesh_t;

typedef struct
{
	vec3_t			pos;
	vec3_t			rot;
} s_bone_t;

typedef struct s_model_s
{
	char name[64];

	int numbones;
	s_node_t node[MDL_MAX_BONES];
	s_bone_t skeleton[MDL_MAX_BONES];
	int boneref[MDL_MAX_BONES]; // is local bone (or child) referenced with a vertex
	int	bonemap[MDL_MAX_BONES]; // local bone to world bone mapping
	int	boneimap[MDL_MAX_BONES]; // world bone to local bone mapping

	vec3_t boundingbox[MDL_MAX_BONES][2];

	s_mesh_t* trimesh[MDL_MAX_VERTS];
	int trimap[MDL_MAX_VERTS];

	int numverts;
	s_vertex_t vert[MDL_MAX_VERTS];

	int numnorms;
	s_normal_t normal[MDL_MAX_VERTS];

	int nummesh;
	s_mesh_t* pmesh[MDL_MAX_MESHES];

	float boundingradius;

	int numframes;
	float interval;
	struct s_model_s* next;
} s_model_t;

EXTERN	int nummodels;
EXTERN	s_model_t* model[MDL_MAX_MODELS];

EXTERN	vec3_t adjust;
EXTERN	vec3_t defaultadjust;

typedef struct
{
	char				name[32];
	int					nummodels;
	int					base;
	s_model_t* pmodel[MDL_MAX_BODIES];
} s_bodypart_t;

EXTERN	int numbodyparts;
EXTERN	s_bodypart_t bodypart[MDL_MAX_GROUPS];

#endif