#include "cubement.h"
#include <setjmp.h>

static entmap_s gentmap[MAX_ENTMAP];

int gnuments;
static jmp_buf gent_jump;
entity_s* gents[MAX_ENTITIES];
edict_s gedicts[MAX_ENTITIES];

static entity_s* gentplayer;

//=============================================================//
// ENTITIES
//=============================================================//
static entmap_s* ent_find_map(const char* name)
{
	int i;
	hash_t hash;
	entmap_s* map;

	hash = util_hash_str(name);
	for (i = 0; i < MAX_ENTMAP; i++)
	{
		map = gentmap + i;
		if (!map->name || map->hash != hash)
			continue;

		if (strcmpi(map->name, name))
			return map;
	}

	return NULL;
}

void ent_register(const entmap_s* ent)
{
	if ((dword)ent->id >= MAX_ENTMAP)
		return;

	if (gentmap[ent->id].name || ent_find_map(ent->name))
	{
		con_printf(COLOR_RED, "%s - already registered", ent->name);
		return;
	}

	memcpy(gentmap + ent->id, ent, sizeof(entmap_s));
	gentmap[ent->id].hash = util_hash_str(gentmap[ent->id].name);
}

entity_s* ent_create(const char* classname)
{
	int i;
	entmap_s* map;

	map = ent_find_map(classname);
	if (!map)
	{
		con_printf(COLOR_RED, "%s - not registered", classname);
		return NULL;
	}

	for (i = 0; i < gnuments; i++)
	{
		if (!gents[i] || gents[i]->id == ENTID_FREE)
			break;
	}

	if (i == MAX_ENTITIES)
	{
		con_print(COLOR_RED, "MAX_ENTITIES");
		return NULL;
	}

	if (i == gnuments)
	{
		gnuments++;
		gengine.entities_max = gnuments;
	}
	else
	{
		if (gents[i])
			util_free(gents[i]);
	}

	gents[i] = util_calloc(map->pev_size, sizeof(byte));
	memzero(&gedicts[i], sizeof(gedicts[i]));
	gedicts[i].old.model = BAD_HANDLE;
	gedicts[i].e = gents[i];

	gents[i]->id = map->id;
	gents[i]->model = BAD_HANDLE;
	gents[i]->renderamt = 255;
	return gents[i];
}

static edict_s* ent_get_edict(entity_s* ent)
{
	int i;

	for (i = 0; i < gnuments; i++)
	{
		if (gents[i] == ent)
			return gedicts + i;
	}

	return NULL;
}

void ent_remove(entity_s* ent)
{
	edict_s* ed;

	if (!ent || ent->id == ENTID_FREE || ent->id == ENTID_WORLDSPAWN || ent->id == ENTID_PLAYER)
		return;

	ent->id = ENTID_FREE;
	ed = ent_get_edict(ent);
	if (ed)
		ed->e = NULL;
}

//=============================================================//
// THINK
//=============================================================//
static void ent_fill_areas(edict_s* ed)
{
	int i, j;
	area_s* a;
	entity_s* e;
	vec3_t absmin, absmax;
	bool_t anyintersect = FALSE;

	e = ed->e;
	ed->num_areas = 0;
	vec_add_val(absmax, e->absmax, TRACE_EPSILON);
	vec_add_val(absmin, e->absmin, -TRACE_EPSILON);

	for (i = 1; i < gbru.num_areas; i++)
	{
		a = gbru.areas + i;
		for (j = 0; j < a->num_boxes; j++)
		{
			if (vec2_aabb(a->absmin[j], a->absmax[j], absmin, absmax))
				continue;

			ed->areas[ed->num_areas++] = i;
			anyintersect = TRUE;
			break;
		}
	}

	if (!anyintersect)
		ed->areas[ed->num_areas++] = 0;
}

static void ent_update(edict_s* ed)
{
	bool_t relink;
	brushmodel_s* bm;
	entity_s* e = ed->e;

	if (!e)
		return;

	relink = FALSE;
	if (ed->old.model != e->model)
	{
		relink = TRUE;
		ed->old.model = e->model;

		if (e->model != BAD_HANDLE && gres[e->model].type == RES_BRUSH)
		{
			bm = gres[e->model].brush;
			vec_copy(e->origin, bm->origin);
			vec_copy(e->maxs, bm->maxs);
			vec_copy(e->mins, bm->mins);
		}
	}

	if (!vec_cmp(ed->old.origin, e->origin))
	{
		vec_copy(ed->old.origin, e->origin);
		relink = TRUE;
	}

	if (!vec_cmp(ed->old.angles, e->angles))
	{
		if (e->model != BAD_HANDLE && gres[e->model].type == RES_BRUSH)
		{
			e->angles[0] = anglemod(e->angles[0]);
			e->angles[1] = anglemod(e->angles[1]);
			e->angles[2] = anglemod(e->angles[2]);

			bm = gres[e->model].brush;
			vec_copy(e->maxs, bm->maxs);
			vec_copy(e->mins, bm->mins);
			vec_rotate_bbox(e->angles, bm->offset, e->mins, e->maxs);
		}

		vec_copy(ed->old.angles, e->angles);
		relink = TRUE;
	}

	if (!vec_cmp(ed->old.mins, e->mins) || !vec_cmp(ed->old.maxs, e->maxs))
	{
		vec_copy(ed->old.mins, e->mins);
		vec_copy(ed->old.maxs, e->maxs);
		relink = TRUE;
	}

	if (relink)
	{
		vec_add(e->absmax, e->origin, e->maxs);
		vec_add(e->absmin, e->origin, e->mins);
		vec_copy(ed->old.absmax, e->absmax);
		vec_copy(ed->old.absmin, e->absmin);

		if (e->flags & FL_TEMP)
		{
			ed->num_areas = 1;
			ed->areas[0] = 0;
		}
		else
			ent_fill_areas(ed);
	}
	else
	{
		if (!vec_cmp(ed->old.absmin, e->absmin) || !vec_cmp(ed->old.absmax, e->absmax))
		{
			vec_copy(e->absmin, ed->old.absmin);
			vec_copy(e->absmax, ed->old.absmax);
		}
	}
}

void ent_think()
{
	int i;
	entity_s* e;
	edict_s* ed;

	for (i = 0; i < gnuments; i++)
	{
		ed = gedicts + i;
		if (!ed->e)
			continue;

		e = ed->e;
		gentmap[e->id].think(e);
		ent_update(ed);
	}
}

//=============================================================//
// PARSE
//=============================================================//
static int ent_parse_ent(const char** pfile, keyvalue_s* kv, char* classname)
{
	int numpairs;
	name_t keyname;
	string_t token;

	numpairs = 0;
	classname[0] = '\0';

	while (TRUE)
	{
		*pfile = util_parse(*pfile, token);
		if (!*pfile)
		{
			con_print(COLOR_RED, "ent_parse_ent: EOF without closing brace");
			longjmp(gent_jump, TRUE);
		}

		if (token[0] == '}')
			break;

		strcpyn(keyname, token);
		*pfile = util_parse(*pfile, token);
		if (!*pfile)
		{
			con_print(COLOR_RED, "ent_parse_ent: EOF without closing brace");
			longjmp(gent_jump, TRUE);
		}

		if (token[0] == '}')
		{
			con_print(COLOR_RED, "ent_parse_ent: closing brace without data");
			longjmp(gent_jump, TRUE);
		}

		if (!keyname[0] || !token[0])
			continue;

		if (strcmpc(keyname, "classname"))
			strcpy(classname, token);
		else
		{
			if (numpairs == MAX_KEYVALUES)
				continue;

			strcpyn(kv[numpairs].key, keyname);
			strcpyn(kv[numpairs].value, token);
			numpairs++;
		}
	}

	return numpairs;
}

bool_t ent_parse(const char* pfile)
{
	entmap_s* map;
	entity_s* ent;
	bool_t isworld;
	string_t token;
	int i, numpairs;
	char classname[256];
	keyvalue_s kv[MAX_KEYVALUES];

	if (setjmp(gent_jump))
		return FALSE;

	if (!gentmap[ENTID_WORLDSPAWN].name || !gentmap[ENTID_PLAYER].name)
	{
		con_printf(COLOR_RED, "Main entities are not registered");
		return FALSE;
	}

	gnuments = 2;
	gents[0] = util_calloc(gentmap[ENTID_WORLDSPAWN].pev_size, sizeof(byte));
	gents[1] = util_calloc(gentmap[ENTID_PLAYER].pev_size, sizeof(byte));
	gedicts[0].e = gents[0];
	gedicts[1].e = gents[1];
	gentworld = gents[0];
	gentplayer = gents[1];
	isworld = TRUE;

	while ((pfile = util_parse(pfile, token)))
	{
		if (token[0] != '{')
		{
			con_printf(COLOR_RED, "ent_parse: found %s when expecting {", token);
			longjmp(gent_jump, TRUE);
		}

		numpairs = ent_parse_ent(&pfile, kv, classname);
		if (!numpairs || classname[0] == '\0')
			continue;

		if (isworld)
		{
			isworld = FALSE;
			ent = gentworld;
			ent->id = ENTID_WORLDSPAWN;
			vec_copy(ent->mins, gbru.models[0].mins);
			vec_copy(ent->maxs, gbru.models[0].maxs);
		}
		else
		{
			ent = ent_create(classname);
			if (!ent)
				continue;
		}

		map = gentmap + ent->id;
		for (i = 0; i < numpairs; i++)
			map->keyvalue(ent, kv + i);

		map->precache(ent);
		if (!map->spawn(ent))
			ent_remove(ent);
		else
			ent_update(ent_get_edict(ent));
	}

	map = gentmap + ENTID_PLAYER;
	gentplayer->id = ENTID_PLAYER;
	map->precache(gentplayer);
	map->spawn(gentplayer);
	ent_update(&gedicts[1]);
	return TRUE;
}