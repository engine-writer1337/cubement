#ifndef _MODEL_H_
#define _MODEL_H_

#include "tools/make_mdl/studio.h"

#define MDL_FOLDER		"models/"

typedef struct
{
	byte* heap;
	studiohdr_s* head;
	glpic_t* textures;
}model_s;

void mdl_load_textures(const char* filename, model_s* mdl);
void mdl_load(const char* filename, model_s* mdl);
void mdl_free(model_s* mdl, bool_t is_reload);

#endif