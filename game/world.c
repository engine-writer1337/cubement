#include "game.h"

static bool_t spawn_worldspawn(entity_s* pev) 
{ 
	gplayer = NULL;
	pev->contents = CONTENTS_WORLD;
	return TRUE;
}

static void precache_worldspawn(entity_s* pev) {}
static void keyvalue_worldspawn(entity_s* pev, keyvalue_s* kv) 
{
	if (strcmpi(kv->key, "skyname"))
		cment->sky_load(kv->value);
}

static void saverestore_worldspawn(entity_s* pev) {}
static void think_worldspawn(entity_s* pev) 
{
	//pev->velocity[0] += cment->frametime;
	//pev->velocity[1] += cment->frametime;
	//pev->velocity[2] += cment->frametime;
	//cment->sky_rotate(pev->velocity);
}

LINK_ENTITY(worldspawn, ENTID_WORLDSPAWN, sizeof(entity_s))