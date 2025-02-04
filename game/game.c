#include "game.h"
#include <windows.h>

engine_s* cment;

static void cvar_init()
{

}

static void font_init()
{

}

static void entity_register()
{
	REGISTER_ENTITY(func_wall);
}

static void game_precache()
{

}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	game_s g;

	g.pev_size = sizeof(entvar_s);

	g.cvar_init = cvar_init;
	g.font_init = font_init;
	g.after_engine_init = game_precache;
	g.entity_register = entity_register;

	Cubement(&cment, &g);
	return 1;
}