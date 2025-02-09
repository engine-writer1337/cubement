#ifndef _PLAYER_H_
#define _PLAYER_H_

#define IN_FORWARD	(1 << 0)
#define IN_BACK		(1 << 1)
#define IN_LEFT		(1 << 2)
#define IN_RIGHT	(1 << 3)

typedef struct
{
	int state;
	int down[2];
}kbutton_s;

typedef struct
{
	entbase_s base;

	int buttons;

	vec3_t v_forward, v_right, v_up;
}player_s;

extern entity_s* gplayer;

void cvar_init();

#endif 