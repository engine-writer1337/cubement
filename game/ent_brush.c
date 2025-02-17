#include "game.h"

static bool_t spawn_func_wall(entity_s* pev)
{
	pev->contents = CONTENTS_SOLID;
	return TRUE;
}

static void precache_func_wall(entity_s* pev)
{

}

static void keyvalue_func_wall(entity_s* pev, keyvalue_s* kv)
{
	if (strcmpi(kv->key, "model"))
		pev->model = cment->resource_precache(kv->value);
	else if (strcmpi(kv->key, "rendermode"))
		pev->render = atoi(kv->value);
}

static void saverestore_func_wall(entity_s* pev) {}

static void think_func_wall(entity_s* pev) { }

LINK_ENTITY(func_wall, ENTID_WALL, sizeof(entity_s))


static bool_t spawn_func_door_rotating(entity_s* pev)
{
	pev->angles[YAW] = 90;
	pev->contents = CONTENTS_SOLID;
	return TRUE;
}

static void precache_func_door_rotating(entity_s* pev)
{

}

static void keyvalue_func_door_rotating(entity_s* pev, keyvalue_s* kv)
{
	if (strcmpi(kv->key, "model"))
		pev->model = cment->resource_precache(kv->value);
	else if (strcmpi(kv->key, "rendermode"))
		pev->render = atoi(kv->value);
}

static void saverestore_func_door_rotating(entity_s* pev) {}

static void think_func_door_rotating(entity_s* pev) 
{

}

LINK_ENTITY(func_door_rotating, ENTID_DOOR_ROT, sizeof(entity_s))
