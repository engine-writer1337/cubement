#include "cubement.h"

static int gnum_matmaps;
static matmap_s gmatmap[MAX_MATERIALS];

static int gnum_mats;
static material_s gmats[MAX_MAT_ENRTIES];

void mat_register(const matmap_s* matmap, int num)
{
	int i;

	if (num >= MAX_MATERIALS)
		num = MAX_MATERIALS - 1;

	gnum_matmaps = num;
	memcpy(gmatmap, matmap, gnum_matmaps * sizeof(matmap_s));
	for (i = 0; i < gnum_matmaps; i++)
		gmatmap[i].hash = util_hash_str(gmatmap[i].name);
}

static material_e mat_find_index(const char* str)
{
	int i;
	hash_t hash;
	matmap_s* map;

	hash = util_hash_str(str);
	for (i = 0; i < gnum_matmaps; i++)
	{
		map = gmatmap + i;
		if (map->hash != hash)
			continue;

		if (strcmpi(map->name, str))
			return map->index;
	}

	return MAT_DEFAULT;
}

static material_e mat_find_material(const char* texturename)
{
	int i;
	hash_t hash;
	material_s* mat;

	hash = util_hash_str(texturename);
	for (i = 0; i < gnum_mats; i++)
	{
		mat = gmats + i;
		if (mat->hash != hash)
			continue;

		if (strcmpi(mat->name, texturename))
			return mat->material;
	}

	return MAT_DEFAULT;
}

void mat_file(const char* filename)
{
	FILE* fp;
	material_e mat;
	string_t buffer;
	static string_t oldname;

	if (!ghost.precache)
	{
		con_printf(COLOR_RED, "%s - precache materials is not allowed");
		return;
	}

	if (strcmpi(oldname, filename))
		return;

	gnum_mats = 0;
	strcpyn(oldname, filename);
	fp = util_open(filename, "r");
	if (!fp)
	{
		oldname[0] = '\0';
		return;
	}

	mat = MAT_DEFAULT;
	while (fscanf(fp, "%s", buffer) == 1)
	{
		if (buffer[0] == '$')
			mat = mat_find_index(buffer);
		else
		{
			gmats[gnum_mats].material = mat;
			strcpyn(gmats[gnum_mats].name, buffer);
			gmats[gnum_mats].hash = util_hash_str(gmats[gnum_mats].name);
			gnum_mats++;
		}
	}

	util_close(fp);
}

void mat_update()
{
	int i;

	for (i = 0; i < gbru.num_textures; i++)
		gbru.textures[i].material = mat_find_material(gbru.textures[i].name);
}