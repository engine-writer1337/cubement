#include "game.h"

entity_s* ent_find_by_id(entid_e id)
{
	int i;
	entity_s* e;

	for (i = 0; i < cment->entities_max; i++)
	{
		e = cment->entities[i];
		if (!e || e->id == ENTID_FREE)
			continue;

		if (e->id == id)
			return e;
	}

	return NULL;
}