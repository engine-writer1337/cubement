#include "cubement.h"
#include <setjmp.h>

bru_s gbru;
vertbuf_s gvertbuf;
static jmp_buf gbru_jump;
static bru_surf_s* gsurfs;
static tmp_texinfo_s* gtexinfo;
static byte gcolrow[] = { 0, 32, 64, 96, 128, 160, 192, 224, 255, 255, 255, 255, 255, 255, 255, 255 };

static void bru_load_ent(const byte* data, const bru_lump_s* l)
{
	if (l->len < 1)
	{
		con_print(COLOR_RED, "LUMP_ENTITIES bad size");
		longjmp(gbru_jump, TRUE);
	}

	if (!ent_parse(data + l->ofs))
		longjmp(gbru_jump, TRUE);
}

static void bru_load_brushareas(const byte* data, const bru_lump_s* l)
{
	if (l->len < 1 || (l->len % sizeof(word)))
	{
		con_print(COLOR_RED, "LUMP_BRUSHAREAS bad size");
		longjmp(gbru_jump, TRUE);
	}

	gbru.brushareas = util_malloc(l->len);
	memcpy(gbru.brushareas, data + l->ofs, l->len);
}

static void bru_load_areaboxes(const byte* data, const bru_lump_s* l)
{
	int i, numboxes;
	bru_areabox_s* in;

	if (l->len < 1 || (l->len % sizeof(*in)))
	{
		con_print(COLOR_RED, "LUMP_AREABOXES bad size");
		longjmp(gbru_jump, TRUE);
	}

	in = (bru_areabox_s*)(data + l->ofs);
	numboxes = l->len / sizeof(*in);
	gbru.box_mins = util_malloc(numboxes * sizeof(vec2_t));
	gbru.box_maxs = util_malloc(numboxes * sizeof(vec2_t));

	for (i = 0; i < numboxes; i++)
	{
		vec2_copy(gbru.box_mins[i], in[i].mins);
		vec2_copy(gbru.box_maxs[i], in[i].maxs);
	}
}

static void bru_load_faces(const byte* data, const bru_lump_s* l)
{
	if (l->len < 1 || (l->len % sizeof(bru_surf_s)))
	{
		con_print(COLOR_RED, "LUMP_SURFACES bad size");
		longjmp(gbru_jump, TRUE);
	}

	gbru.num_surfes = l->len / sizeof(bru_surf_s);
	gbru.surfes = util_malloc(gbru.num_surfes * sizeof(surf_s));

	gsurfs = util_malloc(l->len);
	memcpy(gsurfs, data + l->ofs, l->len);

	gvertbuf.count = gbru.num_surfes << 2;
	gvertbuf.st = util_malloc(gvertbuf.count * sizeof(vec2_t)); 
	gvertbuf.xyz = util_malloc(gvertbuf.count * sizeof(vec3_t));
}

static void bru_load_textures(const byte* data, const bru_lump_s* l)
{
	int i;
	string_t tmp;
	btexture_s* out;
	bru_texture_s* in;

	if (l->len < 1 || (l->len % sizeof(*in)))
	{
		con_print(COLOR_RED, "LUMP_TEXTURES bad size");
		longjmp(gbru_jump, TRUE);
	}

	in = (bru_texture_s*)(data + l->ofs);
	gbru.num_textures = l->len / sizeof(*in);
	out = gbru.textures = util_malloc(gbru.num_textures * sizeof(*out));

	gimg.mipmap = TRUE;
	gimg.clamp = FALSE;
	gimg.nearest = gimg.nofilter->value;

	for (i = 0; i < gbru.num_textures; i++)
	{
		sprintf(tmp, TEXTURE_FOLDER"%s", in[i].name);
		strcpyn(out[i].name, in[i].name);

		out[i].t = img_load(tmp);
		out[i].width = gimg.out_width;
		out[i].height = gimg.out_height;

		//TODO: загрузка анимации если есть файл с расширением '.a'
	}
}

static void bru_load_texinfos(const byte* data, const bru_lump_s* l)
{
	int i, numtexinfo;
	bru_texinfo_s* in;
	tmp_texinfo_s* out;

	if (l->len < 1 || (l->len % sizeof(*in)))
	{
		con_print(COLOR_RED, "LUMP_TEXINFO bad size");
		longjmp(gbru_jump, TRUE);
	}

	in = (bru_texinfo_s*)(data + l->ofs);
	numtexinfo = l->len / sizeof(*in);
	out = gtexinfo = util_malloc(numtexinfo * sizeof(*out));

	for (i = 0; i < numtexinfo; i++)
	{
		out[i].texture = in[i].texture;
		out[i].vecs[0][0] = TEXCOORD_SCALE_INV * in[i].vecs[0][0];
		out[i].vecs[0][1] = TEXCOORD_SCALE_INV * in[i].vecs[0][1];
		out[i].vecs[0][2] = TEXCOORD_SCALE_INV * in[i].vecs[0][2];
		out[i].vecs[0][3] = TEXCOORD_OFFSET_INV * in[i].vecs[0][3];
		out[i].vecs[1][0] = TEXCOORD_SCALE_INV * in[i].vecs[1][0];
		out[i].vecs[1][1] = TEXCOORD_SCALE_INV * in[i].vecs[1][1];
		out[i].vecs[1][2] = TEXCOORD_SCALE_INV * in[i].vecs[1][2];
		out[i].vecs[1][3] = TEXCOORD_OFFSET_INV * in[i].vecs[1][3];
	}
}

static void bru_create_face(int index, surf_s* out, const vec3_t mins, const vec3_t maxs)
{
	int w, h, i;
	bru_surf_s* in;
	float* st, * xyz;
	tmp_texinfo_s* texinfo = NULL;

	in = gsurfs + index;
	texinfo = gtexinfo + in->texinfo;

	out->offset = index << 2;
	out->type = in->encode >> 12;
	out->texture = texinfo->texture;
	vec4_set(out->color, gcolrow[in->encode & 15], gcolrow[(in->encode >> 4) & 15], gcolrow[(in->encode >> 8) & 15], 255);

	w = gbru.textures[texinfo->texture].width;
	h = gbru.textures[texinfo->texture].height;
	st = (float*)(gvertbuf.st + out->offset);
	xyz = (float*)(gvertbuf.xyz + out->offset);

	for (i = 3; i >= 0; i--)
	{
		switch (out->type)
		{
		case SURF_TYPE_X:
			switch (i)
			{
			case 0: vec_set(xyz, maxs[0], mins[1], mins[2]); break;
			case 1: vec_set(xyz, maxs[0], maxs[1], mins[2]); break;
			case 2: vec_set(xyz, maxs[0], maxs[1], maxs[2]); break;
			case 3: vec_set(xyz, maxs[0], mins[1], maxs[2]); break;
			}
			break;

		case SURF_TYPE_Y:
			switch (i)
			{
			case 0: vec_set(xyz, maxs[0], maxs[1], mins[2]); break;
			case 1: vec_set(xyz, mins[0], maxs[1], mins[2]); break;
			case 2: vec_set(xyz, mins[0], maxs[1], maxs[2]); break;
			case 3: vec_set(xyz, maxs[0], maxs[1], maxs[2]); break;
			}
			break;

		case SURF_TYPE_Z:
			switch (i)
			{
			case 0: vec_set(xyz, maxs[0], maxs[1], maxs[2]); break;
			case 1: vec_set(xyz, mins[0], maxs[1], maxs[2]); break;
			case 2: vec_set(xyz, mins[0], mins[1], maxs[2]); break;
			case 3: vec_set(xyz, maxs[0], mins[1], maxs[2]); break;
			}
			break;

		case SURF_TYPE_SX:
			switch (i)
			{
			case 0: vec_set(xyz, mins[0], mins[1], maxs[2]); break;
			case 1: vec_set(xyz, mins[0], maxs[1], maxs[2]); break;
			case 2: vec_set(xyz, mins[0], maxs[1], mins[2]); break;
			case 3: vec_set(xyz, mins[0], mins[1], mins[2]); break;
			}
			break;

		case SURF_TYPE_SY:
			switch (i)
			{
			case 0: vec_set(xyz, maxs[0], mins[1], maxs[2]); break;
			case 1: vec_set(xyz, mins[0], mins[1], maxs[2]); break;
			case 2: vec_set(xyz, mins[0], mins[1], mins[2]); break;
			case 3: vec_set(xyz, maxs[0], mins[1], mins[2]); break;
			}
			break;

		case SURF_TYPE_SZ:
			switch (i)
			{
			case 0: vec_set(xyz, mins[0], maxs[1], mins[2]); break;
			case 1: vec_set(xyz, maxs[0], maxs[1], mins[2]); break;
			case 2: vec_set(xyz, maxs[0], mins[1], mins[2]); break;
			case 3: vec_set(xyz, mins[0], mins[1], mins[2]); break;
			}
			break;
		}

		st[0] = (vec_dot(xyz, texinfo->vecs[0]) + texinfo->vecs[0][3]) / w;
		st[1] = (vec_dot(xyz, texinfo->vecs[1]) + texinfo->vecs[1][3]) / h;

		st += 2;
		xyz += 3;
	}
}

static void bru_load_brushes(const byte* data, const bru_lump_s* l)
{
	int i, j;
	brush_s* out;
	bru_brush_s* in;

	if (l->len < 1 || (l->len % sizeof(*in)))
	{
		con_print(COLOR_RED, "LUMP_BRUSHES bad size");
		longjmp(gbru_jump, TRUE);
	}

	in = (bru_brush_s*)(data + l->ofs);
	gbru.num_brushes = l->len / sizeof(*in);
	out = gbru.brushes = util_malloc(gbru.num_brushes * sizeof(*out));

	for (i = 0; i < gbru.num_brushes; i++)
	{
		vec_copy(out[i].mins, in[i].mins);
		vec_copy(out[i].maxs, in[i].maxs);
		out[i].num_surfes = in[i].num_surfaces;
		out[i].surfes = gbru.surfes + in[i].start_surface;

		for (j = 0; j < out[i].num_surfes; j++)
			bru_create_face(in[i].start_surface + j, out[i].surfes + j, out[i].mins, out[i].maxs);
	}
}

static void bru_load_models(const byte* data, const bru_lump_s* l)
{
	int i;
	bru_model_s* in;
	brushmodel_s* out;

	if (l->len < 1 || (l->len % sizeof(*in)))
	{
		con_print(COLOR_RED, "LUMP_MODELS bad size");
		longjmp(gbru_jump, TRUE);
	}

	in = (bru_model_s*)(data + l->ofs);
	gbru.num_models = l->len / sizeof(*in);
	out = gbru.models = util_malloc(gbru.num_models * sizeof(*out));

	for (i = 0; i < gbru.num_models; i++)
	{
		vec_copy(out[i].mins, in[i].mins);
		vec_copy(out[i].maxs, in[i].maxs);
		out[i].num_brushes = in[i].num_brushes;
		out[i].brushes = gbru.brushes + in[i].start_brush;
	}
}

static void bru_load_areas(const byte* data, const bru_lump_s* l)
{
	int i;
	area_s* out;
	bru_area_s* in;

	if (l->len < 1 || (l->len % sizeof(*in)))
	{
		con_print(COLOR_RED, "LUMP_AREAS bad size");
		longjmp(gbru_jump, TRUE);
	}

	in = (bru_area_s*)(data + l->ofs);
	gbru.num_areas = l->len / sizeof(*in);
	out = gbru.areas = util_malloc(gbru.num_areas * sizeof(*out));

	for (i = 0; i < gbru.num_areas; i++)
	{
		strcpyn(out[i].name, in[i].name);	
		out[i].num_boxes = in[i].num_boxes;
		out[i].num_brushareas = in[i].num_brushareas;
		out[i].brushareas = gbru.brushareas + in[i].start_brusharea;
		out[i].mins = gbru.box_mins + in[i].start_box;
		out[i].maxs = gbru.box_maxs + in[i].start_box;
	}
}

bool_t bru_load(const char* name)
{
	byte* data;
	string_t mapname;
	bru_header_s* header;

	sprintf(mapname, BRU_FOLDER"%s.bru", name);
	data = util_full(mapname, NULL);
	if (!data)
	{
		con_printf(COLOR_RED, "%s - not found", mapname);
		return FALSE;
	}

	header = (bru_header_s*)data;
	if (header->magic != MAP_MAGIC)
	{
		util_free(data);
		con_printf(COLOR_RED, "%s - BRU has wrong magic", mapname);
		return FALSE;
	}

	if (setjmp(gbru_jump))
	{
		util_free(data);
		util_free(gtexinfo);
		util_free(gsurfs);
		bru_free();
		return FALSE;
	}

	bru_load_textures(data, &header->lumps[BRU_LUMP_TEXTURES]);
	bru_load_texinfos(data, &header->lumps[BRU_LUMP_TEXINFOS]);
	bru_load_brushareas(data, &header->lumps[BRU_LUMP_BRUSHAREAS]);
	bru_load_areaboxes(data, &header->lumps[BRU_LUMP_AREABOXES]);
	bru_load_faces(data, &header->lumps[BRU_LUMP_SURFACES]);
	bru_load_brushes(data, &header->lumps[BRU_LUMP_BRUSHES]);
	bru_load_models(data, &header->lumps[BRU_LUMP_MODELS]);
	bru_load_areas(data, &header->lumps[BRU_LUMP_AREAS]);
	bru_load_ent(data, &header->lumps[BRU_LUMP_ENTITIES]);

	util_free(data);
	util_free(gtexinfo);
	util_free(gsurfs);
	strcpyn(gbru.name, name);
	return TRUE;
}

void bru_free()
{
	int i;

	for (i = 0; i < gnuments; i++)
		util_free(gents[i]);
	for (i = 0; i < gbru.num_textures; i++)
		util_tex_free(gbru.textures[i].t);

	util_free(gbru.brushes);
	util_free(gbru.models);
	util_free(gbru.textures);
	util_free(gbru.surfes);
	util_free(gbru.box_mins);
	util_free(gbru.box_maxs);
	util_free(gbru.brushareas);
	util_free(gbru.areas);
	util_free(gvertbuf.st);
	util_free(gvertbuf.xyz);
	util_buf_free(gvertbuf.buffer);
	memzero(&gbru, sizeof(gbru));
	memzero(&gvertbuf, sizeof(gvertbuf));
	memzero(gents, sizeof(gents));
}

void bru_unload()
{
	int i;

	for (i = 0; i < gbru.num_textures; i++)
	{
		util_tex_free(gbru.textures[i].t);
		gbru.textures[i].t = 0;
	}

	sky_free();
	util_buf_free(gvertbuf.buffer);
	gvertbuf.buffer = 0;
}

void bru_reload()
{
	int i;
	string_t tmp;

	for (i = 0; i < gbru.num_textures; i++)
	{
		sprintf(tmp, TEXTURE_FOLDER"%s", gbru.textures[i].name);
		gbru.textures[i].t = img_load(tmp);
	}

	sky_load(gsky.name);
}