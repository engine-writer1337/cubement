#ifndef _TRACE_H_
#define _TRACE_H_

void trace_bbox(const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs, const entity_s* ignore, int contents, trace_s* tr);
entity_s* trace_test_stuck(const vec3_t org, const vec3_t mins, const vec3_t maxs, const entity_s* ignore, int contents);
bool_t trace_test_stuck_ent(const entity_s* check, const vec3_t org, const vec3_t mins, const vec3_t maxs);

#endif