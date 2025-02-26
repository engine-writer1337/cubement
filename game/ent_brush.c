#include "game.h"

static bool_t spawn_func_wall(entity_s* pev)
{
	pev->contents = CONTENTS_BRUSH;
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

static void think_func_wall(entity_s* pev) 
{
	//pev->origin[0] += cment->frametime;
	//pev->origin[1] += cment->frametime;
}

LINK_ENTITY(func_wall, ENTID_WALL, sizeof(entity_s))

static void door_rot_use(door_rot_s* pev, entity_s* activator)
{
	if (pev->toggle.is_closed)
	{
		pev->toggle.ideal_yaw = 90;
		pev->base.avelocity[YAW] = 100;
		cment->area_active(pev->area1, TRUE);
		cment->area_active(pev->area2, TRUE);
		pev->toggle.is_closed = FALSE;
	}
	else
	{
		pev->toggle.ideal_yaw = 0;
		pev->base.avelocity[YAW] = -100;
		cment->area_active(pev->area1, FALSE);
		cment->area_active(pev->area2, FALSE);
		pev->toggle.is_closed = TRUE;
	}
}

static bool_t spawn_func_door_rotating(door_rot_s* pev)
{
	pev->toggle.is_closed = TRUE;
	pev->base.contents = CONTENTS_BRUSH;
	pev->base.use = door_rot_use;
	return TRUE;
}

static void precache_func_door_rotating(door_rot_s* pev)
{

}

static void keyvalue_func_door_rotating(door_rot_s* pev, keyvalue_s* kv)
{
	if (strcmpi(kv->key, "model"))
		pev->base.model = cment->resource_precache(kv->value);
	else if (strcmpi(kv->key, "rendermode"))
		pev->base.render = atoi(kv->value);
	else if (strcmpi(kv->key, "area1"))
		strcpyn(pev->area1, kv->value);
	else if (strcmpi(kv->key, "area2"))
		strcpyn(pev->area2, kv->value);
}

static void saverestore_func_door_rotating(door_rot_s* pev) {}

static void think_func_door_rotating(door_rot_s* pev)
{
	float delta = fabsf(pev->toggle.ideal_yaw - pev->base.angles[YAW]);
	if (delta < 0.001f)
		return;

	if (delta < 1)
	{
		pev->base.angles[YAW] = pev->toggle.ideal_yaw;
		cment->sound_play(glob.doorstop, ENT(pev), 0, 1, 512);
	}
	else
		ent_rotate(ENT(pev), 0);
}

LINK_ENTITY(func_door_rotating, ENTID_DOOR_ROT, sizeof(door_rot_s))
