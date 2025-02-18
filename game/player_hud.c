#include "game.h"

void plr_hud_cvar()
{
	glob.fov = cment->con_create_cvar("r_fov", 90, TRUE);
}