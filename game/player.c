#include "game.h"

player_s* gplayer;

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
	cment->con_create_cmd("+forward", forward_down);
	cment->con_create_cmd("-forward", forward_up);
	cment->con_create_cmd("+back", back_down);
	cment->con_create_cmd("-back", back_up);
	cment->con_create_cmd("+left", left_down);
	cment->con_create_cmd("-left", left_up);
	cment->con_create_cmd("+right", right_down);
	cment->con_create_cmd("-right", right_up);

	glob.sens = cment->con_create_cvar("m_sens", 3, TRUE);
	glob.noclip = cment->con_create_cvar("noclip", FALSE, FALSE);
}

static bool_t spawn_player(player_s* pev)
{
	entity_s* spawn;

	gplayer = pev;
	spawn = ent_find_by_id(ENTID_START);
	if (spawn)
	{
		vec_copy(pev->base.origin, spawn->origin);
		pev->base.angles[YAW] = spawn->angles[YAW];
	}

	return TRUE;
}

static void precache_player(player_s* pev)
{

}

static void keyvalue_player(player_s* pev, keyvalue_s* kv)
{

}

#define STOP_EPSILON 0.1
void PM_ClipVelocity(vec3_t in, vec3_t normal, vec3_t out, float overbounce)
{
	float backoff;
	float change;
	int i;

	backoff = vec_dot(in, normal) * overbounce;
	for (i = 0; i < 3; i++)
	{
		change = normal[i] * backoff;
		out[i] = in[i] - change;
		if ((out[i] > -STOP_EPSILON) && (out[i] < STOP_EPSILON))
			out[i] = 0;
	}
}

void PM_StepSlideMove(player_s* pev)
{
	int bumpcount;
	int numplanes;
	vec3_t planes[5];
	vec3_t primal_velocity;
	int i, j;
	trace_s trace;
	vec3_t end;
	float time_left;
	vec3_t mins = { -16, -16, -32 }, maxs = { 16, 16, 32 };
	//vec3_t mins = { 0, 0, 0 }, maxs = { 0, 0, 0 };

	vec_copy(primal_velocity, pev->base.velocity);
	numplanes = 0;
	time_left = cment->frametime;

	for (bumpcount = 0; bumpcount < 3; bumpcount++)
	{
		for (i = 0; i < 3; i++)
		{
			end[i] = pev->base.origin[i] + time_left * pev->base.velocity[i];
		}

		cment->trace(pev->base.origin, end, mins, maxs, NULL, CONTENTS_ALL, &trace);
		if (trace.endstuck)
		{
			vec_clear(pev->base.velocity);
			return;
		}
		if (trace.fraction > 0)
		{
			vec_copy(pev->base.origin, trace.endpos);
			numplanes = 0;
		}

		if (trace.fraction == 1)
			break; 

		time_left -= time_left * trace.fraction;//TODO: ?
		if (numplanes >= 5)
		{
			vec_clear(pev->base.velocity);
			break;
		}

		//cment->con_printf(COLOR_WHITE, "%s", trace.texturename);
		vec_copy(planes[numplanes], trace.normal);
		numplanes++;

		for (i = 0; i < numplanes; i++)
		{
			PM_ClipVelocity(pev->base.velocity, planes[i], pev->base.velocity, 1.01f);
			//cment->con_printf(COLOR_WHITE, "%i %f %f %f", bumpcount, pev->base.velocity[0], pev->base.velocity[1], pev->base.velocity[2]);
			for (j = 0; j < numplanes; j++)
			{
				if (j != i)
				{
					if (vec_dot(pev->base.velocity, planes[j]) < 0)
						break;
				}
			}

			if (j == numplanes)
				break;
		}

		if (vec_dot(pev->base.velocity, primal_velocity) <= 0)
		{
			vec_clear(pev->base.velocity);
			break;
		}
	}
}

void PM_Accelerate(player_s* pev, vec3_t wishdir, float wishspeed, float accel)
{
	int i;
	float addspeed, accelspeed, currentspeed;

	currentspeed = vec_dot(pev->base.velocity, wishdir);
	addspeed = wishspeed - currentspeed;

	if (addspeed <= 0)
	{
		return;
	}

	accelspeed = accel * cment->frametime * wishspeed;

	if (accelspeed > addspeed)
	{
		accelspeed = addspeed;
	}

	for (i = 0; i < 3; i++)
	{
		pev->base.velocity[i] += accelspeed * wishdir[i];
	}
}

void PM_Friction(player_s* pev)
{
	float* vel;
	float speed, newspeed, control;
	float friction;
	float drop;

	vel = pev->base.velocity;
	speed = (float)sqrt(vel[0] * vel[0] + vel[1] * vel[1] + vel[2] * vel[2]);
	if (speed < 1)
	{
		vel[0] = 0;
		vel[1] = 0;
		return;
	}

	drop = 0;
	friction = 4;
	control = speed < 100 ? 100 : speed;
	drop += control * friction * cment->frametime;

	newspeed = speed - drop;
	if (newspeed < 0)
		newspeed = 0;

	newspeed /= speed;
	vel[0] = vel[0] * newspeed;
	vel[1] = vel[1] * newspeed;
	vel[2] = vel[2] * newspeed;
}

static void think_player(player_s* pev)
{
	vec3_t wishdir, wishvel;
	float spd_side, spd_forward, wishspeed;

	if (!cment->console_active)
	{
		int mx, my;

		cment->cursor_get_pos(&mx, &my, FALSE);
		mx -= cment->centr_x;
		my -= cment->centr_y;
		if (mx || my)
		{
			pev->base.angles[YAW] -= 0.01f * glob.sens->value * mx;
			pev->base.angles[YAW] = anglemod(pev->base.angles[YAW]);
			pev->base.angles[PITCH] += 0.01f * glob.sens->value * my;
			pev->base.angles[PITCH] = clamp(-89, pev->base.angles[PITCH], 89);
			cment->cursor_reset_pos();
		}
	}

	pev->buttons = plr_key_bits();
	vec_from_angles(pev->base.angles, pev->v_forward, pev->v_right, pev->v_up);

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

	vec_scale(wishvel, spd_forward, pev->v_forward);
	vec_ma(wishvel, wishvel, spd_side, pev->v_right);

	if (glob.noclip->value)
		vec_ma(pev->base.origin, pev->base.origin, cment->frametime, wishvel);
	else
	{
		vec_copy(wishdir, wishvel);
		wishspeed = vec_normalize(wishdir);

		PM_Accelerate(pev, wishdir, wishspeed, 10);
		if (vec_len(pev->base.velocity) > 0.1f)
		{
			PM_Friction(pev);
			PM_StepSlideMove(pev);
		}
	}

	cment->set_view_org(pev->base.origin);
	cment->set_view_ang(pev->base.angles);
	cment->set_view_fov(90);
}

static void saverestore_player(player_s* pev) {}

LINK_ENTITY(player, ENTID_PLAYER, sizeof(player_s))

static bool_t spawn_info_player_start(entity_s* pev) { return TRUE; }
static void precache_info_player_start(entity_s* pev) {}
static void keyvalue_info_player_start(entity_s* pev, keyvalue_s* kv)
{
	if (strcmpi(kv->key, "origin"))
		vec_from_str(pev->origin, kv->value);
	else if (strcmpi(kv->key, "angles"))
		vec_from_str(pev->angles, kv->value);
}

static void saverestore_info_player_start(entity_s* pev) {}
static void think_info_player_start(entity_s* pev) { }

LINK_ENTITY(info_player_start, ENTID_START, sizeof(entity_s))