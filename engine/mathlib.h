#ifndef _MATHLIB_H_
#define _MATHLIB_H_

#define PITCH	0
#define YAW		1
#define ROLL	2

#define MATH_PI		3.14159265358979323846f

#define NAN_MASK	(255 << 23)
#define	is_nan(x)	(((*(int *)&x) & NAN_MASK) == NAN_MASK)

#define deg2rad(x)				(x * MATH_PI / 180.f)
#define rad2deg(x)				(x * (180.f / MATH_PI))
#define clamp(min, num, max)	((num >= min) ? ((num < max) ? num : max) : min)

#define vec2_copy(dst, src)			(dst[0] = src[0], dst[1] = src[1])
#define vec2_set(dst, x, y)			(dst[0] = x, dst[1] = y)

#define vec_dot(x, y)				(x[0] * y[0] + x[1] * y[1] + x[2] * y[2])
#define vec_len(v)					(sqrtf(vec_dot(v, v)))
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
#define vec_normalize_fast(v)		{float ilength = math_rsqrt(vec_dot(v, v)); vec_mul(v, ilength);}
#define vec_cross(cross, v1, v2)	(cross[0] = v1[1]*v2[2] - v1[2]*v2[1], cross[1] = v1[2]*v2[0] - v1[0]*v2[2], cross[2] = v1[0]*v2[1] - v1[1]*v2[0])

#define vec4_init(dst, a)			(dst[0] = a, dst[1] = a, dst[2] = a, dst[3] = a)
#define vec4_copy(dst, src)			(dst[0] = src[0], dst[1] = src[1], dst[2] = src[2], dst[3] = src[3])
#define vec4_set(dst, x, y, z, w)	(dst[0] = x, dst[1] = y, dst[2] = z, dst[3] = w)

inline float math_rsqrt(float number)
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

inline int vec_signbits(const vec3_t normal)
{
	int bits = 0;

	if (normal[0] < 0) bits |= 1 << 0;
	if (normal[1] < 0) bits |= 1 << 1;
	if (normal[2] < 0) bits |= 1 << 2;
	return bits;
}

inline float vec3_normalize(vec3_t v)
{
	float length;

	length = vec_len(v);
	if (length)
	{
		float ilength;

		ilength = 1.0f / length;
		v[0] *= ilength;
		v[1] *= ilength;
		v[2] *= ilength;
	}

	return length;
}

inline void vec_from_angles(const vec3_t angles, vec3_t forward, vec3_t right, vec3_t up)
{
	float angle, sr, sp, sy, cr, cp, cy;

	if (angles[YAW])
	{
		angle = deg2rad(angles[YAW]);
		sy = sinf(angle);
		cy = cosf(angle);
	}
	else
		sy = 0, cy = 1;

	if (angles[PITCH])
	{
		angle = deg2rad(angles[PITCH]);
		sp = sinf(angle);
		cp = cosf(angle);
	}
	else
		sp = 0, cp = 1;

	if (angles[ROLL])
	{
		angle = deg2rad(angles[ROLL]);
		sr = sinf(angle);
		cr = cosf(angle);
	}
	else
		sr = 0, cr = 1;

	if (forward)
	{
		forward[0] = cp * cy;
		forward[1] = cp * sy;
		forward[2] = -sp;
	}

	if (right)
	{
		right[0] = -sr * sp * cy + cr * sy;
		right[1] = -sr * sp * sy - cr * cy;
		right[2] = -sr * cp;
	}

	if (up)
	{
		up[0] = cr * sp * cy + sr * sy;
		up[1] = cr * sp * sy - sr * cy;
		up[2] = cr * cp;
	}
}

inline void vec_to_angles(const vec3_t forward, vec3_t angles)
{
	float tmp, yaw, pitch;

	if (forward[0] == 0 && forward[1] == 0)
	{
		yaw = 0;
		if (forward[2] > 0)
			pitch = 90.0f;
		else 
			pitch = 270.0f;
	}
	else
	{
		yaw = (atan2f(forward[1], forward[0]) * 180 / MATH_PI);
		if (yaw < 0) yaw += 360;

		tmp = sqrtf(forward[0] * forward[0] + forward[1] * forward[1]);
		pitch = (atan2f(forward[2], tmp) * 180 / MATH_PI);
		if (pitch < 0) pitch += 360;
	}

	vec_set(angles, -pitch, yaw, 0);//TODO: check stupid quake bug
}

#endif