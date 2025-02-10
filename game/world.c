#include "game.h"

static bool_t spawn_worldspawn(entity_s* pev) 
{ 
	gplayer = NULL;
	return TRUE;
}

static void precache_worldspawn(entity_s* pev) {}
static void keyvalue_worldspawn(entity_s* pev, keyvalue_s* kv) 
{
	if (strcmpc(kv->name, "skyname"))
		cment->sky_load(kv->value);
}

static void saverestore_worldspawn(entity_s* pev) {}
static void think_worldspawn(entity_s* pev) {}

LINK_ENTITY(worldspawn, ENTID_WORLDSPAWN, sizeof(entity_s))