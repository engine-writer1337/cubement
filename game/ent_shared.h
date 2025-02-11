#ifndef _ENT_SHARED_H_
#define _ENT_SHARED_H_

#define CONTENTS_EMPTY		0
#define CONTENTS_WORLD		(1 << 0)
#define CONTENTS_SOLID		(1 << 1)
#define CONTENTS_TRIGGER	(1 << 2)
#define CONTENTS_WATER		(1 << 3)
#define CONTENTS_ALL		(CONTENTS_WORLD | CONTENTS_SOLID | CONTENTS_TRIGGER | CONTENTS_WATER)

typedef enum
{
	ENTID_FREE = -1,
	ENTID_WORLDSPAWN,
	ENTID_PLAYER,
	ENTID_WALL,

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
	vec3_t endorigin;

	vec3_t mins;
	vec3_t maxs;

	render_e render;
	byte renderamt;
	byte color[3];

	int skin;
	int body;
	float scale;

	vec2_t scroll;

	ftime_t nextthink;
	//======================================================
	// Here are custom fields, can be written in any order
	//======================================================
	vec3_t velocity;

}entity_s;

#endif