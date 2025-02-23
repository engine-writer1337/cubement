#ifndef _ENT_SHARED_H_
#define _ENT_SHARED_H_

#define CONTENTS_EMPTY		0
#define CONTENTS_WORLD		(1 << 0)
#define CONTENTS_BBOX		(1 << 1)
#define CONTENTS_BRUSH		(1 << 2)
#define CONTENTS_TRIGGER	(1 << 3)
#define CONTENTS_WATER		(1 << 4)

#define CONTENTS_SOLID		(CONTENTS_WORLD | CONTENTS_BBOX | CONTENTS_BRUSH)
#define CONTENTS_ALL		(CONTENTS_WORLD | CONTENTS_BBOX | CONTENTS_BRUSH | CONTENTS_TRIGGER | CONTENTS_WATER)

typedef enum
{
	MAT_DEFAULT = 0,
	MAT_DIRT = 1,
	MAT_METAL = 2,

	MAX_MATERIALS = 64
}material_e;

typedef enum
{
	ENTID_FREE = -1,
	ENTID_WORLDSPAWN,
	ENTID_PLAYER,
	ENTID_WALL,
	ENTID_DOOR_ROT,
	ENTID_START,

	MAX_ENTMAP = 512
}entid_e;

typedef struct entity_t
{
	//======================================================
	// Here are engine fields, do not move them
	//======================================================
	entid_e id;
	ihandle_t model;

	int flags;
	int contents;

	vec3_t origin;
	vec3_t angles;

	vec3_t mins;
	vec3_t maxs;

	vec3_t absmin;
	vec3_t absmax;

	render_e render;
	byte renderamt;
	byte color[3];

	int skin;
	int body;
	float scale;

	vec2_t scroll;
	//======================================================
	// Here are custom fields, can be written in any order
	//======================================================
	vec3_t velocity;
	vec3_t avelocity;

	ftime_t nextthink;

	void (*block)(void* pev, struct entity_t* blocker);
	void (*touch)(void* pev, struct entity_t* toucher, struct trace_t* tr);
	void (*use)(void* pev, struct entity_t* activator);
}entity_s;

#endif