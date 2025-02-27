#ifndef _GAME_H_
#define _GAME_H_

#include "../engine/shared.h"

typedef enum
{
	STATE_CONSOLE,
	STATE_MENU,
	STATE_LOADING,
	STATE_GAME,
}state_e;

typedef struct
{
	state_e state;
	bool_t old_console;

	ihandle_t confont;

	ihandle_t doorstop;

	cvar_s* sens;
	cvar_s* noclip;
	cvar_s* fov;
}global_s;

extern global_s glob;

#include "entity.h"
#include "player.h"
#include "world.h"

#endif