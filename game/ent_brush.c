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

SAVEFUNC void SaveMe()
{

}

static void saverestore_func_wall(entvar_s* pev) {}

LINK_ENTITY(func_wall, ENTID_WALL)