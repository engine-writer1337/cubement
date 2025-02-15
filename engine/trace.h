#ifndef _TRACE_H_
#define _TRACE_H_

#define TRACE_EPSILON	(1.0f / 8)

void trace(const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs, const entity_s* ignore, int contents, trace_s* tr);

#endif