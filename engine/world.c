#include "cubement.h"

sky_s gsky;
world_s gworld;
entity_s* gentworld;
static plane_s gfrustum[FRUSTUM_NUM];

//==========================================================================//
// FRUSTUM
//==========================================================================//
static void frustum_set_plane(int side, const vec3_t normal, float dist)
{
	gfrustum[side].signbits = vec_signbits(normal);
	vec_copy(gfrustum[side].normal, normal);
	gfrustum[side].dist = dist;
}

static void frustum_adjust()
{
	vec3_t normal;

	vec_mam(normal, gworld.sin_x, gworld.v_forward, -gworld.cos_x, gworld.v_right);
	frustum_set_plane(FRUSTUM_LEFT, normal, vec_dot(gworld.vieworg, normal));

	vec_mam(normal, gworld.sin_x, gworld.v_forward, gworld.cos_x, gworld.v_right);
	frustum_set_plane(FRUSTUM_RIGHT, normal, vec_dot(gworld.vieworg, normal));

	vec_mam(normal, gworld.sin_y, gworld.v_forward, -gworld.cos_y, gworld.v_up);
	frustum_set_plane(FRUSTUM_BOTTOM, normal, vec_dot(gworld.vieworg, normal));

	vec_mam(normal, gworld.sin_y, gworld.v_forward, gworld.cos_y, gworld.v_up);
	frustum_set_plane(FRUSTUM_TOP, normal, vec_dot(gworld.vieworg, normal));
}

static bool_t frustum_clip(const vec3_t mins, const vec3_t maxs)
{
	int i;
	plane_s* p;

	for (i = 0; i < FRUSTUM_NUM; i++)
	{
		p = gfrustum + i;
		switch (p->signbits)
		{
		case 0:
			if (p->normal[0] * maxs[0] + p->normal[1] * maxs[1] + p->normal[2] * maxs[2] < p->dist)
				return TRUE;
			break;
		case 1:
			if (p->normal[0] * mins[0] + p->normal[1] * maxs[1] + p->normal[2] * maxs[2] < p->dist)
				return TRUE;
			break;
		case 2:
			if (p->normal[0] * maxs[0] + p->normal[1] * mins[1] + p->normal[2] * maxs[2] < p->dist)
				return TRUE;
			break;
		case 3:
			if (p->normal[0] * mins[0] + p->normal[1] * mins[1] + p->normal[2] * maxs[2] < p->dist)
				return TRUE;
			break;
		case 4:
			if (p->normal[0] * maxs[0] + p->normal[1] * maxs[1] + p->normal[2] * mins[2] < p->dist)
				return TRUE;
			break;
		case 5:
			if (p->normal[0] * mins[0] + p->normal[1] * maxs[1] + p->normal[2] * mins[2] < p->dist)
				return TRUE;
			break;
		case 6:
			if (p->normal[0] * maxs[0] + p->normal[1] * mins[1] + p->normal[2] * mins[2] < p->dist)
				return TRUE;
			break;
		case 7:
			if (p->normal[0] * mins[0] + p->normal[1] * mins[1] + p->normal[2] * mins[2] < p->dist)
				return TRUE;
			break;
		default:
			return FALSE;
		}
	}
	return FALSE;
}

static bool_t frustum_clip2d(const vec2_t mins, const vec2_t maxs)
{
	int i;
	plane_s* p;

	for (i = 0; i < FRUSTUM_NUM; i++)
	{
		p = gfrustum + i;
		switch (p->signbits)
		{
		case 0:
			if (p->normal[0] * maxs[0] + p->normal[1] * maxs[1] + p->normal[2] * MAX_MAP_RANGE < p->dist)
				return TRUE;
			break;
		case 1:
			if (p->normal[0] * mins[0] + p->normal[1] * maxs[1] + p->normal[2] * MAX_MAP_RANGE < p->dist)
				return TRUE;
			break;
		case 2:
			if (p->normal[0] * maxs[0] + p->normal[1] * mins[1] + p->normal[2] * MAX_MAP_RANGE < p->dist)
				return TRUE;
			break;
		case 3:
			if (p->normal[0] * mins[0] + p->normal[1] * mins[1] + p->normal[2] * MAX_MAP_RANGE < p->dist)
				return TRUE;
			break;
		case 4:
			if (p->normal[0] * maxs[0] + p->normal[1] * maxs[1] - p->normal[2] * MAX_MAP_RANGE < p->dist)
				return TRUE;
			break;
		case 5:
			if (p->normal[0] * mins[0] + p->normal[1] * maxs[1] - p->normal[2] * MAX_MAP_RANGE < p->dist)
				return TRUE;
			break;
		case 6:
			if (p->normal[0] * maxs[0] + p->normal[1] * mins[1] - p->normal[2] * MAX_MAP_RANGE < p->dist)
				return TRUE;
			break;
		case 7:
			if (p->normal[0] * mins[0] + p->normal[1] * mins[1] - p->normal[2] * MAX_MAP_RANGE < p->dist)
				return TRUE;
			break;
		default:
			return FALSE;
		}
	}
	return FALSE;
}

//==========================================================================//
// SKYBOX
//==========================================================================//
static void sky_draw()
{
	vec2_t st[4];
	vec3_t mins, maxs, verts[4];
	static vec2_t texcoords[4] = { {0, 0}, {0, 1}, {1, 1}, {1, 0} };

	if (!gsky.visible || !gsky.pics[0])
		return;

	glDisable(GL_DEPTH_TEST);
	vec_set(mins, gworld.vieworg[0] - 256, gworld.vieworg[1] - 256, gworld.vieworg[2] - 256);
	vec_set(maxs, gworld.vieworg[0] + 256, gworld.vieworg[1] + 256, gworld.vieworg[2] + 256);

	//==========================================================================//
	vec2_copy(st[0], texcoords[2]);
	vec2_copy(st[1], texcoords[1]);
	vec2_copy(st[2], texcoords[0]);
	vec2_copy(st[3], texcoords[3]);

	vec_set(verts[0], maxs[0], maxs[1], mins[2]);
	vec_set(verts[1], mins[0], maxs[1], mins[2]);
	vec_set(verts[2], mins[0], maxs[1], maxs[2]);
	vec_set(verts[3], maxs[0], maxs[1], maxs[2]);

	img_bind(gsky.pics[0]);
	glTexCoordPointer(2, GL_FLOAT, 0, st);
	glVertexPointer(3, GL_FLOAT, 0, verts);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	//==========================================================================//
	vec_set(verts[0], maxs[0], mins[1], mins[2]);
	vec_set(verts[1], maxs[0], maxs[1], mins[2]);
	vec_set(verts[2], maxs[0], maxs[1], maxs[2]);
	vec_set(verts[3], maxs[0], mins[1], maxs[2]);

	img_bind(gsky.pics[3]);
	glTexCoordPointer(2, GL_FLOAT, 0, st);
	glVertexPointer(3, GL_FLOAT, 0, verts);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	//==========================================================================//
	vec2_copy(st[0], texcoords[0]);
	vec2_copy(st[1], texcoords[3]);
	vec2_copy(st[2], texcoords[2]);
	vec2_copy(st[3], texcoords[1]);

	vec_set(verts[0], maxs[0], mins[1], maxs[2]);
	vec_set(verts[1], mins[0], mins[1], maxs[2]);
	vec_set(verts[2], mins[0], mins[1], mins[2]);
	vec_set(verts[3], maxs[0], mins[1], mins[2]);

	img_bind(gsky.pics[1]);
	glTexCoordPointer(2, GL_FLOAT, 0, st);
	glVertexPointer(3, GL_FLOAT, 0, verts);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	//==========================================================================//
	vec_set(verts[0], mins[0], mins[1], maxs[2]);
	vec_set(verts[1], mins[0], maxs[1], maxs[2]);
	vec_set(verts[2], mins[0], maxs[1], mins[2]);
	vec_set(verts[3], mins[0], mins[1], mins[2]);

	img_bind(gsky.pics[2]);
	glTexCoordPointer(2, GL_FLOAT, 0, st);
	glVertexPointer(3, GL_FLOAT, 0, verts);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	//==========================================================================//
	vec2_copy(st[0], texcoords[1]);
	vec2_copy(st[1], texcoords[0]);
	vec2_copy(st[2], texcoords[3]);
	vec2_copy(st[3], texcoords[2]);

	vec_set(verts[0], maxs[0], maxs[1], maxs[2]);
	vec_set(verts[1], mins[0], maxs[1], maxs[2]);
	vec_set(verts[2], mins[0], mins[1], maxs[2]);
	vec_set(verts[3], maxs[0], mins[1], maxs[2]);

	img_bind(gsky.pics[4]);
	glTexCoordPointer(2, GL_FLOAT, 0, st);
	glVertexPointer(3, GL_FLOAT, 0, verts);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	//==========================================================================//
	vec_set(verts[0], mins[0], maxs[1], mins[2]);
	vec_set(verts[1], maxs[0], maxs[1], mins[2]);
	vec_set(verts[2], maxs[0], mins[1], mins[2]);
	vec_set(verts[3], mins[0], mins[1], mins[2]);

	img_bind(gsky.pics[5]);
	glTexCoordPointer(2, GL_FLOAT, 0, st);
	glVertexPointer(3, GL_FLOAT, 0, verts);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glEnable(GL_DEPTH_TEST);
}

void sky_load(const char* name)
{
	int i;
	string_t path;
	static const char* sides[6] = { "bk", "ft", "lf", "rt", "up", "dn" };

	sky_free();
	gimg.clamp = TRUE;
	gimg.mipmap = FALSE;
	gimg.nearest = gimg.nofilter->value;

	for (i = 0; i < 6; i++)
	{
		sprintf(path, SKY_FOLDER"%s%s", name, sides[i]);
		gsky.pics[i] = img_load(path);

		if (!gsky.pics[i])
		{
			sky_free();
			return;
		}
	}
}

void sky_free()
{
	if (!gsky.pics[0])
		return;

	util_tex_free(gsky.pics[0]);
	util_tex_free(gsky.pics[1]);
	util_tex_free(gsky.pics[2]);
	util_tex_free(gsky.pics[3]);
	util_tex_free(gsky.pics[4]);
	util_tex_free(gsky.pics[5]);
	gsky.pics[0] = 0;
}

//==========================================================================//
// WORLD DRAW
//==========================================================================//
#define APPEND_SURF(s)	s->next = gbru.textures[s->texture].chain, gbru.textures[s->texture].chain = s

static void world_area_visibles()
{
	area_s* a;
	surf_s* s;
	brush_s* b;
	int i, j, k, m;
	bool_t anyintersect = FALSE;

	gbru.areas[0].visframe = gworld.framecount;
	for (i = 1; i < gbru.num_areas; i++)
	{
		a = gbru.areas + i;
		for (j = 0; j < a->num_boxes; j++)
		{
			if (gworld.vieworg[0] > a->maxs[j][0] || gworld.vieworg[1] > a->maxs[j][1] ||
				gworld.vieworg[0] < a->mins[j][0] || gworld.vieworg[1] < a->mins[j][1])
				continue;

			a->visframe = gworld.framecount;
			anyintersect = TRUE;
			break;
		}
	}

	if (!anyintersect)
	{
		for (i = 1; i < gbru.num_areas; i++)
			gbru.areas[i].visframe = gworld.framecount;
	}

	for (i = 0; i < gbru.num_areas; i++)
	{
		a = gbru.areas + i;
		if (a->visframe != gworld.framecount && !a->active)
			continue;

		a->visframe = gworld.framecount;
		for (j = 0; j < a->num_boxes; j++)
		{
			if (frustum_clip2d(a->mins[j], a->maxs[j]))
				continue;

			for (k = 0; k < a->num_brushareas; k++)
			{
				b = gbru.brushes + a->brushareas[k];
				if (b->visframe == gworld.framecount)
					continue;

				b->visframe = gworld.framecount;
				if (frustum_clip(b->mins, b->maxs))
					continue;

				for (m = 0; m < b->num_surfes; m++)
				{
					s = b->surfes + m;
					if (s->texture == BAD_HANDLE)
						continue;//CLIP texture

					switch (s->type)
					{
					case SURF_TYPE_X:
						if (gworld.vieworg[0] < b->mins[0])
							APPEND_SURF(s);
						break;
					case SURF_TYPE_Y:
						if (gworld.vieworg[1] < b->mins[1])
							APPEND_SURF(s);
						break;
					case SURF_TYPE_Z:
						if (gworld.vieworg[2] < b->mins[2])
							APPEND_SURF(s);
						break;
					case SURF_TYPE_SX:
						if (gworld.vieworg[0] > b->maxs[0])
							APPEND_SURF(s);
						break;
					case SURF_TYPE_SY:
						if (gworld.vieworg[1] > b->maxs[1])
							APPEND_SURF(s);
						break;
					case SURF_TYPE_SZ:
						if (gworld.vieworg[2] > b->maxs[2])
							APPEND_SURF(s);
						break;
					}
				}
			}
			break;
		}
	}
}

static void world_vbo()
{
	if (gworld.vbo->value)
	{
		if (gvertbuf.buffer)
			return;

		gvertbuf.buffer = util_buf_gen();
		glBindBuffer(GL_ARRAY_BUFFER, gvertbuf.buffer);
		glBufferData(GL_ARRAY_BUFFER, gvertbuf.count * (sizeof(vec3_t) + sizeof(vec2_t)), 0, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, gvertbuf.count * sizeof(vec3_t), gvertbuf.xyz);
		glBufferSubData(GL_ARRAY_BUFFER, gvertbuf.count * sizeof(vec3_t), gvertbuf.count * sizeof(vec2_t), gvertbuf.st);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	else
	{
		if (!gvertbuf.buffer)
			return;

		util_buf_free(gvertbuf.buffer);
		gvertbuf.buffer = 0;
	}
}

static void world_draw_worldspawn()
{
	int i;
	surf_s* s;
	btexture_s* t;

	vid_rendermode(RENDER_NORMAL);//TODO: grab rendermode from game.exe
	for (i = 0; i < gbru.num_textures; i++)
	{
		t = gbru.textures + i;
		if (!t->chain)
			continue;

		s = t->chain;
		img_bind(t->t);
		while (s)
		{
			glColor4ubv(s->color);
			glDrawArrays(GL_TRIANGLE_FAN, s->offset, 4);
			s = s->next;
		}
	}
}

void world_draw()
{
	int i;

	gworld.framecount++;
	for (i = 0; i < gbru.num_textures; i++)
		gbru.textures[i].chain = NULL;

	world_vbo();
	frustum_adjust();
	sky_draw();

	world_area_visibles();
	//TODO: entity visible here

	//========== DRAW BRUSHES ==========//
	if (gvertbuf.buffer)
	{
		glBindBuffer(GL_ARRAY_BUFFER, gvertbuf.buffer);
		glVertexPointer(3, GL_FLOAT, 0, NULL);
		glTexCoordPointer(2, GL_FLOAT, 0, (void*)(gvertbuf.count * sizeof(vec3_t)));
	}
	else
	{
		glTexCoordPointer(2, GL_FLOAT, 0, gvertbuf.st);
		glVertexPointer(3, GL_FLOAT, 0, gvertbuf.xyz);
	}

	world_draw_worldspawn();
	if (gvertbuf.buffer)
		glBindBuffer(GL_ARRAY_BUFFER, 0);

	//========== DRAW MODELS ==========//

	//========== DRAW BEAMS ==========//

	//========== DRAW SPRITES ==========//

}

//==========================================================================//
// WORLD ADJUST AND LOAD
//==========================================================================//
void world_set_fov(float fov)
{
	float angle;

	gworld.fov_x = fov;
	gworld.fov_y = util_calc_fov(&gworld.fov_x, gvid.width, gvid.height);
	util_adjust_fov(&gworld.fov_x, &gworld.fov_y, gvid.width, gvid.height);

	gworld.max_x = tanf(gworld.fov_x * MATH_PI / 360);
	gworld.max_y = tanf(gworld.fov_y * MATH_PI / 360);

	angle = deg2rad(gworld.fov_x) * 0.5f;
	gworld.sin_x = sinf(angle);
	gworld.cos_x = cosf(angle);

	angle = deg2rad(gworld.fov_y) * 0.5f;
	gworld.sin_y = sinf(angle);
	gworld.cos_y = cosf(angle);
}

void world_view_ang(const vec3_t ang)
{
	vec_copy(gworld.viewang, ang);
	vec_from_angles(gworld.viewang, gworld.v_forward, gworld.v_right, gworld.v_up);
}

void world_view_org(const vec3_t org)
{
	vec_copy(gworld.vieworg, org);
}

void world_setup3d()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-gworld.max_x, gworld.max_x, -gworld.max_y, gworld.max_y, 1, 64000);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glRotatef(-90, 1, 0, 0);
	glRotatef(90, 0, 0, 1);
	glRotatef(-gworld.viewang[ROLL], 1, 0, 0);
	glRotatef(-gworld.viewang[PITCH], 0, 1, 0);
	glRotatef(-gworld.viewang[YAW], 0, 0, 1);
	glTranslatef(-gworld.vieworg[0], -gworld.vieworg[1], -gworld.vieworg[2]);

	glEnable(GL_DEPTH_TEST);
	vid_rendermode(RENDER_NORMAL);
	glColor4ub(255, 255, 255, 255);
}

void world_map_cmd(const char* arg1, const char* arg2)
{
	if (!arg1[0])
	{
		HANDLE find;
		WIN32_FIND_DATA wfd;

		find = FindFirstFile(BRU_FOLDER"*.bru", &wfd);
		if (find == INVALID_HANDLE_VALUE)
			return;

		con_printf(COLOR_WHITE, "%s", wfd.cFileName);
		while (FindNextFile(find, &wfd))
			con_printf(COLOR_WHITE, "%s", wfd.cFileName);
		FindClose(find);
		return;
	}

	strcpyn(ghost.newmap, arg1);
}

void world_load_map()
{
	ftime_t timer;

	timer = util_time();
	if (gworld.is_load)
	{
		bru_free();
		gworld.is_load = FALSE;
	}

	ent_string_flush();
	ghost.precache = PRE_TEMP;
	
	if (!bru_load(ghost.newmap))
	{
		ghost.newmap[0] = '\0';
		ghost.precache = PRE_NOT;
		return;
	}

	ghost.gametime = 0;
	gworld.is_load = TRUE;
	ghost.newmap[0] = '\0';
	ghost.precache = PRE_NOT;
	con_printf(COLOR_WHITE, "Level Load Time: %ims", (int)(1000 * (util_time() - timer)));
}