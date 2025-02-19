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
	float d1, d2, enterfrac, leavefrac, frac;

	enterfrac = -1;
	leavefrac = 1;
	startout = getout = FALSE;

	for (k = 0; k < 6; k++)
	{
		i = k >> 1;
		if (k & 1)
		{
			d1 = gtrace.start[i] - b->absmax[i] - gtrace.maxs[i];
			d2 = gtrace.end[i] - b->absmax[i] - gtrace.maxs[i];
		}
		else
		{
			d1 = -(gtrace.start[i] - b->absmin[i] - gtrace.mins[i]);
			d2 = -(gtrace.end[i] - b->absmin[i] - gtrace.mins[i]);
		}

		if (d2 > 0)
			getout = TRUE;
		if (d1 > 0)
			startout = TRUE;

		if (d1 > 0 && (d2 >= TRACE_EPSILON || d2 >= d1))
			return FALSE;

		if (d1 <= 0 && d2 <= 0)
			continue;

		if (d1 > d2)
		{
			frac = (d1 - TRACE_EPSILON) / (d1 - d2);
			if (frac > enterfrac)
			{
				hit = k;
				enterfrac = frac;
			}
		}
		else
		{
			frac = (d1 + TRACE_EPSILON) / (d1 - d2);
			if (frac < leavefrac)
				leavefrac = frac;
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
	entity_s* touch;
	int i, j, brnum;
	brushmodel_s* bm;
	brush_s* b, bfake;
	bool_t anyintersect, rotate;
	vec3_t movevec, absmin, absmax;

	if (!gentworld)
		return;

	brnum = -1;
	touch = NULL;
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
	vec_add(absmin, start, gtrace.epsmin);
	vec_add(absmax, start, gtrace.epsmax);
	vec_maxmin(start, end, gtrace.epsmin, gtrace.epsmax, gtrace.bmin, gtrace.bmax);

	tr->dist = 0;
	vec_init(tr->color, 0);
	vec_init(tr->normal, 0);
	tr->material = MAT_DEFAULT;

	gworld.framecount++;

	for (i = 0; !gtrace.endstuck && i < gbru.num_areas; i++)
	{
		a = gbru.areas + i;
		for (j = 0; j < a->num_boxes; j++)
		{
			if (!vec2_aabb(a->absmin[j], a->absmax[j], absmin, absmax))
			{
				a->framenum = gworld.framecount;
				break;
			}

			if (!a->activecount || vec2_aabb(a->absmin[j], a->absmax[j], gtrace.bmin, gtrace.bmax))
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
			if (vec_aabb(b->absmin, b->absmax, gtrace.bmin, gtrace.bmax))
				continue;
			
			if (!trace_brush(b))
				continue;

			touch = gentworld;
			brnum = a->brushareas[j];
		}
	}

	for (i = 1; !gtrace.endstuck && i < gnuments; i++)
	{
		ed = gedicts + i;
		if (!ed->e)
			continue;

		e = ed->e;
		if (ignore == e || !(e->contents & contents) || e->model == BAD_HANDLE)
			continue;

		anyintersect = FALSE;
		for (j = 0; j < ed->num_areas; j++)
		{
			if (gbru.areas[ed->areas[j]].framenum != gworld.framecount)
				continue;

			anyintersect = TRUE;
			break;
		}

		if (!anyintersect)
			continue;

		switch (gres[e->model].type)
		{
		case RES_BRUSH:
			bm = gres[e->model].data.brush;
			rotate = !vec_is_zero(e->angles);
			vec_sub(movevec, e->origin, bm->origin);

			for (j = 0; j < bm->num_brushes; j++)
			{
				b = bm->brushes + j;
				vec_add(bfake.absmin, movevec, b->absmin);
				vec_add(bfake.absmax, movevec, b->absmax);

				if (rotate)
					vec_rotate_org_bbox(e->angles, e->origin, bm->offset, bfake.absmin, bfake.absmax);

				if (vec_aabb(bfake.absmin, bfake.absmax, gtrace.bmin, gtrace.bmax))
					continue;

				if (!trace_brush(&bfake))
					continue;

				touch = e;
				brnum = bm->start_brush + j;
			}
			break;
		}
	}

	tr->fraction = gtrace.fraction;
	vec_lerp(tr->endpos, tr->fraction, start, end);
	tr->endstuck = gtrace.endstuck;
	tr->startstuck = gtrace.startstuck;
	tr->ent = touch;

	if (brnum != -1)
	{
		b = gbru.brushes + brnum;
		for (i = 0; i < b->num_surfes; i++)
		{
			s = b->surfes + i;
			if (s->itype != gtrace.brtype || s->sign != gtrace.brsign)
				continue;

			vec_copy(tr->color, s->color);
			tr->material = gbru.textures[s->texture].material;
			break;
		}

		if (gtrace.brsign)
		{
			tr->dist = b->absmin[gtrace.brtype];
			tr->normal[gtrace.brtype] = -1;
		}
		else
		{
			tr->dist = b->absmax[gtrace.brtype];
			tr->normal[gtrace.brtype] = 1;
		}
	}
}