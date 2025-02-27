#ifndef _ENTITY_H_
#define _ENTITY_H_

#define MAX_KEYVALUES	64
#define MAX_ENTITIES	3072

typedef struct _edict_s
{
	entity_s* e;

	int num_areas;
	byte areas[MAX_MAP_AREAS];

	float frame;

	float fps;
	int sequence;
	int numframes;

	struct
	{
		vec3_t origin;
		vec3_t angles;

		vec3_t absmin;
		vec3_t absmax;

		vec3_t mins;
		vec3_t maxs;

		ihandle_t model;
	}old;

	struct _edict_s* next;
}edict_s;

extern int gnuments;
extern entity_s* gents[MAX_ENTITIES];

extern edict_s gedicts[MAX_ENTITIES];

void ent_register(const entmap_s* ent);
entity_s* ent_create(const char* classname);
void ent_remove(entity_s* ent);

void ent_think();

bool_t ent_parse(const char* pfile);

#endif