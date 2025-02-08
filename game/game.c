#include "game.h"
#include <windows.h>

#define WINDOW_NAME		"Cubement"

global_s glob;
engine_s* cment;

static void engine_init()
{
	REGISTER_ENTITY(func_wall);
	REGISTER_ENTITY(player);
	REGISTER_ENTITY(worldspawn);

	glob.confont = cment->get_resource_handle("console.fnt");
	glob.cat1 = cment->precache_resource("pics/1.jpg");
	glob.cat2 = cment->precache_resource("pics/2.png");
}

static void draw_2d()
{
	//cment->pic_draw(glob.cat1, 256, 256, RENDER_NORMAL, 255, 170, 30, 255, NULL);
	//cment->pic_draw(glob.cat2, 128, 128, RENDER_ALPHA, 255, 255, 255, 127, NULL);
}

bool_t char_events(int ch) { return FALSE; }
bool_t key_events(int key, bool_t down) { return FALSE; }

static void window_active() { }
static void window_inactive() {}

static bool_t draw_world() { return TRUE; }

void after_draw_3d() { }

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	game_s g;

	g.windowname = WINDOW_NAME;

	g.engine_init = engine_init;

	g.char_events = char_events;
	g.key_events = key_events;

	g.window_active = window_active;
	g.window_inactive = window_inactive;

	g.draw_2d = draw_2d;
	g.draw_3d = after_draw_3d;
	g.draw_world = draw_world;

	cubement(&cment, &g);
	return 1;
}