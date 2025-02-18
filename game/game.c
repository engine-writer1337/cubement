#include "game.h"
#include <windows.h>

global_s glob;
engine_s* cment;

#define WINDOW_NAME		"Cubement"

static const matmap_s gmatmap[] =
{
	{ "$conc", MAT_DEFAULT },
	{ "$dirt", MAT_DIRT },
	{ "$metal", MAT_METAL },
};

static void engine_init()
{
	REGISTER_ENTITY(func_wall);
	REGISTER_ENTITY(player);
	REGISTER_ENTITY(worldspawn);
	REGISTER_ENTITY(info_player_start);
	REGISTER_ENTITY(func_door_rotating);

	cment->materials_register(gmatmap, ARRAYSIZE(gmatmap));

	glob.confont = cment->resource_get_handle("console.fnt");
}

static void cvar_init()
{
	plr_input_cvar();
	plr_move_cvar();
	plr_hud_cvar();
}

static void draw_2d()
{
	if (gplayer)
	{
		char str[256];

		sprintf(str, "org: %.2f %.2f %.2f", gplayer->base.origin[0], gplayer->base.origin[1], gplayer->base.origin[2]);
		cment->font_print(glob.confont, str, 0, 64, RENDER_TRANSPARENT, 255, 255, 255, 255);

		sprintf(str, "ang: %.2f %.2f %.2f", gplayer->base.angles[0], gplayer->base.angles[1], gplayer->base.angles[2]);
		cment->font_print(glob.confont, str, 0, 96, RENDER_TRANSPARENT, 255, 255, 255, 255);

		sprintf(str, "vel: %.2f %.2f %.2f", gplayer->base.velocity[0], gplayer->base.velocity[1], gplayer->base.velocity[2]);
		cment->font_print(glob.confont, str, 0, 128, RENDER_TRANSPARENT, 255, 255, 255, 255);
	}

	//cment->pic_draw(glob.cat1, 256, 256, RENDER_NORMAL, 255, 170, 30, 255, NULL);
	//cment->pic_draw(glob.cat2, 128, 128, RENDER_ALPHA, 255, 255, 255, 127, NULL);
}

bool_t char_events(int ch) { return FALSE; }
bool_t key_events(int key, bool_t down) { return FALSE; }

static void window_active() 
{
	if (cment->world_load && !cment->console_active)
		cment->cursor_show(FALSE);
}

static void window_inactive() 
{
	cment->cursor_show(TRUE);
}

static void game_precache() {}

static bool_t draw_world() { return TRUE; }

static bool_t pause_world() { return cment->console_active; }

void after_draw_3d() { }

void start_frame() 
{
	if (glob.old_console != cment->console_active)
	{//TODO: think something better
		glob.old_console = cment->console_active;
		cment->cursor_show(cment->console_active);
	}
}

void engine_free() { }
void game_start() { }
void game_end() { }
void draw_3d() { }

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	game_s g;

	g.windowname = WINDOW_NAME;
	g.cvar_init = cvar_init;
	g.engine_init = engine_init;
	g.engine_free = engine_free;
	g.start_frame = start_frame;
	g.game_precache = game_precache;
	g.game_start = game_start;
	g.game_end = game_end;
	g.draw_2d = draw_2d;
	g.draw_3d = draw_3d;
	g.window_active = window_active;
	g.window_inactive = window_inactive;
	g.pause_world = pause_world;
	g.draw_world = draw_world;
	g.char_events = char_events;
	g.key_events = key_events;
	cubement(&cment, &g);
	return 1;
}