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

	struct _edict_s* next;
}edict_s;

extern int gnuments;
extern entity_s* gents[MAX_ENTITIES];

extern edict_s gedicts[MAX_ENTITIES];

void ent_register(const entmap_s* ent);

void ent_think();

bool_t ent_parse(const char* pfile);



#endif