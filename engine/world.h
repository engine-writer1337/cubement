#ifndef _WORLD_H_
#define _WORLD_H_

#define SKY_FOLDER		"env/"

#define FRUSTUM_LEFT	0
#define FRUSTUM_RIGHT	1
#define FRUSTUM_BOTTOM	2
#define FRUSTUM_TOP		3
#define FRUSTUM_NUM		4

typedef struct
{
	float dist;
	vec3_t normal;
	int signbits;
}plane_s;

typedef struct
{
	int framecount;

	float max_x;
	float max_y;
	float fov_x;
	float fov_y;
	float sin_x;
	float sin_y;
	float cos_x;
	float cos_y;

	bool_t is_load;

	vec3_t viewang;
	vec3_t vieworg;

	vec3_t v_forward;
	vec3_t v_right;
	vec3_t v_up;

	convar_s* vbo;
}world_s;

typedef struct
{
	name_t name;
	bool_t visible;
	glpic_t pics[6];
}sky_s;

extern sky_s gsky;
extern world_s gworld;
extern entity_s* gentworld;

void sky_load(const char* name);
void sky_free();

void world_draw();

void world_set_fov(float fov);
void world_view_ang(const vec3_t ang);
void world_view_org(const vec3_t org);
void world_setup3d();

void world_map_cmd(const char* arg1, const char* arg2);
void world_load_map();

#endif