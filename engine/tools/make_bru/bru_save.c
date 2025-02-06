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

static int gnum_sectors;
static bru_sector_s gsectors[MAX_MAP_SECTORS];

static int gnum_brushsectors;
static word gbrushsectors[MAX_MAP_BRUSHSECTOR];

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
	char* buf, * end;

	buf = gent_data;
	end = buf;
	*end = 0;

	for (i = 0; i < gnum_entities; i++)
	{
		e = &gentities[i];
		if (e->is_area)
			continue;

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

static void emit_sector(void)
{
	bru_brush_s* b;
	word* data, * pnum;
	short mins[2], maxs[2];
	int i, x, y, w, h, num, count;

	w = ceilf(fabsf(gmodels[0].maxs[0] - gmodels[0].mins[0]) / SECTOR_SIZE);
	h = ceilf(fabsf(gmodels[0].maxs[1] - gmodels[0].mins[1]) / SECTOR_SIZE);

	if (w < 1) w = 1;
	if (h < 1) h = 1;

	count = 0;
	data = (word*)gsec_data;
	*data++ = (word)w;
	*data++ = (word)h;
	*data++ = (word)SECTOR_SIZE;
	data += 2;

	for (y = 0; y < h; y++)
	{
		mins[1] = gmodels[0].mins[1] + y * SECTOR_SIZE;
		maxs[1] = mins[1] + SECTOR_SIZE;

		for (x = 0; x < w; x++)
		{
			mins[0] = gmodels[0].mins[0] + x * SECTOR_SIZE;
			maxs[0] = mins[0] + SECTOR_SIZE;

			num = 0;
			pnum = data;
			data++;

			for (i = 0; i < gmodels[0].num_brushes; i++)
			{
				b = &gbrushes[gmodels[0].start_brush + i];
				if (mins[0] >= b->maxs[0] || mins[1] >= b->maxs[1] ||
					maxs[0] <= b->mins[0] || maxs[1] <= b->mins[1])
					continue;

				num++;
				count++;
				*data++ = (word)(gmodels[0].start_brush + i);
			}

			*pnum = num;
		}
	}

	*(int*)(gsec_data + 6) = count;
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
	FILE* fp;
	string_t outfile = { 0 };

	emit_entities();
	emit_sector();

	lvl_lump_add(BRU_LUMP_SURFACES, gsurfes, gnum_surfaces * sizeof(bru_surf_s));
	lvl_lump_add(BRU_LUMP_BRUSHES, gbrushes, gnum_brushes * sizeof(bru_brush_s));
	lvl_lump_add(BRU_LUMP_TEXINFOS, gtexinfos, gnum_texinfos * sizeof(bru_texinfo_s));
	lvl_lump_add(BRU_LUMP_MODELS, gmodels, gnum_models * sizeof(bru_model_s));
	lvl_lump_add(BRU_LUMP_TEXTURES, gtextures, gnum_textures * sizeof(bru_texture_s));
	lvl_lump_add(BRU_LUMP_SECTORS, gsectors, gnum_sectors * sizeof(bru_sector_s));
	lvl_lump_add(BRU_LUMP_AREAS, gareas, gnum_areas * sizeof(bru_area_s));
	lvl_lump_add(BRU_LUMP_BRUSHSECTORS, gbrushsectors, gnum_brushsectors * sizeof(word));
	lvl_lump_add(BRU_LUMP_ENTITIES, gent_data, glen_entstring);

	gmap.magic = MAP_MAGIC;
	memcpy(gdata, &gmap, sizeof(bru_header_s));

	//save file
	sprintf(outfile, "%s.bru", filename);

	fp = fopen(outfile, "wb");
	if (!fp)
		fatal_error("Can't save %s", outfile);

	fwrite(gdata, 1, gofs, fp);
	fclose(fp);
	printf("Saved: %s - %i\n\n", outfile, gofs);

	//print results
	printf("%7i           entities data\n", glen_entstring);
	printf("%7i/%7i surfaces\t\t%7i\n", gnum_surfaces, MAX_MAP_SURFACES, (int)(gnum_surfaces * sizeof(bru_surf_s)));
	printf("%7i/%7i brushes\t\t%7i\n", gnum_brushes, MAX_MAP_BRUSHES, (int)(gnum_brushes * sizeof(bru_brush_s)));
	printf("%7i/%7i texinfos\t\t%7i\n", gnum_texinfos, MAX_MAP_TEXINFOS, (int)(gnum_texinfos * sizeof(bru_texinfo_s)));
	printf("%7i/%7i entities\n", gnum_entities, MAX_MAP_ENTITIES);
	printf("%7i/%7i models\t\t%7i\n", gnum_models, MAX_MAP_ENTITIES, (int)(gnum_models * sizeof(bru_model_s)));
	printf("%7i/%7i textures\t\t%7i\n", gnum_textures, MAX_MAP_TEXTURES, (int)(gnum_textures * sizeof(bru_texture_s)));
	printf("%7i/%7i areas\t\t%7i\n", gnum_areas, MAX_MAP_AREAS, (int)(gnum_areas * sizeof(bru_area_s)));
	printf("%7i/%7i sectors\t\t%7i\n", gnum_sectors, MAX_MAP_SECTORS, (int)(gnum_sectors * sizeof(bru_sector_s)));
	printf("%7i/%7i brushsectors\t\t%7i\n", gnum_brushsectors, MAX_MAP_BRUSHSECTOR, (int)(gnum_brushsectors * sizeof(word)));
}