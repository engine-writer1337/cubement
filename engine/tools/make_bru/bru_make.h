#ifndef _BRU_MAKE_H_
#define _BRU_MAKE_H_

#define  _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "bru.h"
#include "bru_save.h"

#define TRUE	1
#define FALSE	0

#define EQUAL_EPSILON	0.01f

#define TEX_NULL	-1
#define TEX_TRIGGER	-2
#define TEX_AREA	-3
#define TEX_ORIGIN	-4

#define vec2_copy(dst, src)			(dst[0] = src[0], dst[1] = src[1])

#define vec_dot(x, y)				(x[0] * y[0] + x[1] * y[1] + x[2] * y[2])
#define vec_len(v)					(sqrt(vec_dot(v, v)))
#define vec_copy(dst, src)			(dst[0] = src[0], dst[1] = src[1], dst[2] = src[2])
#define vec_set(dst, x, y, z)		(dst[0] = x, dst[1] = y, dst[2] = z)
#define vec_init(dst, a)			(dst[0] = a, dst[1] = a, dst[2] = a)
#define vec_neg(x, y)				(x[0] = -y[0], x[1] = -y[1], x[2] = -y[2])
#define vec_mul(v, a)				(v[0] *= a, v[1] *= a, v[2] *= a)
#define vec_sub(v, a, b)			(v[0] = a[0] - b[0], v[1] = a[1] - b[1], v[2] = a[2] - b[2]) 
#define vec_add(v, a, b)			(v[0] = a[0] + b[0], v[1] = a[1] + b[1], v[2] = a[2] + b[2]) 
#define vec_add_val(v, a, val)		(v[0] = a[0] + val, v[1] = a[1] + val, v[2] = a[2] + val) 
#define vec_ma(v, a, scale, b)		(v[0] = a[0] + scale * b[0], v[1] = a[1] + scale * b[1], v[2] = a[2] + scale * b[2])
#define vec_mam(v, scale1, b1, scale2, b2) (v[0] = scale1 * b1[0] + scale2 * b2[0], v[1] = scale1 * b1[1] + scale2 * b2[1], v[2] = scale1 * b1[2] + scale2 * b2[2])
#define vec_scale(v, scale, b)		(v[0] = scale * b[0], v[1] = scale * b[1], v[2] = scale * b[2])
#define vec_clear(v)				(v[0] = v[1] = v[2] = 0)
#define vec_dist(a, b)				(sqrtf((a[0] - b[0]) * (a[0] - b[0]) + (a[1] - b[1]) * (a[1] - b[1]) + (a[2] - b[2]) * (a[2] - b[2])))
#define vec_lerp(a, scale, b, c)	(a[0] = b[0] + scale * (c[0] - b[0]), a[1] = b[1] + scale * (c[1] - b[1]), a[2] = b[2] + scale * (c[2] - b[2]))
#define vec_cross(cross, v1, v2)	(cross[0] = v1[1]*v2[2] - v1[2]*v2[1], cross[1] = v1[2]*v2[0] - v1[0]*v2[2], cross[2] = v1[0]*v2[1] - v1[1]*v2[0])
#define vec_normalize(v)			{double ilength = rsqrt(vec_dot(v, v)); vec_mul(v, ilength);}

#define clamp(min, num, max)		((num >= min) ? ((num < max) ? num : max) : min)

typedef char bool_t;
typedef double vec3_t[3];
typedef char string_t[1024];

typedef struct _epair_s
{
	char key[MAX_MAP_KEY];
	char value[MAX_MAP_VALUE];

	struct _epair_s* next;
}epair_s;

typedef struct _esurf_s
{
	byte type;
	short color;

	int texinfo;
	struct _esurf_s* next;
}esurf_s;

typedef struct _ebrush_s
{
	bool_t is_origin;

	esurf_s* esurfes;
	vec3_t mins, maxs;

	struct _ebrush_s* next;
}ebrush_s;

typedef struct
{
	epair_s* epairs;
	ebrush_s* ebrushes;

	vec3_t offset;
	bool_t has_offset;
}entity_s;

extern int gnum_entities;
extern entity_s gentities[MAX_MAP_ENTITIES];

extern void fatal_error(const char* error, ...);

_inline float rsqrt(float number)
{
	int i;
	float x, y;

	if (!number)
		return 0;

	x = number * 0.5f;
	i = *(int*)&number;
	i = 0x5f3759df - (i >> 1);
	y = *(float*)&i;
	y = y * (1.5f - (x * y * y));
	return y;
}

#endif