#include "game.h"
#include <windows.h>

#define WINDOW_NAME		"Cubement"

global_s glob;
engine_s* cment;

static void cvar_init()
{

}

static void font_init()
{
	glob.confont = cment->precache_font("console");
}

static void entity_register()
{
	REGISTER_ENTITY(func_wall);
	REGISTER_ENTITY(player);
	REGISTER_ENTITY(worldspawn);
}

static void after_engine_init()
{
	glob.cat1 = cment->precache_pic("pics/1.jpg", 0);
	glob.cat2 = cment->precache_pic("pics/2.png", 0);
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

void game_once_precache() { }
void game_before_init() { }
void game_after_init() { }
void game_disconnect() { }
void before_draw_3d()
{ 
	cment->set_view_fov(90);
}

void after_draw_3d() { }

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	game_s g;

	g.windowname = WINDOW_NAME;

	g.cvar_init = cvar_init;
	g.font_init = font_init;
	g.after_engine_init = after_engine_init;
	g.entity_register = entity_register;

	g.char_events = char_events;
	g.key_events = key_events;

	g.window_active = window_active;
	g.window_inactive = window_inactive;

	g.draw_2d = draw_2d;
	g.draw_world = draw_world;

	g.game_once_precache = game_once_precache;

	g.game_before_init = game_before_init;
	g.game_after_init = game_after_init;
	g.game_disconnect = game_disconnect;
	g.before_draw_3d = before_draw_3d;
	g.after_draw_3d = after_draw_3d;

	cubement(&cment, &g);
	return 1;
}