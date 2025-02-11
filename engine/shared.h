#ifndef _SHARED_H_
#define _SHARED_H_

#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4244)

#include <math.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <memory.h>

#define TRUE	1
#define FALSE	0

#define EXPORTFUNC		__declspec(dllexport)

#define strnull(str)		(!str || !str[0])
#define strcmpc(dst, src)	(!strcmp(dst, src))
#define strcmpi(dst, src)	(!_stricmp(dst, src))
#define strcpyn(dst, src)	(strncpy(dst, src, sizeof(dst) - 1), dst[sizeof(dst) - 1] = '\0')
#define strcatn(dst, src)	(strncat(dst, src, sizeof(dst) - 1), dst[sizeof(dst) - 1] = '\0')
#define memzero(mem, sz)	(memset(mem, 0, sz))

#define FL_INVISIBLE	(1 << 0)
#define FL_BEAM			(1 << 1)
#define FL_UPRIGHT		(1 << 2)
#define FL_MODULATE		(1 << 3)
#define FL_NOFOG		(1 << 4)
#define FL_SHADOW		(1 << 5)
#define FL_NODISSIP		(1 << 6)
#define FL_SAVE			(1 << 7)
#define FL_TEMP			(1 << 8)

#define IMG_NEAREST		(1 << 0)
#define IMG_CLAMP		(1 << 1)
#define IMG_MIPMAP		(1 << 2)

#define ANIM_REVERSE	(1 << 0)
#define ANIM_GAIT		(1 << 1)
#define ANIM_LOOP		(1 << 2)

#define BAD_HANDLE	-1

typedef int ihandle_t;
typedef float ftime_t;
typedef float vec2_t[2];
typedef float vec3_t[3];
typedef unsigned char byte;
typedef unsigned int hash_t;
typedef unsigned char bool_t;
typedef void (*conact_t)(const char* arg1, const char* arg2);

#include "keys.h"
#include "mathlib.h"

typedef enum
{
	COLOR_WHITE,
	COLOR_ORANGE,
	COLOR_CYAN,
	COLOR_RED,
	COLOR_GREEN,
}color_e;

typedef enum
{
	RENDER_NORMAL,
	RENDER_TRANSPARENT,
	RENDER_ALPHA,
	RENDER_ADDTIVE,
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
	FIELD_VECTOR2,
	FIELD_VECTOR3,
	FIELD_TIME,
	FIELD_STRING,
	FIELD_RESOURCE,
	FIELD_ENTITY_POINTER,
}field_e;

#include "../game/ent_shared.h"

typedef struct
{
	char key[32];
	char value[256];
}keyvalue_s;

typedef struct
{
	entid_e id;
	int pev_size;
	const char* name;
	bool_t(*spawn)(void* pev);
	void (*precache)(void* pev);
	void (*keyvalue)(void* pev, keyvalue_s* kv);
	void (*saverestore)(void* pev);
	void (*think)(void* pev);

	hash_t hash;
}entmap_s;

typedef struct
{
	float fraction;
	vec3_t endpos;
	entity_s* ent;

	float dist;
	vec3_t normal;

	int hitbox;

	byte color[3];
	char texturename[33];
}trace_s;

typedef struct
{
	int frame;
	void (*func)(void* pev);
}event_s;

typedef struct
{
	float framerate;

	int sequence_num;
	const char* sequence_name;

	int num_events;
	event_s* events;
}anim_s;

typedef struct
{
	float value;
	bool_t is_change;
}cvar_s;

typedef struct
{
	//======================================================
	// Global variables that are updated every frame
	//======================================================
	ftime_t time;
	ftime_t gametime;
	ftime_t frametime;

	int width;
	int height;
	int centr_x;
	int centr_y;

	bool_t world_load;
	bool_t console_active;

	//======================================================
	// Entity
	//======================================================
	int entities_max;
	entity_s** entities;

	void (*ent_register)(const entmap_s* ent);
	entity_s* (*ent_create)(const char* classname);
	void (*ent_remove)(entity_s* ent);

	void (*ent_saverestore)(void* data, int count, field_e type);
	void (*ent_play_anim)(anim_s* anim, int flags);
	void (*ent_set_body)(entity_s* ent, int group, int body);
	void (*ent_get_bonepos)(const entity_s* ent, const char* name, vec3_t pos);
	void (*ent_add_boneang)(const entity_s* ent, const char* name, const vec3_t ang);

	//======================================================
	// Resources
	//======================================================
	ihandle_t(*resource_precache)(const char* filename);
	ihandle_t(*resource_precache_ex)(const char* filename, int flags);
	ihandle_t(*resource_get_handle)(const char* filename);

	//======================================================
	// Font
	//======================================================
	int (*font_height)(ihandle_t idx);
	int (*font_len)(ihandle_t idx, const char* text);
	int (*font_print)(ihandle_t idx, const char* text, int x, int y, render_e render, byte r, byte g, byte b, byte a);

	//======================================================
	// Skybox
	//======================================================
	void (*sky_load)(const char* name);
	void (*sky_visible)(bool_t is_visible);
	void (*sky_rotate)(const vec3_t ang);

	//======================================================
	// Cursor
	//======================================================
	void (*cursor_reset_pos)();
	void (*cursor_show)(bool_t show);
	void (*cursor_set_pos)(int x, int y);
	void (*cursor_get_pos)(int* x, int* y, bool_t client);

	//======================================================
	// Custom 2d drawing
	//======================================================
	int (*pic_get_numframes)(ihandle_t idx);
	void (*pic_get_size)(ihandle_t idx, int frame, int* w, int* h);
	void (*pic_draw)(ihandle_t idx, int frame, int x, int y, render_e render, byte r, byte g, byte b, byte a);
	void (*pic_draw_stretch)(ihandle_t idx, int frame, int x, int y, int w, int h, render_e render, byte r, byte g, byte b, byte a);

	void (*world_to_screen)(const vec3_t org, int* x, int* y);

	void (*rect_draw)(int x, int y, int w, int h, render_e render, byte r, byte g, byte b, byte a);

	//======================================================
	// Sound
	//======================================================
	void (*music_play)(const char* filename);
	void (*music_pause)(bool_t pause);
	void (*music_stop)();

	void (*sound_play)(ihandle_t idx, const entity_s* ent, channel_e chan, float volume, float distance);
	void (*sound_play_ambient)(ihandle_t idx, const entity_s* ent, vec3_t org, float volume, float distance);//TODO:
	void (*sound_play_local)(ihandle_t idx, float volume);
	void (*sound_stop)(const entity_s* ent, channel_e chan);

	//======================================================
	// Render settings
	//======================================================
	void (*set_view_fov)(float fov);
	void (*set_view_ang)(const vec3_t ang);
	void (*set_view_org)(const vec3_t org);
	void (*set_fog)(float distance, byte r, byte g, byte b);

	//======================================================
	// Console
	//======================================================
	void (*con_execute)(const char* command);
	void (*con_printf)(color_e color, const char* text, ...);

	void (*con_create_cmd)(const char* name, conact_t action);
	cvar_s* (*con_create_cvar)(const char* name, float value, bool_t save);

	//======================================================
	// Custom 3d drawing
	//======================================================
	void (*custom_model)(ihandle_t idx, const vec3_t org, const vec3_t ang, render_e render, byte r, byte g, byte b, byte a);
	void (*custom_sprite)(ihandle_t idx, int frame, const vec3_t org, float scale, render_e render, byte r, byte g, byte b, byte a);

	void (*custom_bind)(ihandle_t idx, int frame);
	void (*custom_color)(byte r, byte g, byte b, byte a);
	void (*custom_rendermode)(render_e render);
	void (*custom_color_pointer)(const byte* colors);
	void (*custom_texcoord_pointer)(const vec2_t* texcoords);
	void (*custom_verts_pointer)(const vec3_t* verts);
	void (*custom_draw_arrays)(bool_t quads, int first, int count);

	//======================================================
	// Uncategorized
	//======================================================
	void (*get_mapname)(char* name);

	void (*trace)(const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs, const entity_s* ignore, int contents, trace_s* tr);
	bool_t(*bbox_is_stuck)(const vec3_t org, const vec3_t mins, const vec3_t maxs, const entity_s* ignore, int contents);

	void (*area_active)(const char* name, bool_t is_active);
	int (*get_areas)(const vec3_t org, const vec3_t mins, const vec3_t maxs, int* indicies, int count);
}engine_s;

typedef struct
{
	const char* windowname;

	void (*cvar_init)();
	void (*engine_init)();
	void (*engine_free)();

	void (*game_precache)();
	void (*game_start)();
	void (*game_end)();

	void (*draw_2d)();
	void (*draw_3d)();

	void (*window_active)();
	void (*window_inactive)();

	bool_t(*pause_world)();
	bool_t(*draw_world)();

	bool_t(*char_events)(int ch);
	bool_t(*key_events)(int key, bool_t down);
}game_s;

extern engine_s* cment;

EXPORTFUNC void cubement(engine_s** e, game_s* g);

#define LINK_ENTITY(ename, eid, esize) \
void add_##ename() {\
	entmap_s ent; \
	ent.id = eid;\
	ent.name = #ename;\
	ent.pev_size = esize;\
	ent.spawn = spawn_##ename;\
	ent.think = think_##ename;\
	ent.precache = precache_##ename;\
	ent.keyvalue = keyvalue_##ename;\
	ent.saverestore = saverestore_##ename;\
	cment->ent_register(&ent);\
}

#define REGISTER_ENTITY(ename) extern void add_##ename(); add_##ename()

#endif