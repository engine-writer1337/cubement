#ifndef _ENTITY_H_
#define _ENTITY_H_

#define ENT(ent)	&ent->base

typedef struct
{
	entity_s base;

	float ideal_yaw;
	bool_t is_closed;
}entity_toggle_s;

typedef struct
{
	union
	{
		entity_s base;
		entity_toggle_s toggle;
	};

	char area1[20];
	char area2[20];
}door_rot_s;

entity_s* ent_find_by_id(entid_e id);

entity_s* ent_rotate(entity_s* pev, float friction);

#endif