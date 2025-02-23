#include "game.h"
#include <windows.h>

#define WINDOW_NAME		"Cubement"

static const matmap_s gmatmap[] =
{
	{ "$conc", MAT_DEFAULT },
	{ "$dirt", MAT_DIRT },
	{ "$metal", MAT_METAL },
};

static void game_engine_init()
{
	REGISTER_ENTITY(func_wall);
	REGISTER_ENTITY(player);
	REGISTER_ENTITY(worldspawn);
	REGISTER_ENTITY(info_player_start);
	REGISTER_ENTITY(func_door_rotating);

	cment->materials_register(gmatmap, ARRAYSIZE(gmatmap));

	glob.confont = cment->resource_get_handle("console.fnt");
}

void game_cvar_init();
void game_draw_2d();
bool_t game_char_events(int ch);
bool_t game_key_events(int key, bool_t down);
void game_window_active();
void game_window_inactive();
void game_precache();
bool_t game_draw_world();
bool_t game_pause_world();
void game_start_frame();
void game_engine_free();
void game_start();
void game_end();
void game_draw_3d();

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	game_s g;

	g.windowname = WINDOW_NAME;
	g.cvar_init = game_cvar_init;
	g.engine_init = game_engine_init;
	g.engine_free = game_engine_free;
	g.start_frame = game_start_frame;
	g.game_precache = game_precache;
	g.game_start = game_start;
	g.game_end = game_end;
	g.draw_2d = game_draw_2d;
	g.draw_3d = game_draw_3d;
	g.window_active = game_window_active;
	g.window_inactive = game_window_inactive;
	g.pause_world = game_pause_world;
	g.draw_world = game_draw_world;
	g.char_events = game_char_events;
	g.key_events = game_key_events;
	cubement(&cment, &g);
	return 1;
}