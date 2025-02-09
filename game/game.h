#ifndef _GAME_H_
#define _GAME_H_

#include "../engine/shared.h"

typedef struct
{
	ihandle_t confont;

	ihandle_t cat1;
	ihandle_t cat2;

	cvar_s* sens;
}global_s;

extern global_s glob;

#include "entity.h"
#include "player.h"

#endif