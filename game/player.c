#include "game.h"

player_s* gplayer;

static bool_t spawn_player(player_s* pev)
{
	entity_s* spawn;

	gplayer = pev;
	spawn = ent_find_by_id(ENTID_START);
	if (spawn)
	{
		vec_copy(pev->base.origin, spawn->origin);
		pev->base.angles[YAW] = spawn->angles[YAW];
	}

	pev->viewofs = PLR_VIEW_OFS;
	pev->base.contents = CONTENTS_BBOX;
	return TRUE;
}

static void precache_player(player_s* pev)
{

}

static void keyvalue_player(player_s* pev, keyvalue_s* kv)
{

}

static void think_player(player_s* pev)
{
	plr_inputs(pev);
	plr_move(pev);

	if ((pev->buttons & IN_USE) && pev->nextuse < cment->gametime)
	{
		trace_s tr;
		vec3_t endpos;

		vec_ma(endpos, pev->vieworg, 128, pev->v_forward);
		cment->trace_bbox(pev->vieworg, endpos, gvec_zeros, gvec_zeros, pev, CONTENTS_SOLID, &tr);
		if (tr.ent && tr.ent->use)
			tr.ent->use(tr.ent, pev);

		pev->nextuse = cment->gametime + 0.2f;
	}

	cment->set_view_org(pev->vieworg);
	cment->set_view_ang(pev->base.angles);
	cment->set_view_fov(clamp(50, glob.fov->value, 150));
}

static void saverestore_player(player_s* pev) {}

LINK_ENTITY(player, ENTID_PLAYER, sizeof(player_s))

static bool_t spawn_info_player_start(entity_s* pev) { return TRUE; }
static void precache_info_player_start(entity_s* pev) {}
static void keyvalue_info_player_start(entity_s* pev, keyvalue_s* kv)
{
	if (strcmpi(kv->key, "origin"))
		vec_from_str(pev->origin, kv->value);
	else if (strcmpi(kv->key, "angles"))
		vec_from_str(pev->angles, kv->value);
}

static void saverestore_info_player_start(entity_s* pev) {}
static void think_info_player_start(entity_s* pev) { }

LINK_ENTITY(info_player_start, ENTID_START, sizeof(entity_s))