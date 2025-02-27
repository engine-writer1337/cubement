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
			return TRUE;
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

void trace_bbox(const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs, const entity_s* ignore, int contents, trace_s* tr)
{
	area_s* a;
	surf_s* s;
	entity_s* e;
	edict_s* ed;
	entity_s* touch;
	int i, j, brnum;
	brushmodel_s* bm;
	brush_s* b, bfake, bfaketouch;
	vec3_t movevec, absmin, absmax;
	bool_t anyintersect, rotate, faketouch;

	if (!gentworld)
		return;

	brnum = -1;
	touch = NULL;
	faketouch = FALSE;
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
		if (ignore == e || !(e->contents & contents))
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

		if (e->model != BAD_HANDLE && gres[e->model].type == RES_BRUSH)
		{
			bm = gres[e->model].brush;
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
				faketouch = FALSE;
			}
		}
		else
		{
			vec_copy(bfake.absmin, e->absmin);
			vec_copy(bfake.absmax, e->absmax);
			if (vec_aabb(bfake.absmin, bfake.absmax, gtrace.bmin, gtrace.bmax))
				continue;

			if (!trace_brush(&bfake))
				continue;

			//if (e->model != BAD_HANDLE && gres[e->model].type == RES_MODEL)
			//TODO: hitboxes (when mins and maxs is zeros)

			touch = e;
			brnum = -1;
			faketouch = TRUE;
			vec_copy(bfaketouch.absmin, bfake.absmin);
			vec_copy(bfaketouch.absmax, bfake.absmax);
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
	else if (faketouch)
	{
		if (gtrace.brsign)
		{
			tr->dist = bfaketouch.absmin[gtrace.brtype];
			tr->normal[gtrace.brtype] = -1;
		}
		else
		{
			tr->dist = bfaketouch.absmax[gtrace.brtype];
			tr->normal[gtrace.brtype] = 1;
		}
	}
}

entity_s* trace_test_stuck(const vec3_t org, const vec3_t mins, const vec3_t maxs, const entity_s* ignore, int contents)
{
	int i, j;
	area_s* a;
	entity_s* e;
	edict_s* ed;
	brushmodel_s* bm;
	brush_s* b, bfake;
	bool_t anyintersect, rotate;
	vec3_t movevec, absmin, absmax;

	if (!gentworld)
		return NULL;

	gworld.framecount++;
	vec_add(absmin, org, mins);
	vec_add(absmax, org, maxs);
	vec_add_val(absmin, absmin, -BOUND_EPSILON);
	vec_add_val(absmax, absmax, BOUND_EPSILON);

	for (i = 0; i < gbru.num_areas; i++)
	{
		a = gbru.areas + i;
		for (j = 0; j < a->num_boxes; j++)
		{
			if (!vec2_aabb(a->absmin[j], a->absmax[j], absmin, absmax))
			{
				a->framenum = gworld.framecount;
				break;
			}

			if (!a->activecount || vec2_aabb(a->absmin[j], a->absmax[j], absmin, absmax))
				continue;

			a->framenum = gworld.framecount;
			break;
		}

		if (a->framenum != gworld.framecount)
			continue;

		if (!(gentworld->contents & contents))
			continue;

		for (j = 0; j < a->num_brushareas; j++)
		{
			b = gbru.brushes + a->brushareas[j];
			if (b->framenum == gworld.framecount)
				continue;

			b->framenum = gworld.framecount;
			if (!vec_aabb(b->absmin, b->absmax, absmin, absmax))
				return gentworld;
		}
	}

	for (i = 1; i < gnuments; i++)
	{
		ed = gedicts + i;
		if (!ed->e)
			continue;

		e = ed->e;
		if (ignore == e || !(e->contents & contents))
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

		if (e->model != BAD_HANDLE && gres[e->model].type == RES_BRUSH)
		{
			bm = gres[e->model].brush;
			rotate = !vec_is_zero(e->angles);
			vec_sub(movevec, e->origin, bm->origin);

			for (j = 0; j < bm->num_brushes; j++)
			{
				b = bm->brushes + j;
				vec_add(bfake.absmin, movevec, b->absmin);
				vec_add(bfake.absmax, movevec, b->absmax);

				if (rotate)
					vec_rotate_org_bbox(e->angles, e->origin, bm->offset, bfake.absmin, bfake.absmax);

				if (!vec_aabb(bfake.absmin, bfake.absmax, absmin, absmax))
					return e;
			}
		}
		else
		{
			if (!vec_aabb(e->absmin, e->absmax, absmin, absmax))
				return e;
		}
	}

	return NULL;
}

bool_t trace_test_stuck_ent(const entity_s* check, const vec3_t org, const vec3_t mins, const vec3_t maxs)
{
	int j;
	bool_t rotate;
	brushmodel_s* bm;
	brush_s* b, bfake;
	vec3_t movevec, absmin, absmax;

	if (!gentworld || !check || check->id == ENTID_FREE)
		return FALSE;

	vec_add(absmin, org, mins);
	vec_add(absmax, org, maxs);
	vec_add_val(absmin, absmin, -BOUND_EPSILON);
	vec_add_val(absmax, absmax, BOUND_EPSILON);

	if (vec_aabb(check->absmin, check->absmax, absmin, absmax))
		return FALSE;

	if (check->model != BAD_HANDLE && gres[check->model].type == RES_BRUSH)
	{
		if (check == gentworld)
			bm = gbru.models;
		else
			bm = gres[check->model].brush;

		rotate = !vec_is_zero(check->angles);
		vec_sub(movevec, check->origin, bm->origin);

		for (j = 0; j < bm->num_brushes; j++)
		{
			b = bm->brushes + j;
			vec_add(bfake.absmin, movevec, b->absmin);
			vec_add(bfake.absmax, movevec, b->absmax);

			if (rotate)
				vec_rotate_org_bbox(check->angles, check->origin, bm->offset, bfake.absmin, bfake.absmax);

			if (!vec_aabb(bfake.absmin, bfake.absmax, absmin, absmax))
				return TRUE;			
		}
	}

	return FALSE;
}