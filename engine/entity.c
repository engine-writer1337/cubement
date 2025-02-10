#include "cubement.h"
#include <setjmp.h>

static int gstringpos;
static char gstringpool[STRINGPOOL];
static entmap_s gentmap[MAX_ENTMAP];

int gnuments;
static jmp_buf gent_jump;
entity_s* gents[MAX_ENTITIES];

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
	if ((unsigned)ent->id >= MAX_ENTMAP)
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
		gnuments++;
	else
	{
		if (gents[i])
			util_free(gents[i]);
	}

	gents[i] = util_calloc(map->pev_size, sizeof(byte));
	gents[i]->id = map->id;
	return gents[i];
}

void ent_remove(entity_s* ent)
{
	if (ent)
		ent->id = ENTID_FREE;
}

//=============================================================//
// STRING POOL
//=============================================================//
char* ent_string_alloc(const char* string)
{
	char* p;
	int len;

	len = (int)strlen(string) + 1;
	if (gstringpos + len >= STRINGPOOL)
	{
		con_print(COLOR_RED, "Failed to allocate string");
		return NULL;
	}

	p = gstringpool + gstringpos;
	gstringpos += (len + 3) & ~3;
	strcpy(p, string);
	return p;
}

void ent_string_flush()
{
	gstringpos = 0;
}

//=============================================================//
// THINK
//=============================================================//
void ent_think()
{
	int i;
	entity_s* e;

	for (i = 0; i < gnuments; i++)
	{
		e = gents[i];
		if (e && e->nextthink < ghost.gametime)
			gentmap[e->id].think(e);
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

			strcpyn(kv[numpairs].name, keyname);
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
			map = gentmap + ENTID_WORLDSPAWN;
		}
		else
		{
			ent = ent_create(classname);
			if (!ent)
				continue;
		}

		for (i = 0; i < numpairs; i++)
			map->keyvalue(ent, kv + i);

		map->precache(ent);
		if (!map->spawn(ent))
			ent_remove(ent);
	}

	map = gentmap + ENTID_PLAYER;
	gentplayer->id = ENTID_PLAYER;
	map->precache(gentplayer);
	map->spawn(gentplayer);
	return TRUE;
}