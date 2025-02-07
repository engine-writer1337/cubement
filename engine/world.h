#ifndef _WORLD_H_
#define _WORLD_H_

typedef struct
{
	bool_t is_load;
}world_s;

extern world_s gworld;

void world_map_cmd(const char* arg1, const char* arg2);
void world_load_map();

#endif