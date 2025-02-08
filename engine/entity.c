#include "cubement.h"
#include <setjmp.h>

static int gstringpos;
static char gstringpool[STRINGPOOL];
static entmap_s gentmap[MAX_ENTMAP];

int gnuments;
static jmp_buf gent_jump;
entity_s gents[MAX_ENTITIES];

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
// PARSE
//=============================================================//
static bool_t ent_parse_ent(const char** pfile, entity_s* ent)
{
	bool_t found;
	entmap_s* map;
	name_t keyname;
	string_t token;
	int i, numpairs;
	keyvalue_s kv[MAX_KEYVALUES];

	numpairs = 0;
	found = FALSE;
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

		if (!found && strcmpc(keyname, "classname"))
		{
			map = ent_find_map(token);
			if (!map)
			{
				con_printf(COLOR_RED, "%s - not registered", token);
				continue;
			}

			found = TRUE;
			ent->id = map->id;
			if (map->pev_size)
				ent->pev = util_calloc(map->pev_size, sizeof(byte));
		}
		else
		{
			if (numpairs == MAX_KEYVALUES)
				continue;

			strcpyn(kv[numpairs].name, keyname);
			strcpyn(kv[numpairs].value, token);
			numpairs++;
		}
	}

	if (!found)
		return FALSE;

	for (i = 0; i < numpairs; i++)
		gentmap[ent->id].keyvalue(ent, &kv[i]);
	return TRUE;
}

bool_t ent_parse(const char* pfile)
{
	entmap_s* map;
	entity_s* ent;
	bool_t isworld;
	string_t token;

	if (setjmp(gent_jump))
		return FALSE;

	if (!gentmap[ENTID_WORLDSPAWN].name || !gentmap[ENTID_PLAYER].name)
	{
		con_printf(COLOR_RED, "Main entities are not registered");
		return FALSE;
	}

	gnuments = 2;
	gentworld = gents;
	gentplayer = gents + 1;
	vec_copy(gentworld->mins, gbru.models[0].mins);
	vec_copy(gentworld->maxs, gbru.models[0].maxs);
	isworld = TRUE;

	while ((pfile = util_parse(pfile, token)))
	{
		if (token[0] != '{')
		{
			con_printf(COLOR_RED, "ent_parse: found %s when expecting {", token);
			longjmp(gent_jump, TRUE);
		}

		if (gnuments == MAX_ENTITIES)
		{
			con_print(COLOR_RED, "MAX_ENTITIES");
			break;
		}

		if (isworld)
		{
			isworld = FALSE;
			ent = gentworld;
		}
		else
			ent = &gents[gnuments];

		if (ent_parse_ent(&pfile, ent))
		{
			gentmap[ent->id].precache(ent);
			if (gentmap[ent->id].spawn(ent))
				gnuments++;
		}
	}

	map = gentmap + ENTID_PLAYER;
	if (map->pev_size)
		gentplayer->pev = util_calloc(map->pev_size, sizeof(byte));

	gentplayer->id = map->id;
	map->precache(gentplayer);
	map->spawn(gentplayer);
	return TRUE;
}