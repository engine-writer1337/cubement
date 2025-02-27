#include "cubement.h"

int gnum_details;
detail_s gdetails[MAX_MATERIALS];

static int gnum_matmaps;
static matmap_s gmatmap[MAX_MATERIALS];

static int gnum_mats;
static material_s gmats[MAX_MAT_ENRTIES];

static int gnum_detmats;
static material_s gdetmats[MAX_MAT_ENRTIES];

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

static material_e mat_find_material(const char* texturename, material_s* mats, int num_mats)
{
	int i;
	hash_t hash;
	material_s* mat;

	hash = util_hash_str(texturename);
	for (i = 0; i < num_mats; i++)
	{
		mat = mats + i;
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
		con_printf(COLOR_RED, "%s - precache materials is not allowed", filename);
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
	while (!feof(fp))
	{
		fscanf(fp, "%s", buffer);
		if (strnull(buffer))
			continue;

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
		gbru.textures[i].material = mat_find_material(gbru.textures[i].name, gmats, gnum_mats);
}

static int det_find_index(const char* str)
{
	int i;
	hash_t hash;
	detail_s* det;

	hash = util_hash_str(str);
	for (i = 0; i < gnum_details; i++)
	{
		det = gdetails + i;
		if (det->hash != hash)
			continue;

		if (strcmpi(det->name, str))
			return i;
	}

	if (i == MAX_MATERIALS)
	{
		con_print(COLOR_RED, "MAX_MATERIALS");
		return gnum_details - 1;
	}

	strcpyn(gdetails[i].name, str);
	gdetails[i].hash = hash;
	return gnum_details++;
}

void det_file(const char* filename)
{
	FILE* fp;
	material_e mat;
	string_t buffer;
	static string_t oldname;

	if (!ghost.precache)
	{
		con_printf(COLOR_RED, "%s - precache detail textures is not allowed", filename);
		return;
	}

	if (strcmpi(oldname, filename))
		return;

	det_unload();
	gnum_details = gnum_detmats = 0;
	strcpyn(oldname, filename);
	fp = util_open(filename, "r");
	if (!fp)
	{
		oldname[0] = '\0';
		return;
	}

	mat = MAT_DEFAULT;
	while (!feof(fp))
	{
		fscanf(fp, "%s", buffer);
		if (strnull(buffer))
			continue;

		if (buffer[0] == '$')
			mat = det_find_index(buffer + 1);
		else
		{
			gdetmats[gnum_detmats].material = mat;
			strcpyn(gdetmats[gnum_detmats].name, buffer);
			gdetmats[gnum_detmats].hash = util_hash_str(gdetmats[gnum_detmats].name);
			gnum_detmats++;
		}
	}

	util_close(fp);
	det_reload();
}

void det_update()
{
	int i;

	if (!gnum_detmats)
		return;

	for (i = 0; i < gbru.num_textures; i++)
		gbru.textures[i].detail = mat_find_material(gbru.textures[i].name, gdetmats, gnum_detmats);
}

void det_unload()
{
	int i;

	for (i = 0; i < gnum_details; i++)
	{
		util_tex_free(gdetails[i].texture);
		gdetails[i].texture = 0;
	}
}

void det_reload()
{
	int i;
	string_t path;

	gimg.mipmap = TRUE;
	gimg.clamp = FALSE;
	gimg.nearest = FALSE;
	for (i = 0; i < gnum_details; i++)
	{
		sprintf(path, DET_FOLDER"%s", gdetails[i].name);
		gdetails[i].texture = img_load(path);
	}
}