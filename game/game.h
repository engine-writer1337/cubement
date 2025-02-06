#ifndef _GAME_H_
#define _GAME_H_

#include "../engine/shared.h"

#include "game.h"
#include "entity.h"

typedef struct
{
	ihandle_t confont;

	ihandle_t cat1;
	ihandle_t cat2;
}global_s;

extern global_s glob;

#endif