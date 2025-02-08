#include "cubement.h"

static int gnumres;
resource_s gres[MAX_RESOURCES];

static restype_e res_type(const char* name)
{
	int len;

	len = (int)strlen(name);
	if ((*(int*)(name + len - 4) == 'gpj.') || (*(int*)(name + len - 4) == 'gnp.'))
		return RES_PIC;

	if ((*(int*)(name + len - 4) == 'tnf.'))
		return RES_FONT;

	return RES_BAD;
}

static ihandle_t res_alloc()
{
	int i;
	resource_s* r;

	for (i = 0; i < gnumres; i++)
	{
		r = gres + i;
		if (!r->name[0])
			return i;
	}

	if (gnumres == MAX_RESOURCES)
	{
		con_print(COLOR_RED, "MAX_RESOURCES");
		return BAD_HANDLE;
	}

	return gnumres++;	
}

static void res_free(ihandle_t idx)
{
	resource_s* r = gres + idx;
	if (!r->name[0])
		return;

	switch (r->type)
	{
	case RES_PIC:
		util_tex_free(r->data.pic.t);
		break;
	case RES_FONT:
		util_tex_free(r->data.font.texture);
		break;
	}

	r->name[0] = '\0';
}

bool_t res_notvalid(ihandle_t handle, restype_e type)
{
	if ((unsigned)handle >= (unsigned)gnumres)
		return TRUE;

	if (!gres[handle].name[0])
		return TRUE;

	return gres[handle].type != type;
}

ihandle_t res_find(const char* name)
{
	int i;
	hash_t hash;
	resource_s* r;

	hash = util_hash_str(name);
	for (i = 0; i < gnumres; i++)
	{
		r = gres + i;
		if (!r->name[0] || r->hash != hash)
			continue;

		if (strcmpi(r->name, name))
			return i;
	}

	return BAD_HANDLE;
}

ihandle_t res_precache_ex(const char* filename, int flags)
{
	resource_s* r;
	ihandle_t idx;
	restype_e type;

	if (!ghost.precache)
	{
		con_printf(COLOR_RED, "%s - precache is not allowed");
		return BAD_HANDLE;
	}

	if (strnull(filename))
		return BAD_HANDLE;

	idx = res_find(filename);
	if (idx != BAD_HANDLE)
		return idx;

	type = res_type(filename);
	if (type == RES_BAD)
	{
		con_printf(COLOR_RED, "%s - unknown resource");
		return BAD_HANDLE;
	}

	idx = res_alloc();
	if (idx == BAD_HANDLE)
		return BAD_HANDLE;

	r = gres + idx;
	switch (type)
	{
	case RES_PIC:
		gimg.mipmap = FALSE;
		gimg.clamp = (flags & IMG_CLAMP);
		gimg.nearest = (flags & IMG_NEAREST);

		r->data.pic.t = img_load(filename);
		if (!r->data.pic.t)
			return BAD_HANDLE;

		r->data.pic.width = gimg.out_width;
		r->data.pic.height = gimg.out_height;
		break;

	case RES_FONT:
		font_load(filename, &r->data.font);
		if (!r->data.font.texture)
			return BAD_HANDLE;
		break;
	}

	r->type = type;
	strcpyn(r->name, filename);
	r->hash = util_hash_str(r->name);
	r->is_temp = (ghost.precache == PRE_TEMP);
	return idx;
}

ihandle_t res_precache(const char* filename)
{
	return res_precache_ex(filename, 0);
}

void res_flush_temp()
{
	int i;
	resource_s* r;

	for (i = 0; i < gnumres; i++)
	{
		r = gres + i;
		if (r->name[0] && r->is_temp)
			res_free(i);
	}
}

void res_free_all()
{
	int i;

	for (i = 0; i < gnumres; i++)
		res_free(i);
}

void res_reload_font()
{
	int i;
	resource_s* r;

	for (i = 0; i < gnumres; i++)
	{
		r = gres + i;
		if (!r->name[0] || r->type != RES_FONT)
			continue;

		util_tex_free(r->data.font.texture);
		r->data.font.texture = 0;

		font_load(r->name, &r->data.font);
		if (!r->data.font.texture)
			r->name[0] = '\0';
	}
}