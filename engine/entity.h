#ifndef _ENTITY_H_
#define _ENTITY_H_

#define STRINGPOOL		(512 * 1024)

#define MAX_KEYVALUES	64
#define MAX_ENTITIES	2048

typedef struct _edict_s
{
	entity_s* e;

	bool_t areas[MAX_MAP_AREAS];

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

char* ent_string_alloc(const char* string);
void ent_string_flush();

void ent_think();

bool_t ent_parse(const char* pfile);



#endif