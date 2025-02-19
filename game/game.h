#ifndef _GAME_H_
#define _GAME_H_

#include "../engine/shared.h"

typedef struct
{
	ihandle_t confont;

	ihandle_t doorstop;

	cvar_s* sens;
	cvar_s* noclip;
	cvar_s* fov;

	bool_t old_console;
}global_s;

extern global_s glob;
extern const vec3_t gvec_zeros;

#include "entity.h"
#include "player.h"
#include "world.h"

#endif