#include "game.h"

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

		cment->trace_bbox(pev->base.origin, end, mins, maxs, NULL, CONTENTS_ALL, &trace);
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


static void plr_fly(player_s* pev)
{
	float wishspeed;
	vec3_t wishvel, wishdir;

	vec_scale(wishvel, pev->maxspeed * pev->spd_forward, pev->v_forward);
	vec_ma(wishvel, wishvel, pev->maxspeed * pev->spd_side, pev->v_right);

	vec_copy(wishdir, wishvel);
	wishspeed = vec_normalize(wishdir);

	PM_Accelerate(pev, wishdir, wishspeed, 10);
	if (vec_len(pev->base.velocity) > 0.1f)
	{
		PM_Friction(pev);
		PM_StepSlideMove(pev);
	}
}

static void plr_noclip(player_s* pev)
{
	vec3_t wishvel;

	vec_scale(wishvel, pev->maxspeed * pev->spd_forward, pev->v_forward);
	vec_ma(wishvel, wishvel, pev->maxspeed * pev->spd_side, pev->v_right);
	vec_ma(pev->base.origin, pev->base.origin, cment->frametime, wishvel);
}

static void plr_vieworg(player_s* pev)
{
	vec_copy(pev->vieworg, pev->base.origin);
	pev->vieworg[2] += pev->viewofs;
}

void plr_move(player_s* pev)
{
	pev->maxspeed = 320;
	vec_from_angles(pev->base.angles, pev->v_forward, pev->v_right, pev->v_up);

	if (pev->buttons & IN_FORWARD)		pev->spd_forward = 1;
	else if (pev->buttons & IN_BACK)	pev->spd_forward = -1;
	else								pev->spd_forward = 0;

	if (pev->buttons & IN_RIGHT)		pev->spd_side = 1;
	else if (pev->buttons & IN_LEFT)	pev->spd_side = -1;
	else								pev->spd_side = 0;

	if (glob.noclip->value)
		plr_noclip(pev);
	else
	{
		plr_fly(pev);
	}

	plr_vieworg(pev);
}

void plr_move_cvar()
{
	glob.noclip = cment->con_create_cvar("noclip", FALSE, FALSE);
}