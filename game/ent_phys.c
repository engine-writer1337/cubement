#include "game.h"

void ent_angular_move(entity_s* pev, float friction)
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

entity_s* ent_rotate(entity_s* pev, float friction)
{
	int i;
	vec3_t oldang;
	entity_s* ent;

	vec_copy(oldang, pev->angles);
	ent_angular_move(pev, friction);

	if (pev->contents == CONTENTS_EMPTY)
		return NULL;

	for (i = 1; i < cment->entities_max; i++)
	{
		ent = cment->entities[i];
		if (!ent || ent->id == ENTID_FREE)
			continue;

		if (!(ent->contents & CONTENTS_SOLID))
			continue;

		if (!cment->trace_test_stuck_ent(pev, ent->origin, ent->mins, ent->maxs))
			continue;
		//TODO: float point inaccuracy bug
		vec_copy(pev->angles, oldang);
		return ent;
	}

	return NULL;
}