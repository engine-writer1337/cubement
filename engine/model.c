#include "cubement.h"

void mdl_load_textures(const char* filename, model_s* mdl)
{
	int i;
	string_t cut, path;
	mstudiotexture_s* tex;

	util_filepath(filename, cut);

	gimg.mipmap = TRUE;
	gimg.clamp = FALSE;
	gimg.nearest = gimg.nofilter->value;

	tex = (mstudiotexture_s*)(mdl->heap + mdl->head->ofstextures);
	for (i = 0; i < mdl->head->numtextures; i++, tex++)
	{
		sprintf(path, MDL_FOLDER"%s%s", cut, tex->name);
		mdl->textures[i] = img_load(path);
	}
}

void mdl_load(const char* filename, model_s* mdl)
{
	string_t path;

	sprintf(path, MDL_FOLDER"%s", filename);
	mdl->heap = util_full(path, NULL);
	if (!mdl->heap)
	{
		con_printf(COLOR_RED, "%s - not found", path);
		return;
	}

	mdl->head = (studiohdr_s*)mdl->heap;
	mdl->textures = util_malloc(mdl->head->numtextures * sizeof(glpic_t));
	mdl_load_textures(filename, mdl);
}

void mdl_free(model_s* mdl, bool_t is_reload)
{
	int i;

	for (i = 0; i < mdl->head->numtextures; i++)
	{
		util_tex_free(mdl->textures[i]);
		mdl->textures[i] = 0;
	}

	if (is_reload)
		return;

	util_free(mdl->textures);
	util_free(mdl->heap);
	mdl->heap = NULL;
}