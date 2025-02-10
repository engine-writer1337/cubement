#ifndef _RESOURCE_H_
#define _RESOURCE_H_

#define MAX_RESOURCES	5000

typedef enum
{
	RES_BAD = -1,
	RES_PIC,
	RES_FONT,
	RES_SPRITE,
	RES_MODEL,
	RES_SOUND,
	RES_BRUSH,
}restype_e;

typedef struct
{
	name_t name;
	bool_t is_temp;
	restype_e type;
	hash_t hash;

	union
	{
		pic_s pic;
		font_s font;
		brushmodel_s* brush;
	}data;
}resource_s;

extern resource_s gres[MAX_RESOURCES];

bool_t res_notvalid(ihandle_t handle, restype_e type);
ihandle_t res_find(const char* name);
ihandle_t res_precache_ex(const char* filename, int flags);
ihandle_t res_precache(const char* filename);
void res_flush_temp();
void res_free_all();
void res_reload_font();

void res_unload();
void res_reload();

#endif