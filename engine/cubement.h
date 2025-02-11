#ifndef _CUBEMENT_H_
#define _CUBEMENT_H_

#include "shared.h"
#include <windows.h>
#include <gl/gl.h>

typedef unsigned short	word;
typedef unsigned char	enum_t;
typedef unsigned int	dword;
typedef unsigned int	glbuf_t;
typedef unsigned int	glpic_t;
typedef float			vec4_t[4];
typedef char			name_t[64];
typedef char			constr_t[96];
typedef char			string_t[1024];

#include "util.h"
#include "console.h"

typedef enum
{
	PRE_NOT,
	PRE_PERS,
	PRE_TEMP,
}pretype_e;

typedef struct
{
	int files;
	int memory;
	int textures;
	int buffers;

	ftime_t time;
	ftime_t frametime;
	ftime_t gametime;

	name_t newmap;
	string_t string;

	convar_s* fps;

	pretype_e precache;
	bool_t precache_once;
}host_s;

extern host_s ghost;
extern game_s ggame;

void host_shutdown();

#include "vidinit.h"
#include "input.h"

#include "image.h"
#include "font.h"

#include "bru_load.h"
#include "entity.h"
#include "world.h"

#include "resource.h"

extern byte gbuffer[IMG_MAX_SIZE * 4];

#endif