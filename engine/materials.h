#ifndef _MATERIALS_H_
#define _MATERIALS_H_

#define MAX_MAT_ENRTIES		2048

typedef struct
{
	hash_t hash;
	char name[32];
	material_e material;
}material_s;

void mat_register(const matmap_s* matmap, int num);
void mat_file(const char* filename);
void mat_update();

#endif