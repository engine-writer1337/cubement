#ifndef _MATERIALS_H_
#define _MATERIALS_H_

#define MAX_MAT_ENRTIES		4096
#define DET_FOLDER			"detail/"

typedef struct
{
	hash_t hash;
	char name[32];
	material_e material;
}material_s;

extern int gnum_details;
extern detail_s gdetails[MAX_MATERIALS];

void mat_register(const matmap_s* matmap, int num);
void mat_file(const char* filename);
void mat_update();

void det_file(const char* filename);
void det_update();
void det_unload();
void det_reload();

#endif