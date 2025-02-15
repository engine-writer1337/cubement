#include "cubement.h"

static struct
{
	vec3_t dir;
	vec3_t idir;
	vec3_t start;

	vec3_t trmin;
	vec3_t trmax;

	vec3_t epsmin;
	vec3_t epsmax;

	vec3_t absmin;
	vec3_t absmax;

	float dist;

	int brnum;
	int brtype;
	bool_t brsign;
}gtrace;

static bool_t trace_brush(brush_s* b)
{
	int k;
	float dist;
	vec3_t pt, bmin, bmax;

	for (k = 0; k < 3; k++)
	{
		if (!gtrace.dir[k])
			continue;

		if (gtrace.dir[k] > 0)
		{
			if (gtrace.absmax[k] > b->mins[k])
				continue;

			dist = (gtrace.absmax[k] - b->mins[k]) * gtrace.idir[k];
			if (dist > gtrace.dist)
				return FALSE;
		}
		else
		{
			if (gtrace.absmin[k] < b->maxs[k])
				continue;

			dist = (gtrace.absmin[k] - b->maxs[k]) * gtrace.idir[k];
			if (dist > gtrace.dist)
				return FALSE;
		}

		vec_ma(pt, gtrace.start, dist, gtrace.dir);
		vec_add(bmin, pt, gtrace.epsmin);
		vec_add(bmax, pt, gtrace.epsmax);
		if (vec_aabb(b->mins, b->maxs, bmin, bmax))
			continue;

		gtrace.dist = dist;
		vec_maxmin(gtrace.start, pt, gtrace.epsmin, gtrace.epsmax, gtrace.trmin, gtrace.trmax);
		gtrace.brsign = (gtrace.dir[k] > 0);
		gtrace.brtype = k;
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
	int i, j, k;
	vec3_t offset;
	resource_s* res;
	brushmodel_s* bm;
	brush_s* b, bfake;
	float dist, startdist;
	vec2_t vpt, vbmin, vbmax;

	if (!gentworld)
		return;

	vec_sub(gtrace.dir, end, start);
	gtrace.dist = startdist = vec_normalize(gtrace.dir);
	for (i = 0; i < 3; i++)
	{
		if (fabs(gtrace.dir[i]) < 1e-5)
			gtrace.dir[i] = gtrace.idir[i] = 0;
		else
			gtrace.idir[i] = -1.0f / gtrace.dir[i];
	}

	gtrace.brnum = -1;
	gtrace.brtype = 0;
	gtrace.brsign = FALSE;

	gworld.framecount++;
	vec_copy(gtrace.start, start);
	vec_add(gtrace.absmin, start, mins);
	vec_add(gtrace.absmax, start, maxs);
	vec_add_val(gtrace.epsmin, mins, -EPSILON);
	vec_add_val(gtrace.epsmax, maxs, EPSILON);
	vec_maxmin(start, end, gtrace.epsmin, gtrace.epsmax, gtrace.trmin, gtrace.trmax);

	tr->dist = 0;
	vec_init(tr->color, 0);
	vec_init(tr->normal, 0);
	strcpyn(tr->texturename, "NULL");

	for (i = 0; i < gbru.num_areas; i++)
	{
		a = gbru.areas + i;
		for (j = 0; j < a->num_boxes; j++)
		{
			if (!vec2_aabb(a->mins[j], a->maxs[j], gtrace.absmin, gtrace.absmax))
			{
				a->framenum = gworld.framecount;
				break;
			}

			if (!a->activecount)
				continue;

			if (vec2_aabb(a->mins[j], a->maxs[j], gtrace.trmin, gtrace.trmax))
				continue;

			for (k = 0; k < 2; k++)
			{
				if (!gtrace.dir[k])
					continue;

				if (gtrace.dir[k] > 0)
				{
					if (gtrace.absmax[k] > a->mins[j][k])
						continue;

					dist = (gtrace.absmax[k] - a->mins[j][k]) * gtrace.idir[k];
				}
				else
				{
					if (gtrace.absmin[k] < a->maxs[j][k])
						continue;

					dist = (gtrace.absmin[k] - a->maxs[j][k]) * gtrace.idir[k];	
				}

				vec2_ma(vpt, start, dist, gtrace.dir);
				vec2_add(vbmin, vpt, gtrace.epsmin);
				vec2_add(vbmax, vpt, gtrace.epsmax);
				if (vec2_aabb(a->mins[j], a->maxs[j], vbmin, vbmax))
					continue;

				a->framenum = gworld.framecount;
				break;
			}

			if (a->framenum == gworld.framecount)
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
			if (vec_aabb(b->mins, b->maxs, gtrace.trmin, gtrace.trmax))
				continue;
			
			if (trace_brush(b))
				gtrace.brnum = a->brushareas[j];
		}
	}

	for (i = 1; i < gnuments; i++)
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
				if (vec_aabb(bfake.mins, bfake.maxs, gtrace.trmin, gtrace.trmax))
					continue;

				if (trace_brush(&bfake))
					gtrace.brnum = bm->start_brush + j;
			}
			break;
		}
	}

	vec_ma(tr->endpos, start, gtrace.dist, gtrace.dir);
	tr->fraction = startdist / gtrace.dist;

	if (gtrace.brnum != -1)
	{
		//gtrace.dist -= 1;
		//vec_ma(tr->endpos, start, gtrace.dist, gtrace.dir);

		b = gbru.brushes + gtrace.brnum;
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
			tr->normal[gtrace.brtype] = 1;
		}
		else
		{
			tr->dist = b->maxs[gtrace.brtype];
			tr->normal[gtrace.brtype] = -1;
		}
	}
}