#ifndef _CUBEMENT_H_
#define _CUBEMENT_H_

#include "shared.h"
#include <windows.h>
#include <gl/gl.h>

typedef unsigned short	word;
typedef unsigned char	enum_t;
typedef unsigned int	glbuf_t;
typedef unsigned int	glpic_t;
typedef unsigned int	hash_t;
typedef float			vec4_t[4];
typedef char			name_t[64];
typedef char			constr_t[96];
typedef char			bigstr_t[1024];
typedef char			string_t[MAX_PATH];

#include "util.h"
#include "console.h"

typedef struct
{
	int files;
	int memory;
	int textures;
	int buffers;

	ftime_t time;
	ftime_t frametime;

	name_t newmap;
	string_t string;

	cvar_s* fps;

	bool_t load_as_temp;
	bool_t load_is_allow;
}host_s;

extern host_s ghost;
extern game_s ggame;
extern engine_s gengine;

void host_shutdown();

#include "vidinit.h"
#include "input.h"

#include "image.h"
#include "font.h"

#include "bru_load.h"
#include "world.h"

#endif