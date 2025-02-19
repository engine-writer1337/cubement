#ifndef _PLAYER_H_
#define _PLAYER_H_

#define IN_FORWARD	(1 << 0)
#define IN_BACK		(1 << 1)
#define IN_LEFT		(1 << 2)
#define IN_RIGHT	(1 << 3)
#define IN_USE		(1 << 4)

#define PLR_VIEW_OFS	24
#define PLR_MAXSPEED	320

typedef struct
{
	entity_s base;

	int buttons;
	vec3_t v_forward, v_right, v_up;

	float maxspeed;
	float spd_forward, spd_side;

	float viewofs;
	vec3_t vieworg;

	ftime_t nextuse;
}player_s;

extern player_s* gplayer;

void plr_inputs(player_s* pev);
void plr_input_cvar();

void plr_move(player_s* pev);
void plr_move_cvar();

void plr_hud_cvar();

#endif 