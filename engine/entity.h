#ifndef _ENTITY_H_
#define _ENTITY_H_

#define STRINGPOOL		(512 * 1024)

#define MAX_KEYVALUES	64
#define MAX_ENTITIES	2048

void ent_register(const entmap_s* ent);

char* ent_string_alloc(const char* string);
void ent_string_flush();

void ent_think();

bool_t ent_parse(const char* pfile);

extern int gnuments;
extern entity_s* gents[MAX_ENTITIES];

#endif