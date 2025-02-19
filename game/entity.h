#ifndef _ENTITY_H_
#define _ENTITY_H_

typedef struct
{
	entity_s base;

	bool_t is_closed;
	char area1[20];
	char area2[20];
}door_rot_s;

entity_s* ent_find_by_id(entid_e id);

#endif