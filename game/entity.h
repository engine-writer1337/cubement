#ifndef _ENTITY_H_
#define _ENTITY_H_

typedef struct
{
	struct entity_t* owner;
}ent_base_s;

typedef struct
{
	ent_base_s base;
}ent_func_wall_s;

typedef struct
{
	ent_base_s base;
}ent_worldspawn_s;

typedef struct
{
	ent_base_s base;
}ent_player_s;

#endif