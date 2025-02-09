#include "game.h"

entity_s* gplayer;

static kbutton_s in_left;
static kbutton_s in_right;
static kbutton_s in_forward;
static kbutton_s in_back;

static void plr_key_down(kbutton_s* b, const char* arg1)
{
	int k;

	if (arg1[0])
		k = atoi(arg1);
	else
		k = -1;

	if (k == b->down[0] || k == b->down[1])
		return;

	if (!b->down[0])
		b->down[0] = k;
	else if (!b->down[1])
		b->down[1] = k;
	else
		return;

	if (b->state & 1)
		return;

	b->state |= 3;
}

static void plr_key_up(kbutton_s* b, const char* arg1)
{
	int k;

	if (arg1[0])
		k = atoi(arg1);
	else
	{
		b->down[0] = b->down[1] = 0;
		b->state = 4;
		return;
	}

	if (b->down[0] == k)
		b->down[0] = 0;
	else if (b->down[1] == k)
		b->down[1] = 0;
	else
		return;

	if (b->down[0] || b->down[1] || !(b->state & 1))
		return;

	b->state &= ~1;
	b->state |= 4;
}

static int plr_key_bits()
{
	int bits = 0;

	if (in_forward.state & 3)
		bits |= IN_FORWARD;
	if (in_back.state & 3)
		bits |= IN_BACK;
	if (in_left.state & 3)
		bits |= IN_LEFT;
	if (in_right.state & 3)
		bits |= IN_RIGHT;

	in_forward.state &= ~2;
	in_back.state &= ~2;
	in_left.state &= ~2;
	in_right.state &= ~2;
	return bits;
}

#define cmd_key_up(name, kbtn)		void name(const char* arg1, const char* arg2) { plr_key_up(kbtn, arg1); }
#define cmd_key_down(name, kbtn)	void name(const char* arg1, const char* arg2) { plr_key_down(kbtn, arg1); }

cmd_key_up(forward_up, &in_forward);
cmd_key_down(forward_down, &in_forward);
cmd_key_up(back_up, &in_back);
cmd_key_down(back_down, &in_back);
cmd_key_up(left_up, &in_left);
cmd_key_down(left_down, &in_left);
cmd_key_up(right_up, &in_right);
cmd_key_down(right_down, &in_right);

void cvar_init()
{
	cment->create_cmd("+forward", forward_down);
	cment->create_cmd("-forward", forward_up);
	cment->create_cmd("+back", back_down);
	cment->create_cmd("-back", back_up);
	cment->create_cmd("+left", left_down);
	cment->create_cmd("-left", left_up);
	cment->create_cmd("+right", right_down);
	cment->create_cmd("-right", right_up);

	glob.sens = cment->create_cvar("m_sens", 3, TRUE);
}

static bool_t spawn_player(entity_s* self)
{
	gplayer = self;
	return TRUE;
}

static void precache_player(entity_s* self)
{

}

static void keyvalue_player(entity_s* self, keyvalue_s* kv)
{

}

static void think_player(entity_s* self) 
{
	float spd_side, spd_forward;
	player_s* pev = (player_s*)self->pev;

	if (cment->world_load && !cment->console_active)
	{
		int mx, my;

		cment->get_cursor_pos(&mx, &my, FALSE);
		mx -= cment->centr_x;
		my -= cment->centr_y;
		if (mx || my)
		{
			self->angles[YAW] -= 0.01f * glob.sens->value * mx;
			self->angles[YAW] = anglemod(self->angles[YAW]);
			self->angles[PITCH] += 0.01f * glob.sens->value * my;
			self->angles[PITCH] = clamp(-89, self->angles[PITCH], 89);
			cment->reset_cursor_pos();
		}
	}

	pev->buttons = plr_key_bits();
	vec_from_angles(self->angles, pev->v_forward, pev->v_right, pev->v_up);

	if (pev->buttons & IN_FORWARD)
		spd_forward = 400;
	else if (pev->buttons & IN_BACK)
		spd_forward = -400;
	else
		spd_forward = 0;

	if (pev->buttons & IN_RIGHT)
		spd_side = 400;
	else if (pev->buttons & IN_LEFT)
		spd_side = -400;
	else
		spd_side = 0;

	vec_scale(self->velocity, spd_forward, pev->v_forward);
	vec_ma(self->velocity, self->velocity, spd_side, pev->v_right);
	vec_ma(self->origin, self->origin, cment->frametime, self->velocity);

	cment->set_view_org(self->origin);
	cment->set_view_ang(self->angles);
	cment->set_view_fov(90);
}

static void saverestore_player(player_s* pev) {}

LINK_ENTITY(player, ENTID_PLAYER, sizeof(player_s))

static bool_t spawn_worldspawn(entity_s* self) { gplayer = NULL; return TRUE; }
static void precache_worldspawn(entity_s* self) { }
static void keyvalue_worldspawn(entity_s* self, keyvalue_s* kv) { }
static void saverestore_worldspawn(entbase_s* pev) {}
static void think_worldspawn(entity_s* self) { }
LINK_ENTITY(worldspawn, ENTID_WORLDSPAWN, sizeof(entbase_s))