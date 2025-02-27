#include "game.h"

static void ent_angular_move(entity_s* pev, float friction)
{
	int	i;
	float adjustment;

	vec_ma(pev->angles, pev->angles, cment->frametime, pev->avelocity);
	if (friction == 0.0f)
		return;

	adjustment = cment->frametime * fabs(friction) * 40;
	for (i = 0; i < 3; i++)
	{
		if (pev->avelocity[i] > 0.0f)
		{
			pev->avelocity[i] -= adjustment;
			if (pev->avelocity[i] < 0.0f)
				pev->avelocity[i] = 0.0f;
		}
		else
		{
			pev->avelocity[i] += adjustment;
			if (pev->avelocity[i] > 0.0f)
				pev->avelocity[i] = 0.0f;
		}
	}
}

static void ent_linear_move(entity_s* pev, float friction)
{
	int	i;
	float adjustment;

	vec_ma(pev->origin, pev->origin, cment->frametime, pev->velocity);
	if (friction == 0.0f)
		return;

	adjustment = cment->frametime * fabs(friction) * 40;
	for (i = 0; i < 3; i++)
	{
		if (pev->velocity[i] > 0.0f)
		{
			pev->velocity[i] -= adjustment;
			if (pev->velocity[i] < 0.0f)
				pev->velocity[i] = 0.0f;
		}
		else
		{
			pev->velocity[i] += adjustment;
			if (pev->velocity[i] > 0.0f)
				pev->velocity[i] = 0.0f;
		}
	}
}

entity_s* ent_rotate(entity_s* pev, float friction)
{
	float dist;
	trace_s tr;
	entity_s* ent;
	int i, oldcont;
	vec3_t oldang, oldorg, dest, dir1, dir2, bmin, bmax, start;

	vec_copy(oldang, pev->angles);
	ent_angular_move(pev, friction);

	vec_copy(oldorg, pev->origin);
	ent_linear_move(pev, friction);

	if (pev->contents == CONTENTS_EMPTY)
		return NULL;

	vec_set(dir1, oldorg[0] + (pev->maxs[0] + pev->mins[0]) * 0.5f, oldorg[1] + (pev->maxs[1] + pev->mins[1]) * 0.5f, oldorg[2] + (pev->maxs[2] + pev->mins[2]) * 0.5f);
	cment->ent_rotate_brush_bbox(pev, bmin, bmax);
	vec_set(dir2, pev->origin[0] + (bmax[0] + bmin[0]) * 0.5f, pev->origin[1] + (bmax[1] + bmin[1]) * 0.5f, pev->origin[2] + (bmax[2] + bmin[2]) * 0.5f);
	vec_sub(dir2, dir2, dir1);
	dist = vec_normalize(dir2) * 2;
	oldcont = pev->contents;

	for (i = 1; i < cment->entities_max; i++)
	{
		ent = cment->entities[i];
		if (!ent || ent == pev || ent->id == ENTID_FREE)
			continue;

		if (!(ent->contents & CONTENTS_SOLID))
			continue;

		if (!cment->trace_test_stuck_ent(pev, ent->origin, ent->mins, ent->maxs))
			continue;

		//vec_sub(dir1, pev->origin, ent->origin);
		//vec_normalize_fast(dir1);
		//if (vec_dot(dir2, dir1) < 0)
		//	continue;

		vec_ma(dest, ent->origin, dist, dir2);

		pev->contents = CONTENTS_EMPTY;
		cment->trace_bbox(ent->origin, dest, ent->mins, ent->maxs, ent, CONTENTS_SOLID, &tr);
		pev->contents = oldcont;

		if (!tr.endstuck)
		{
			vec_copy(ent->origin, tr.endpos);//TODO: need player relink?
			if (!cment->trace_test_stuck_ent(pev, ent->origin, ent->mins, ent->maxs))
				continue;
		}

		vec_copy(pev->origin, oldorg);
		vec_copy(pev->angles, oldang);
		return ent;
	}

	return NULL;
}

entity_s* ent_find_by_id(entid_e id)
{
	int i;
	entity_s* e;

	for (i = 0; i < cment->entities_max; i++)
	{
		e = cment->entities[i];
		if (!e || e->id == ENTID_FREE)
			continue;

		if (e->id == id)
			return e;
	}

	return NULL;
}