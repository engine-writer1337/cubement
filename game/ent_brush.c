#include "game.h"

static bool_t spawn_func_wall(entity_s* self)
{
	return TRUE;
}

static void precache_func_wall(entity_s* self)
{

}

static void keyvalue_func_wall(entity_s* self, keyvalue_s* kv)
{

}

static void saverestore_func_wall(entity_s* pev) {}

static void think_func_wall(entity_s* self) { }

LINK_ENTITY(func_wall, ENTID_WALL, sizeof(entity_s))