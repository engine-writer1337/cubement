#include "cubement.h"

void trace(const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs, const entity_s* ignore, int contents, trace_s* tr)
{
	area_s* a;
	brush_s* b;
	int i, j, k;
	float dist, startdist;
	vec3_t dir, idir, absmin, absmax, epsmin, epsmax, trmin, trmax, pt, bmin, bmax;

	vec_sub(dir, end, start);
	tr->fraction = startdist = vec_normalize(dir);
	for (i = 0; i < 3; i++)
	{
		if (fabs(dir[i]) < 1e-5)
			dir[i] = idir[i] = 0;
		else
			idir[i] = -1.0f / dir[i];
	}

	gworld.framecount++;
	vec_add(absmin, start, mins);
	vec_add(absmax, start, maxs);
	vec_add_val(epsmin, mins, -EPSILON);
	vec_add_val(epsmax, maxs, EPSILON);
	vec_maxmin(start, end, epsmin, epsmax, trmin, trmax);

	for (i = 0; i < gbru.num_areas; i++)
	{
		a = gbru.areas + i;
		for (j = 0; j < a->num_boxes; j++)
		{
			if (!vec2_aabb(a->mins[j], a->maxs[j], absmin, absmax))
			{
				a->framenum = gworld.framecount;
				break;
			}

			if (!a->activecount)
				continue;

			if (vec2_aabb(a->mins[j], a->maxs[j], trmin, trmax))
				continue;

			for (k = 0; k < 2; k++)
			{
				if (!dir[k])
					continue;

				if (dir[k] > 0)
				{
					if (start[k] > a->mins[j][k])
						continue;

					dist = (absmax[k] - a->mins[j][k]) * idir[k];
					if (tr->fraction < dist)
						continue;

					vec2_ma(pt, start, dist, dir);
				}
				else
				{
					if (start[k] < a->maxs[j][k])
						continue;

					dist = (absmin[k] - a->maxs[j][k]) * idir[k];
					if (tr->fraction < dist)
						continue;

					vec2_ma(pt, start, dist, dir);
				}

				vec2_add(bmin, pt, epsmin);
				vec2_add(bmax, pt, epsmax);
				if (vec2_aabb(bmin, bmax, a->mins[j], a->maxs[j]))
					continue;

				a->framenum = gworld.framecount;
				break;
			}

			if (a->framenum == gworld.framecount)
				break;
		}

		if (a->framenum != gworld.framecount)
			continue;

		for (j = 0; j < a->num_brushareas; j++)
		{
			b = gbru.brushes + a->brushareas[j];
			if (b->framenum == gworld.framecount)
				continue;

			b->framenum = gworld.framecount;
			if (vec_aabb(b->mins, b->maxs, trmin, trmax))
				continue;

			for (k = 0; k < 3; k++)
			{
				if (!dir[k])
					continue;

				if (dir[k] > 0)
				{
					if (start[k] > b->mins[k])
						continue;

					dist = (absmax[k] - b->mins[k]) * idir[k];
					if (tr->fraction < dist)
						continue;

					vec_ma(pt, start, dist, dir);
				}
				else
				{
					if (start[k] < b->maxs[k])
						continue;

					dist = (absmin[k] - b->maxs[k]) * idir[k];
					if (tr->fraction < dist)
						continue;

					vec_ma(pt, start, dist, dir);
				}

				vec_add(bmin, pt, epsmin);
				vec_add(bmax, pt, epsmax);
				if (vec_aabb(bmin, bmax, b->mins, b->maxs))
					continue;
				
				tr->fraction = dist;
				vec_maxmin(start, pt, epsmin, epsmax, trmin, trmax);
				break;
			}
		}
	}

	vec_ma(tr->endpos, start, tr->fraction, dir);
	tr->fraction /= startdist;
}