#include "cubement.h"

static struct
{
	vec3_t start;
	vec3_t end;

	vec3_t mins;
	vec3_t maxs;

	vec3_t epsmin;
	vec3_t epsmax;

	int brtype;
	bool_t brsign;

	vec3_t bmin;
	vec3_t bmax;

	float fraction;

	bool_t startstuck;
	bool_t endstuck;
}gtrace;

static bool_t trace_brush(brush_s* b)
{
	int i, k, hit;
	bool_t startout, getout;
	float d1, d2, enterfrac, leavefrac, dist;

	enterfrac = -1;
	leavefrac = 1;
	startout = getout = FALSE;

	for (k = 0; k < 6; k++)
	{
		i = k >> 1;
		if (k & 1)
		{
			d1 = gtrace.start[i] - b->maxs[i] - gtrace.maxs[i];
			d2 = gtrace.end[i] - b->maxs[i] - gtrace.maxs[i];
		}
		else
		{
			d1 = -(gtrace.start[i] - b->mins[i] - gtrace.mins[i]);
			d2 = -(gtrace.end[i] - b->mins[i] - gtrace.mins[i]);
		}

		if (d2 > 0)
			getout = TRUE;
		if (d1 > 0)
			startout = TRUE;

		if (d1 > 0 && (d2 >= TRACE_EPSILON || d2 >= d1))
			break;

		if (d1 <= 0 && d2 <= 0)
			continue;

		if (d1 > d2)
		{
			dist = (d1 - TRACE_EPSILON) / (d1 - d2);
			if (dist > enterfrac)
			{
				hit = k;
				enterfrac = dist;
			}
		}
		else
		{
			dist = (d1 + TRACE_EPSILON) / (d1 - d2);
			if (dist < leavefrac)
				leavefrac = dist;
		}
	}

	if (!startout) 
	{
		gtrace.startstuck = TRUE;
		if (!getout) 
		{
			gtrace.endstuck = TRUE;
			gtrace.fraction = 0;
		}
		return FALSE;
	}

	if (enterfrac < leavefrac && enterfrac > -1 && enterfrac < gtrace.fraction)
	{
		vec3_t pt;

		if (enterfrac < 0)
			enterfrac = 0;

		gtrace.fraction = enterfrac;
		vec_lerp(pt, gtrace.fraction, gtrace.start, gtrace.end);
		vec_maxmin(gtrace.start, pt, gtrace.epsmin, gtrace.epsmax, gtrace.bmin, gtrace.bmax);
		gtrace.brtype = hit >> 1;
		gtrace.brsign = !(hit & 1);
		return TRUE;
	}

	return FALSE;
}

void trace(const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs, const entity_s* ignore, int contents, trace_s* tr)
{
	area_s* a;
	surf_s* s;
	entity_s* e;
	edict_s* ed;
	vec3_t offset;
	int i, j, brnum;
	resource_s* res;
	brushmodel_s* bm;
	brush_s* b, bfake;

	if (!gentworld)
		return;

	brnum = -1;
	gtrace.brtype = 0;
	gtrace.brsign = FALSE;
	gtrace.fraction = 1;
	gtrace.endstuck = FALSE;
	gtrace.startstuck = FALSE;
	vec_copy(gtrace.end, end);
	vec_copy(gtrace.start, start);
	vec_copy(gtrace.mins, mins);
	vec_copy(gtrace.maxs, maxs);
	vec_add_val(gtrace.epsmin, mins, -TRACE_EPSILON);
	vec_add_val(gtrace.epsmax, maxs, TRACE_EPSILON);
	vec_maxmin(start, end, gtrace.epsmin, gtrace.epsmax, gtrace.bmin, gtrace.bmax);

	tr->dist = 0;
	vec_init(tr->color, 0);
	vec_init(tr->normal, 0);
	strcpyn(tr->texturename, "NULL");

	gworld.framecount++;

	for (i = 0; !gtrace.endstuck && i < gbru.num_areas; i++)
	{
		a = gbru.areas + i;
		for (j = 0; j < a->num_boxes; j++)
		{
			if (!vec2_aabb(a->mins[j], a->maxs[j], start, start))
			{
				a->framenum = gworld.framecount;
				break;
			}

			if (!a->activecount || vec2_aabb(a->mins[j], a->maxs[j], gtrace.bmin, gtrace.bmax))
				continue;

			a->framenum = gworld.framecount;
			break;
		}

		if (a->framenum != gworld.framecount)
			continue;

		if (!(gentworld->contents & contents))
			continue;

		for (j = 0; !gtrace.endstuck && j < a->num_brushareas; j++)
		{
			b = gbru.brushes + a->brushareas[j];
			if (b->framenum == gworld.framecount)
				continue;

			b->framenum = gworld.framecount;
			if (vec_aabb(b->mins, b->maxs, gtrace.bmin, gtrace.bmax))
				continue;
			
			if (trace_brush(b))
				brnum = a->brushareas[j];
		}
	}

	for (i = 1; !gtrace.endstuck && i < gnuments; i++)
	{
		ed = gedicts + i;
		if (!ed->e)
			continue;

		e = ed->e;
		if (!(e->contents & contents) || e->model == BAD_HANDLE)
			continue;

		res = gres + e->model;
		switch (res->type)
		{
		case RES_BRUSH:
			bm = res->data.brush;
			vec_sub(offset, e->origin, bm->origin);
			for (j = 0; j < bm->num_brushes; j++)
			{
				b = bm->brushes + j;
				vec_add(bfake.mins, offset, b->mins);
				vec_add(bfake.maxs, offset, b->maxs);
				if (vec_aabb(bfake.mins, bfake.maxs, gtrace.bmin, gtrace.bmax))
					continue;

				if (trace_brush(&bfake))
					brnum = bm->start_brush + j;
			}
			break;
		}
	}

	tr->fraction = gtrace.fraction;
	vec_lerp(tr->endpos, tr->fraction, start, end);
	tr->endstuck = gtrace.endstuck;
	tr->startstuck = gtrace.startstuck;

	if (brnum != -1)
	{
		vec3_t dir;

		vec_sub(dir, end, start);
		vec_normalize_fast(dir);
		vec_mul(dir, TRACE_EPSILON);
		vec_sub(tr->endpos, tr->endpos, dir);

		b = gbru.brushes + brnum;
		for (i = 0; i < b->num_surfes; i++)
		{
			s = b->surfes + i;
			if (s->itype != gtrace.brtype || s->sign != gtrace.brsign)
				continue;

			vec_copy(tr->color, s->color);
			strcpyn(tr->texturename, gbru.textures[s->texture].name);
			break;
		}

		if (gtrace.brsign)
		{
			tr->dist = b->mins[gtrace.brtype];
			tr->normal[gtrace.brtype] = -1;
		}
		else
		{
			tr->dist = b->maxs[gtrace.brtype];
			tr->normal[gtrace.brtype] = 1;
		}
	}
}