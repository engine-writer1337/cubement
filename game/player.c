#include "game.h"

static bool_t spawn_player(entity_s* self)
{
	return TRUE;
}

static void precache_player(entity_s* self)
{

}

static void keyvalue_player(entity_s* self, keyvalue_s* kv)
{

}

static void think_player(entity_s* self) 
{
	cment->set_view_fov(90);
}

static void saverestore_player(entbase_s* pev) {}

LINK_ENTITY(player, ENTID_PLAYER, sizeof(entbase_s))

static bool_t spawn_worldspawn(entity_s* self)
{
	return TRUE;
}

static void precache_worldspawn(entity_s* self)
{

}

static void keyvalue_worldspawn(entity_s* self, keyvalue_s* kv)
{

}

static void saverestore_worldspawn(entbase_s* pev) {}

static void think_worldspawn(entity_s* self) { }

LINK_ENTITY(worldspawn, ENTID_WORLDSPAWN, sizeof(entbase_s))