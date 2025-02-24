#include "game.h"

global_s glob;
engine_s* cment;
const vec3_t gvec_zeros;

void game_cvar_init()
{
	plr_input_cvar();
	plr_move_cvar();
	plr_hud_cvar();
}

void game_draw_2d()
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

bool_t game_char_events(int ch) 
{
	return FALSE;
}

bool_t game_key_events(int key, bool_t down) 
{
	return FALSE;
}

void game_window_active()
{
	if (cment->world_load && !cment->console_active)
		cment->cursor_show(FALSE);
}

void game_window_inactive()
{
	cment->cursor_show(TRUE);
}

void game_precache()
{
	cment->materials_detail("misc/all.det");
	glob.doorstop = cment->resource_precache("doorstop4.wav");
}

bool_t game_draw_world() 
{
	return TRUE;
}

bool_t game_pause_world() 
{ 
	return cment->console_active;
}

static void state_update()
{
	if (glob.old_console != cment->console_active)
	{
		glob.old_console = cment->console_active;
		cment->cursor_show(cment->console_active);
	}

	if (cment->console_active)
		glob.state = STATE_CONSOLE;
	else
		glob.state = STATE_GAME;
}

void game_start_frame()
{
	state_update();
}

void game_engine_free() 
{

}

void game_start() 
{ 

}

void game_end() 
{

}

void game_draw_3d() 
{

}