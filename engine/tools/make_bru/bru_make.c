#include "bru_make.h"
#include <stdarg.h>
#include <conio.h>

#define MAXTOKEN	1024
#define MAXPAIRS	32768

static bool_t unget;
static int scriptline;
static char token[MAXTOKEN];
static char* script_p, * end_p;

static int gnum_pairs;
static epair_s gpairs[MAXPAIRS];

static int gnum_surfaces;
static int gnum_brushes;

static esurf_s gesurfes[MAX_MAP_SURFACES];
static ebrush_s gebrushes[MAX_MAP_BRUSHES];

int gnum_entities;
entity_s gentities[MAX_MAP_ENTITIES];

void fatal_error(const char* error, ...)
{
	va_list argptr;

	printf("************ ERROR ************\n");

	va_start(argptr, error);
	vprintf(error, argptr);
	va_end(argptr);

	printf("\n");
	_getch();
	exit(1);
}

static byte* util_full(const char* filename, int* len)
{
	FILE* fp;
	byte* data;
	int length;

	fp = fopen(filename, "rb");
	if (!fp)
		return NULL;

	fseek(fp, 0, SEEK_END);
	length = ftell(fp);
	if (!length)
	{
		fclose(fp);
		return NULL;
	}

	data = malloc(length);
	fseek(fp, 0, SEEK_SET);
	fread(data, sizeof(byte), length, fp);
	fclose(fp);
	if (len)
		*len = length;

	return data;
}

static void start_token(char* data, int size)
{
	scriptline = 1;
	script_p = data;
	unget = FALSE;
	end_p = data + size;
}

static bool_t get_token(bool_t crossline)
{
	char* token_p, * temp;
	if (unget)
		return TRUE;

	token[0] = 0;
	for (;;)
	{
		if (*script_p == 0)
			return FALSE;
		else if (*script_p == '\n')
		{
			if (!crossline)
				return FALSE;

			script_p++;
			scriptline++;
		}
		else if (*script_p <= ' ')
			script_p++;
		else if (script_p[0] == '/' && script_p[1] == '/')
		{
			while (*script_p && *script_p != '\n')
				script_p++;
		}
		else if (script_p[0] == '/' && script_p[1] == '*')
		{
			temp = script_p;
			for (;;)
			{
				if (*script_p == 0)
					break;
				else if (script_p[0] == '*' && script_p[1] == '/')
				{
					script_p += 2;
					break;
				}
				else if (*script_p == '\n')
				{
					if (!crossline)
					{
						script_p = temp;
						return FALSE;
					}

					script_p++;
					scriptline++;
				}
				else
					script_p++;
			}
		}
		else
			break;
	}

	token_p = token;
	if (*script_p == '"')
	{
		script_p++;
		while (*script_p != '"')
		{
			if (!*script_p)
				fatal_error("EOF inside quoted token");

			*token_p++ = *script_p++;
			if (token_p > &token[MAXTOKEN - 1])
				fatal_error("Token too large on line %i", scriptline);
		}

		script_p++;
	}
	else while (*script_p > 32)
	{
		*token_p++ = *script_p++;
		if (token_p > &token[MAXTOKEN - 1])
			fatal_error("Token too large on line %i", scriptline);
	}

	*token_p = 0;
	return TRUE;
}

static bool_t token_available()
{
	char* search_p;

	search_p = script_p;
	if (search_p >= end_p)
		return FALSE;

	while (*search_p <= 32)
	{
		if (*search_p == '\n')
			return FALSE;

		search_p++;
		if (search_p == end_p)
			return FALSE;
	}

	if (*search_p == ';')
		return FALSE;

	return TRUE;
}

static void strip_trailing(char* e)
{
	char* s;

	s = e + strlen(e) - 1;
	while (s >= e && *s <= 32)
	{
		*s = 0;
		s--;
	}
}

static epair_s* parse_epair()
{
	epair_s* e;

	if (gnum_pairs == MAXPAIRS)
		fatal_error("gnum_pairs == MAXPAIRS");

	e = &gpairs[gnum_pairs];
	if (strlen(token) >= MAX_MAP_KEY - 1)
	{
		printf("Ignore pair: token 'key' too long: %s\n", token);
		get_token(FALSE);
		return NULL;
	}

	strcpy(e->key, token);
	get_token(FALSE);
	if (strlen(token) >= MAX_MAP_VALUE - 1)
	{
		printf("Ignore pair: token 'value' too long: %s\n", token);
		return NULL;
	}

	strcpy(e->value, token);
	strip_trailing(e->key);
	strip_trailing(e->value);
	gnum_pairs++;
	return e;
}

static int get_texture(const char* name)
{
	int i;

	if (!_stricmp(name, "null"))
		return TEX_NULL;
	if (!_stricmp(name, "trigger"))
		return TEX_TRIGGER;
	if (!_stricmp(name, "area"))
		return TEX_AREA;
	if (!_stricmp(name, "origin"))
		return TEX_ORIGIN;

	for (i = 0; i < gnum_textures; i++)
	{
		if (!_stricmp(gtextures[i].name, name))
			return i;
	}

	if (i == MAX_MAP_TEXTURES)
		fatal_error("num_textures == MAX_MAP_TEXTURES");

	if (strlen(name) >= sizeof(gtextures[i].name) - 1)
		printf("Texture name '%s' too long\n", name);

	strncpy(gtextures[i].name, name, sizeof(gtextures[i].name) - 1);
	_strlwr(gtextures[i].name);
	gnum_textures++;
	return i;
}

static byte plane_frompoints(const vec3_t p0, const vec3_t p1, const vec3_t p2, double* dist)
{
	surftype_e t;
	vec3_t t1, t2, normal;

	vec_sub(t1, p0, p1);
	vec_sub(t2, p2, p1);
	vec_cross(normal, t1, t2);
	vec_normalize(normal);

	if (normal[0] > 1.0f - EQUAL_EPSILON)
	{
		normal[0] = 1;
		t = SURF_TYPE_X;
	}
	else if (normal[0] < -1.0f + EQUAL_EPSILON)
	{
		normal[0] = -1;
		t = SURF_TYPE_SX;
	}
	else if (normal[1] > 1.0f - EQUAL_EPSILON)
	{
		normal[1] = 1;
		t = SURF_TYPE_Y;
	}
	else if (normal[1] < -1.0f + EQUAL_EPSILON)
	{
		normal[1] = -1;
		t = SURF_TYPE_SY;
	}
	else if (normal[2] > 1.0f - EQUAL_EPSILON)
	{
		normal[2] = 1;
		t = SURF_TYPE_Z;
	}
	else if (normal[2] < -1.0f + EQUAL_EPSILON)
	{
		normal[2] = -1;
		t = SURF_TYPE_SZ;
	}
	else
		return SURF_TYPE_BAD;

	*dist = vec_dot(p0, normal);
	if (t >= SURF_TYPE_SX)
		*dist = -(*dist);
	return t;
}

static int get_texinfo(int texture, double* UVaxis, double* scale, double* shift)
{
	int i, j, k;
	double vecs[2][4];
	short ivecs[2][4];
	bru_texinfo_s* tex;

	if (!scale[0]) scale[0] = 1;
	if (!scale[1]) scale[1] = 1;

	for (i = 0; i < 2; i++)
	{
		for (j = 0; j < 3; j++)
			vecs[i][j] = UVaxis[i * 3 + j] / scale[i];
	}

	vecs[0][3] = shift[0];
	vecs[1][3] = shift[1];
	for (i = 0; i < 2; i++)
	{
		for (j = 0; j < 3; j++)
		{
			if (vecs[i][j] < -32.7f) vecs[i][j] = -32.7f;
			if (vecs[i][j] > 32.7f) vecs[i][j] = 32.7f;
			ivecs[i][j] = (short)(vecs[i][j] * TEXCOORD_SCALE);
		}

		if (vecs[i][3] > 3270) vecs[i][3] = 3270;
		if (vecs[i][3] < -3270) vecs[i][3] = -3270;
		ivecs[i][3] = (short)(vecs[i][j] * TEXCOORD_OFFSET);
	}

	for (i = 0; i < gnum_texinfos; i++)
	{
		tex = &gtexinfos[i];
		for (j = 0; j < 2; j++)
		{
			if (tex->texture != texture)
				goto skip_mv;

			for (k = 0; k < 4; k++)
			{
				if (tex->vecs[j][k] != ivecs[j][k])
					goto skip_mv;
			}
		}

		return i;
	skip_mv:;
	}

	if (i == MAX_MAP_TEXINFOS)
		fatal_error("num_texinfos == MAX_MAP_TEXINFOS");

	gtexinfos[i].texture = texture;
	memcpy(gtexinfos[i].vecs, ivecs, sizeof(ivecs));
	gnum_texinfos++;
	return i;
}

static int get_colors(int flags)
{
	int i, fl, c[3];

	for (i = 0; i < 3; i++)
	{
		fl = flags & 511;
		flags >>= 10;

		if (fl & (1 << 0)) c[i] = 0;
		if (fl & (1 << 1)) c[i] = 1;
		if (fl & (1 << 2)) c[i] = 2;
		if (fl & (1 << 3)) c[i] = 3;
		if (fl & (1 << 4)) c[i] = 4;
		if (fl & (1 << 5)) c[i] = 5;
		if (fl & (1 << 6)) c[i] = 6;
		if (fl & (1 << 7)) c[i] = 7;
		if (fl & (1 << 8)) c[i] = 8;
	}

	return (c[2] << 8) | (c[1] << 4) | c[0];
}

static void parse_brush(entity_s* mapent)
{
	esurf_s* surf;
	int i, t, flags;
	ebrush_s* brush;
	vec3_t points[3];
	double val, UVaxis[6], shift[2], scale[2];

	if (gnum_brushes == MAX_MAP_BRUSHES)
		fatal_error("num_brushes == MAX_MAP_BRUSHES");

	brush = &gebrushes[gnum_brushes];
	vec_init(brush->mins, MAX_MAP_RANGE);
	vec_init(brush->maxs, -MAX_MAP_RANGE);

	do
	{
		if (gnum_surfaces == MAX_MAP_SURFACES)
			fatal_error("num_surfaces == MAX_MAP_SURFACES");

		surf = &gesurfes[gnum_surfaces];
		if (!get_token(TRUE))
			break;

		if (!strcmp(token, "}"))
			break;

		//read vertices
		for (i = 0; i < 3; i++)
		{
			if (i != 0)
				get_token(TRUE);

			if (strcmp(token, "("))
				fatal_error("parsing brush");

			get_token(FALSE);
			val = atof(token);
			points[i][0] = clamp(-MAX_MAP_RANGE, val, MAX_MAP_RANGE);

			get_token(FALSE);
			val = atof(token);
			points[i][1] = clamp(-MAX_MAP_RANGE, val, MAX_MAP_RANGE);

			get_token(FALSE);
			val = atof(token);
			points[i][2] = clamp(-MAX_MAP_RANGE, val, MAX_MAP_RANGE);
			
			get_token(FALSE);
			if (strcmp(token, ")"))
				fatal_error("parsing brush");
		}

		//read texturename
		get_token(FALSE);
		t = get_texture(token);

		//only new map version (220)
		//texture U axis
		get_token(FALSE);
		if (strcmp(token, "["))
			fatal_error("missing '[ in texturedef");

		get_token(FALSE); UVaxis[0] = atof(token);
		get_token(FALSE); UVaxis[1] = atof(token);
		get_token(FALSE); UVaxis[2] = atof(token);
		get_token(FALSE); shift[0] = atof(token);

		get_token(FALSE);
		if (strcmp(token, "]"))
			fatal_error("missing ']' in texturedef");

		//texture V axis
		get_token(FALSE);
		if (strcmp(token, "["))
			fatal_error("missing '[ in texturedef");

		get_token(FALSE); UVaxis[3] = atof(token);
		get_token(FALSE); UVaxis[4] = atof(token);
		get_token(FALSE); UVaxis[5] = atof(token);
		get_token(FALSE); shift[1] = atof(token);

		get_token(FALSE);
		if (strcmp(token, "]"))
			fatal_error("missing ']' in texturedef");

		//texture opts
		get_token(FALSE); //rotate = atoi(token);
		get_token(FALSE); scale[0] = atof(token);
		get_token(FALSE); scale[1] = atof(token);

		flags = 0;
		if (token_available())
		{
			get_token(FALSE); //contents = atoi(token);
			get_token(FALSE); flags = atoi(token);
			get_token(FALSE); //value = atoi(token);
		}

		surf->type = plane_frompoints(points[0], points[1], points[2], &val);
		if (surf->type == SURF_TYPE_BAD)
		{
			printf("Ignore surf: surf for brush %i is bad\n", gnum_brushes);
			continue;
		}

		if (surf->type < SURF_TYPE_SX)
		{
			if (val > brush->maxs[surf->type])
				brush->maxs[surf->type] = val;
		}
		else
		{
			if (val < brush->mins[surf->type - SURF_TYPE_SX])
				brush->mins[surf->type - SURF_TYPE_SX] = val;
		}

		if (t == TEX_ORIGIN)
			brush->is_origin = TRUE;

		if (t >= 0)
		{
			surf->texinfo = get_texinfo(t, UVaxis, scale, shift);
			surf->color = get_colors(flags);
			surf->next = brush->esurfes;
			brush->esurfes = surf;
			gnum_surfaces++;
		}
	} while (1);

	brush->next = mapent->ebrushes;
	mapent->ebrushes = brush;
	gnum_brushes++;
}

static bool_t parse_ents()
{
	ebrush_s* brush;
	entity_s* mapent;

	if (!get_token(TRUE))
		return FALSE;

	if (strcmp(token, "{"))
		fatal_error("Parse Entity: { not found");

	if (gnum_entities == MAX_MAP_ENTITIES)
		fatal_error("gnum_ents == MAX_MAP_ENTITIES");

	mapent = &gentities[gnum_entities];
	gnum_entities++;

	do
	{
		if (!get_token(TRUE))
			fatal_error("Parse Entity: EOF without closing brace");

		if (!strcmp(token, "}"))
			break;

		if (!strcmp(token, "{"))
			parse_brush(mapent);
		else
		{
			epair_s* e = parse_epair();
			if (e)
			{
				if (_stricmp(e->key, "mapversion") && _stricmp(e->key, "_generator"))
				{
					e->next = mapent->epairs;
					mapent->epairs = e;
				}
			}
		}
	} while (1);

	brush = mapent->ebrushes;
	while (brush)
	{
		if (brush->is_origin)
		{
			mapent->has_offset = TRUE;
			vec_set(mapent->offset, (brush->maxs[0] + brush->mins[0]) * 0.5, (brush->maxs[1] + brush->mins[1]) * 0.5, (brush->maxs[2] + brush->mins[2]) * 0.5);
			break;
		}

		brush = brush->next;
	}

	return TRUE;
}

int main(int argc, char* argv[])
{
	int len;
	byte* data;

	printf("BRU MAKE %s %s\n", __DATE__, __TIME__);
	if (argc < 2)
		fatal_error("Drag & Drop .map file");

	len = strlen(argv[1]);
	if (_stricmp(&argv[1][len - 4], ".map"))
		fatal_error("%s - unknown file format", argv[1]);

	data = util_full(argv[1], &len);
	if (!data)
		fatal_error("%s - not found", argv[1]);

	data[len - 1] = '\0';
	start_token(data, len);
	while (parse_ents());

	bru_save(argv[1]);
	_getch();
	return 0;
}