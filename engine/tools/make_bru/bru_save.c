#include "bru_make.h"

int gnum_texinfos;
bru_texinfo_s gtexinfos[MAX_MAP_TEXINFOS];

int gnum_textures;
bru_texture_s gtextures[MAX_MAP_TEXTURES];

static int gnum_surfaces;
static bru_surf_s gsurfes[MAX_MAP_SURFACES];

static int gnum_brushes;
static bru_brush_s gbrushes[MAX_MAP_BRUSHES];

static int gnum_models;
static bru_model_s gmodels[MAX_MAP_ENTITIES];

static bru_header_s gmap;
static int gofs = sizeof(bru_header_s);
static byte gdata[16 * 1024 * 1024];

static int glen_entstring;
static char gent_data[MAX_MAP_ENTSTRING];

static int gnum_areas;
static bru_area_s gareas[MAX_MAP_AREAS];

static int gnum_areaboxes;
static bru_areabox_s gareaboxes[MAX_MAP_AREABOXES];

static int gnum_brushareas;
static word gbrushareas[MAX_MAP_BRUSHAREAS];

static char* value_for_key(entity_s* ent, char* key)
{
	epair_s* ep;
	for (ep = ent->epairs; ep; ep = ep->next)
	{
		if (!_stricmp(ep->key, key))
			return ep->value;
	}

	return "";
}

static void emit_entities()
{
	int i, j, k;
	entity_s* e;
	bool_t istrig;
	esurf_s* surf;
	epair_s* pair;
	string_t line;
	ebrush_s* brush;
	bru_model_s* mod;
	bru_area_s* area;
	char* buf, * end, * name;

	buf = gent_data;
	end = buf;
	*end = 0;

	gnum_areas = 1;
	gnum_areaboxes = 1;
	gareas[0].num_boxes = 1;
	strcpy(gareas[0].name, "*main*");

	for (i = 0; i < gnum_entities; i++)
	{
		e = &gentities[i];
		if (!_stricmp("area", value_for_key(e, "classname")))
		{
			if (!e->ebrushes)
				continue;

			if (gnum_areas == MAX_MAP_AREAS)
				fatal_error("gnum_areas == MAX_MAP_AREAS");

			area = &gareas[gnum_areas];
			name = value_for_key(e, "name");
			if (name)
				strncpy(area->name, name, sizeof(area->name) - 1);
			else
				sprintf(area->name, "*%i", gnum_areas);

			area->start_box = gnum_areaboxes;
			brush = e->ebrushes;
			while (brush)
			{
				if (gnum_areaboxes == MAX_MAP_AREABOXES)
					fatal_error("gnum_areaboxes == MAX_MAP_AREABOXES");

				vec2_copy(gareaboxes[gnum_areaboxes].mins, brush->mins);
				vec2_copy(gareaboxes[gnum_areaboxes].maxs, brush->maxs);
				gnum_areaboxes++;
			}

			area->num_boxes = gnum_areaboxes - area->start_box;
			gnum_areas++;
			continue;
		}

		if (!_strnicmp("trigger_", value_for_key(e, "classname"), 10))
			istrig = TRUE;
		else
			istrig = FALSE;

		strcat(end, "{\n"); end += 2;
		if (e->ebrushes)
		{
			if (gnum_models == MAX_MAP_ENTITIES)
				fatal_error("num_models == MAX_MAP_ENTITIES");

			mod = &gmodels[gnum_models];
			vec_init(mod->mins, MAX_MAP_RANGE);
			vec_init(mod->maxs, -MAX_MAP_RANGE);

			j = 0;
			brush = e->ebrushes;
			while (brush)
			{
				if (!istrig)
				{
					k = 0;
					surf = brush->esurfes;
					while (surf)
					{
						gsurfes[gnum_surfaces].encode = (((int)surf->type) << 12) | surf->color;
						gsurfes[gnum_surfaces].texinfo = surf->texinfo;

						surf = surf->next;
						gnum_surfaces++;
						k++;
					}

					gbrushes[gnum_brushes].num_surfaces = k;
					gbrushes[gnum_brushes].start_surface = gnum_surfaces - k;
				}

				vec_copy(gbrushes[gnum_brushes].mins, brush->mins);
				vec_copy(gbrushes[gnum_brushes].maxs, brush->maxs);

				if (mod->mins[0] > brush->mins[0]) mod->mins[0] = brush->mins[0];
				if (mod->maxs[0] < brush->maxs[0]) mod->maxs[0] = brush->maxs[0];

				if (mod->mins[1] > brush->mins[1]) mod->mins[1] = brush->mins[1];
				if (mod->maxs[1] < brush->maxs[1]) mod->maxs[1] = brush->maxs[1];

				if (mod->mins[2] > brush->mins[2]) mod->mins[2] = brush->mins[2];
				if (mod->maxs[2] < brush->maxs[2]) mod->maxs[2] = brush->maxs[2];

				brush = brush->next;
				gnum_brushes++;
				j++;
			}

			mod->num_brushes = j;
			mod->start_brush = gnum_brushes - j;

			sprintf(line, "\"model\" \"*%i\"\n", gnum_models);
			strcat(end, line); end += strlen(line);
			gnum_models++;
		}

		if (e->epairs)
		{
			pair = e->epairs;
			while (pair)
			{
				sprintf(line, "\"%s\" \"%s\"\n", pair->key, pair->value);
				strcat(end, line); end += strlen(line);
				pair = pair->next;
			}
		}

		strcat(end, "}\n"); end += 2;
		if (end > buf + MAX_MAP_ENTSTRING)
			fatal_error("Entity text too long");
	}

	glen_entstring = end - buf + 1;
}

static void emit_areas()
{
	int i, j, k;
	bru_brush_s* b;
	bru_area_s* area;
	bru_areabox_s* abox;
	bool_t brushcheck[MAX_MAP_BRUSHES];
	bool_t alreadyadded[MAX_MAP_BRUSHES];

	vec2_copy(gareaboxes[0].mins, gmodels[0].mins);
	vec2_copy(gareaboxes[0].maxs, gmodels[0].maxs);
	memset(brushcheck, 0, sizeof(brushcheck));

	for (i = 1; i < gnum_areas; i++)
	{
		area = gareas + 1;
		area->start_brusharea = gnum_brushareas;
		memset(alreadyadded, 0, sizeof(alreadyadded));
		for (j = area->start_box; j < area->num_boxes; j++)
		{
			abox = gareaboxes + j;
			for (k = 0; k < gmodels[0].num_brushes; k++)
			{
				if (alreadyadded[k])
					continue;

				b = &gbrushes[gmodels[0].start_brush + k];
				if (abox->mins[0] >= b->maxs[0] || abox->mins[1] >= b->maxs[1] ||
					abox->maxs[0] <= b->mins[0] || abox->maxs[1] <= b->mins[1])
					continue;

				if (gnum_brushareas == MAX_MAP_BRUSHAREAS)
					fatal_error("gnum_brushareas == MAX_MAP_BRUSHAREAS");

				brushcheck[k] = TRUE;
				alreadyadded[k] = TRUE;
				gbrushareas[gnum_brushareas] = gmodels[0].start_brush + k;
				gnum_brushareas++;
			}
		}

		area->num_brushareas = gnum_brushareas - area->start_brusharea;
	}

	area = gareas;
	area->start_brusharea = gnum_brushareas;
	for (k = 0; k < gmodels[0].num_brushes; k++)
	{
		if (brushcheck[k])
			continue;

		gbrushareas[gnum_brushareas] = gmodels[0].start_brush + k;
		gnum_brushareas++;
	}

	area->num_brushareas = gnum_brushareas - area->start_brusharea;
	if (area->num_brushareas > 0)
		printf("Common brushes: %i\n", area->num_brushareas);
}

static void lvl_lump_add(int lump, void* data, int size)
{
	memcpy(gdata + gofs, data, size);
	gmap.lumps[lump].len = size;
	gmap.lumps[lump].ofs = gofs;
	gofs += size;
}

void bru_save(const char* filename)
{
	int len;
	FILE* fp;
	string_t outfile = { 0 };

	emit_entities();
	emit_areas();

	lvl_lump_add(BRU_LUMP_SURFACES, gsurfes, gnum_surfaces * sizeof(bru_surf_s));
	lvl_lump_add(BRU_LUMP_BRUSHES, gbrushes, gnum_brushes * sizeof(bru_brush_s));
	lvl_lump_add(BRU_LUMP_TEXINFOS, gtexinfos, gnum_texinfos * sizeof(bru_texinfo_s));
	lvl_lump_add(BRU_LUMP_MODELS, gmodels, gnum_models * sizeof(bru_model_s));
	lvl_lump_add(BRU_LUMP_TEXTURES, gtextures, gnum_textures * sizeof(bru_texture_s));
	lvl_lump_add(BRU_LUMP_AREABOXES, gareaboxes, gnum_areaboxes * sizeof(bru_areabox_s));
	lvl_lump_add(BRU_LUMP_AREAS, gareas, gnum_areas * sizeof(bru_area_s));
	lvl_lump_add(BRU_LUMP_BRUSHAREAS, gbrushareas, gnum_brushareas * sizeof(word));
	lvl_lump_add(BRU_LUMP_ENTITIES, gent_data, glen_entstring);

	gmap.magic = MAP_MAGIC;
	memcpy(gdata, &gmap, sizeof(bru_header_s));

	//save file
	len = (int)strlen(filename);
	strncpy(outfile, filename, len - 4);
	strcat(outfile, ".bru");

	fp = fopen(outfile, "wb");
	if (!fp)
		fatal_error("Can't save %s", outfile);

	fwrite(gdata, 1, gofs, fp);
	fclose(fp);
	printf("Saved: %s - %i\n\n", outfile, gofs);

	//print results
	printf("%6i/%6i surfaces\t\t%7i\n", gnum_surfaces, MAX_MAP_SURFACES, (int)(gnum_surfaces * sizeof(bru_surf_s)));
	printf("%6i/%6i brushes\t\t%7i\n", gnum_brushes, MAX_MAP_BRUSHES, (int)(gnum_brushes * sizeof(bru_brush_s)));
	printf("%6i/%6i texinfos\t\t%7i\n", gnum_texinfos, MAX_MAP_TEXINFOS, (int)(gnum_texinfos * sizeof(bru_texinfo_s)));
	printf("%6i/%6i entities\t\t%7i\n", gnum_entities, MAX_MAP_ENTITIES, glen_entstring);
	printf("%6i/%6i models\t\t%7i\n", gnum_models, MAX_MAP_ENTITIES, (int)(gnum_models * sizeof(bru_model_s)));
	printf("%6i/%6i textures\t\t%7i\n", gnum_textures, MAX_MAP_TEXTURES, (int)(gnum_textures * sizeof(bru_texture_s)));
	printf("%6i/%6i areas\t\t%7i\n", gnum_areas, MAX_MAP_AREAS, (int)(gnum_areas * sizeof(bru_area_s)));
	printf("%6i/%6i areaboxes\t\t%7i\n", gnum_areaboxes, MAX_MAP_AREABOXES, (int)(gnum_areaboxes * sizeof(bru_areabox_s)));
	printf("%6i/%6i brushareas\t%7i\n", gnum_brushareas, MAX_MAP_BRUSHAREAS, (int)(gnum_brushareas * sizeof(word)));
}