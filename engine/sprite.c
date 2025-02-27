#include "cubement.h"

void spr_load_pic(const char* filename, sprite_s* spr)
{
	string_t cut, path;

	strncpy(cut, filename, strlen(filename) - 4);
	sprintf(path, SPR_FOLDER"%s", cut);
	spr->texture = img_load(path);
}

void spr_load(const char* filename, sprite_s* spr)
{
	int i;
	FILE* fp;
	string_t path;
	sframe_s* frame;

	sprintf(path, SPR_FOLDER"%s", filename);
	fp = util_open(path, "r");
	if (!fp)
	{
		con_printf(COLOR_RED, "%s - not found", path);
		return;
	}

	fscanf(fp, "%i", &spr->numframes);
	if (spr->numframes < 1)
	{
		con_printf(COLOR_RED, "%s - no frames", path);
		util_close(fp);
		return;
	}

	if (spr->numframes > 4096)
		spr->numframes = 4096;

	spr->frames = util_malloc(spr->numframes * sizeof(sframe_s));
	for (i = 0; i < spr->numframes; i++)
	{
		frame = spr->frames + i;
		fscanf(fp, "%i %i %i %i", &frame->x, &frame->y, &frame->w, &frame->h);
	}

	util_close(fp);
	spr_load_pic(filename, spr);
	if (!spr->texture)
		spr_free(spr, FALSE);
}

void spr_free(sprite_s* spr, bool_t is_reload)
{
	util_tex_free(spr->texture);
	spr->texture = 0;
		
	if (is_reload)
		return;

	util_free(spr->frames);
	spr->numframes = 0;
	spr->frames = NULL;
}