#ifndef _SHARED_H_
#define _SHARED_H_

#include <math.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <memory.h>

#define TRUE	1
#define FALSE	0

#define SAVEFUNC		__declspec(dllexport)

#define FL_INVISIBLE	(1 << 0)
#define FL_BEAM			(1 << 1)
#define FL_UPRIGHT		(1 << 2)

typedef float ftime_t;
typedef float vec2_t[2];
typedef float vec3_t[3];
typedef unsigned char byte;
typedef unsigned char bool_t;
typedef void (*conact_t)(const char* arg1, const char* arg2);

typedef enum
{
	RENDER_NORMAL,
	RENDER_TRANSPARENT,
	RENDER_ALPHA,
	RENDER_ADDITIVE,
}render_e;

typedef enum
{
	CHAN_AUTO,
	CHAN_BODY,
	CHAN_ITEM,
	CHAN_VOICE,
	CHAN_WEAPON,
	CHAN_LOOP,
}channel_e;

typedef enum
{
	FIELD_BOOL,
	FIELD_BYTE,
	FIELD_SHORT,
	FIELD_INT,
	FIELD_FLOAT,
	FIELD_TIME,
	FIELD_FUNC_OFFSET,
	FIELD_STRING,
	FIELD_ENTITY_POINTER,
}field_e;

#include "../game/ent_shared.h"

typedef struct entity_t
{
	entid_e id;
	char* model;

	int flags;
	int contents;

	vec3_t origin;
	vec3_t angles;
	vec3_t endorigin;

	vec3_t mins;
	vec3_t maxs;

	render_e render;
	byte renderamt;
	byte color[3];

	int skin;
	int body;
	float scale;
	float frame;

	float fps;
	int sequence;
	int numframes;

	vec2_t scroll;
	
	entvar_s* pev;
}entity_s;

typedef struct
{
	entid_e id;
	char* model;

	int flags;
	vec3_t origin;
	vec3_t angles;
	vec3_t velocity;
	vec3_t endorigin;

	render_e render;
	byte renderamt;
	byte color[3];

	int skin;
	int body;
	float scale;
	float frame;

	float fps;
	int sequence;
	int numframes;

	int iuser1;
	int iuser2;
	int iuser3;
	int iuser4;

	float fuser1;
	float fuser2;
	float fuser3;
	float fuser4;

	vec3_t vuser1;
	vec3_t vuser2;
	vec3_t vuser3;
	vec3_t vuser4;
}temp_entity_s;

typedef struct
{
	char name[32];
	char value[256];
}keyvalue_s;

typedef struct
{
	entid_e id;
	const char* name;
	bool_t(*spawn)(entity_s* self);
	void (*precache)(entity_s* self);
	void (*keyvalue)(entity_s* self, keyvalue_s* kv);
	void (*saverestore)(entvar_s* pev);
}entmap_s;

typedef struct
{
	entid_e id;
	const char* name;
	void (*spawn)(temp_entity_s* self);
}temp_entmap_s;

typedef struct
{
	float fraction;
	vec3_t endpos;
	entity_s* ent;

	float dist;
	vec3_t normal;

	byte color[3];
	char texturename[33];
}trace_s;

typedef struct
{
	int num_entities;
	entity_s* ent_base;

	int num_temp_entities;
	temp_entity_s* temp_ent_base;

	ftime_t time;
	ftime_t frametime;

	int width;
	int height;

	bool_t console_active;

	char* (*alloc_string)(const char* string);
	void (*execute_cmd)(const char* command);
	void (*changelevel)(const char* nextmap);

	void (*register_entity)(const entmap_s* ent);
	entity_s* (*create_entity)(const char* classname);
	void (*remove_entity)(entity_s* ent);

	void (*register_temp_entity)(const temp_entmap_s* ent);
	temp_entity_s* (*create_temp_entity)(const char* classname);
	void (*remove_temp_entity)(temp_entity_s* ent);

	void (*create_cvar)(const char* name, float value, bool_t save);
	void (*create_cmd)(const char* name, conact_t action);
	float (*get_cvar_value)(const char* name);
	void (*set_cvar_value)(const char* name, float value);

	void (*hide_cursor)();
	void (*draw_cursor)();
	void (*set_cursor_pos)(int x, int y);
	void (*set_mouse_pos)(int* x, int* y);

	void (*saverestore)(void* data, int count, field_e type);

	void (*precache_model)(const char* filename);
	void (*precache_sound)(const char* filename);
	void (*precache_sprite)(const char* filename);
	void (*precache_font)(const char* filename);
	void (*precache_pic)(const char* filename);

	int (*font_height)(const char* fontname);
	int (*font_len)(const char* fontname, const char* text);
	int (*font_print)(const char* fontname, const char* text, int x, int y, render_e render, byte r, byte g, byte b, byte a);

	void (*pic_size)(const char* picname, int* w, int* h);
	void (*pic_draw)(const char* picname, int x, int y, render_e render, byte r, byte g, byte b, byte a);
	void (*pic_draw_stretch)(const char* picname, int x, int y, int w, int h, render_e render, byte r, byte g, byte b, byte a);

	void (*world_to_screen)(const vec3_t org, int* x, int* y);

	void (*rect_draw)(int x, int y, int w, int h, render_e render, byte r, byte g, byte b, byte a);

	void (*particle_draw)(const char* picname, const vec3_t org, float size, render_e render, byte r, byte g, byte b, byte a);

	void (*set_numframes)(entity_s* ent);
	void (*set_sequence)(entity_s* ent, const char* name);
	void (*set_bodygroup)(entity_s* ent, int group, int body);
	void (*get_bonepos)(const entity_s* ent, const char* name, vec3_t pos);

	void (*ambient_play)(const char* soundname, vec3_t org, float volume, float distance);
	void (*sound_play)(const char* soundname, const entity_s* ent, channel_e chan, float volume, float distance);
	void (*sound_play_local)(const char* soundname, float volume);
	void (*sound_stop)(const entity_s* ent, channel_e chan);

	void (*set_view_far)(float far);
	void (*set_view_fov)(float fov);
	void (*set_view_ang)(const vec3_t ang);
	void (*set_view_org)(const vec3_t org);
	void (*set_fog)(float distance, byte r, byte g, byte b);

	void (*trace)(const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs, const entity_s* ignore, int contents, trace_s* tr);
	bool_t(*bbox_is_stuck)(const vec3_t org, const vec3_t mins, const vec3_t maxs, const entity_s* ignore, int contents);
}engine_s;

typedef struct
{
	int pev_size;

	void (*cvar_init)();
	void (*font_init)();
	void (*entity_register)();
	void (*after_engine_init)();
	void (*before_engine_free)();

	void (*game_once_precache)();
	void (*game_before_init)();
	void (*game_after_init)();
	void (*game_disconnect)();

	void (*before_draw_3d)();
	void (*after_draw_3d)();

	void (*before_draw_2d)();
	void (*after_draw_2d)();

	void (*before_chanelevel)();
	void (*after_chanelevel)();

	bool_t(*key_events)(int key, bool_t down);
}game_s;

extern engine_s* cment;

SAVEFUNC void Cubement(engine_s** e, game_s* g);

#define LINK_ENTITY(ename, eid) \
void add_##ename() {\
	entmap_s ent; \
	ent.id = eid;\
	ent.name = #ename;\
	ent.spawn = spawn_##ename;\
	ent.precache = precache_##ename;\
	ent.keyvalue = keyvalue_##ename;\
	ent.saverestore = saverestore_##ename;\
	cment->register_entity(&ent);\
}

#define LINK_TEMP_ENTITY(ename, eid) \
void add_temp_##ename() {\
	temp_entmap_s ent; \
	ent.id = eid;\
	ent.name = #ename;\
	ent.spawn = spawn_##ename;\
	cment->register_temp_entity(&ent);\
}

#define REGISTER_ENTITY(ename) extern void add_##ename(); add_##ename()
#define REGISTER_TEMP_ENTITY(ename) extern void add_temp_##ename(); add_temp_##ename()

#endif