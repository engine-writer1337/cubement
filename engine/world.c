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

//==========================================================================//
// SKYBOX
//==========================================================================//
static void sky_draw()
{
	vec2_t st[4];
	bool_t rotate;
	vec3_t mins, maxs, verts[4];
	static vec2_t texcoords[4] = { {0, 0}, {0, 1}, {1, 1}, {1, 0} };

	if (gsky.invisible || !gsky.pics[0])
		return;

	glDisable(GL_DEPTH_TEST);
	vid_rendermode(RENDER_NORMAL);
	glColor4ub(255, 255, 255, 255);

	rotate = !vec_is_zero(gsky.ang);
	if (rotate)
	{
		glPushMatrix();
		glTranslatef(gworld.vieworg[0], gworld.vieworg[1], gworld.vieworg[2]);
		glRotatef(gsky.ang[YAW], 0, 0, 1);
		glRotatef(gsky.ang[PITCH], 0, 1, 0);
		glRotatef(gsky.ang[ROLL], 1, 0, 0);
		
		vec_init(mins, -256);
		vec_init(maxs, 256);
	}
	else
	{
		vec_set(mins, gworld.vieworg[0] - 256, gworld.vieworg[1] - 256, gworld.vieworg[2] - 256);
		vec_set(maxs, gworld.vieworg[0] + 256, gworld.vieworg[1] + 256, gworld.vieworg[2] + 256);
	}

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
	if (rotate)
		glPopMatrix();
}

void sky_rotate(const vec3_t ang)
{
	vec_copy(gsky.ang, ang);
}

void sky_visible(bool_t is_visible)
{
	gsky.invisible = !is_visible;
}

void sky_load(const char* name)
{
	int i;
	string_t path;
	static const char* sides[6] = { "bk", "ft", "lf", "rt", "up", "dn" };

	if (!ghost.precache)
	{
		con_printf(COLOR_RED, "%s - skybox loading not allowed");
		return;
	}

	if (strnull(name) || strcmpi(gsky.name, name))
		return;

	sky_free();
	gimg.clamp = TRUE;
	gimg.mipmap = FALSE;
	gimg.nearest = gimg.nofilter->value;

	strcpyn(gsky.name, name);
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
static void world_area_visibles()
{
	area_s* a;
	surf_s* s;
	brush_s* b;
	entity_s* e;
	edict_s* ed;
	int i, j, k, m;
	bool_t anyintersect = FALSE;

	gbru.areas[0].framenum = gworld.framecount;
	for (i = 1; i < gbru.num_areas; i++)
	{
		a = gbru.areas + i;
		for (j = 0; j < a->num_boxes; j++)
		{
			if (vec2_aabb(gworld.vieworg, gworld.vieworg, a->absmin[j], a->absmax[j]))
				continue;

			a->framenum = gworld.framecount;
			anyintersect = TRUE;
			break;
		}
	}

	if (!anyintersect)
	{
		for (i = 1; i < gbru.num_areas; i++)
			gbru.areas[i].framenum = gworld.framecount;
	}

	for (i = 0; i < gbru.num_areas; i++)
	{
		a = gbru.areas + i;
		if (a->framenum != gworld.framecount && !a->activecount)
			continue;

		anyintersect = FALSE;
		for (j = 0; j < a->num_boxes; j++)
		{
			if (frustum_clip(a->absmin[j], a->absmax[j]))
				continue;

			anyintersect = TRUE;
			for (k = 0; k < a->num_brushareas; k++)
			{
				b = gbru.brushes + a->brushareas[k];
				if (b->framenum == gworld.framecount)
					continue;

				b->framenum = gworld.framecount;
				if (frustum_clip(b->absmin, b->absmax))
					continue;

				for (m = 0; m < b->num_surfes; m++)
				{
					s = b->surfes + m;
					if ((s->sign || gworld.vieworg[s->itype] < b->absmax[s->itype]) && (!s->sign || gworld.vieworg[s->itype] > b->absmin[s->itype]))
						continue;

					s->next = gbru.textures[s->texture].chain;
					gbru.textures[s->texture].chain = s;
				}
			}
			break;
		}

		if (anyintersect)
			a->framenum = gworld.framecount;
	}

	for (i = 0; i < gnuments; i++)
	{
		ed = gedicts + i;
		if (!ed->e)
			continue;

		e = ed->e;
		if (e->model == BAD_HANDLE)
			continue;

		anyintersect = FALSE;
		for (j = 0; j < ed->num_areas; j++)
		{
			if (gbru.areas[ed->areas[j]].framenum != gworld.framecount)
				continue;

			anyintersect = TRUE;
			break;
		}

		if (!anyintersect)
			continue;

		if (frustum_clip(e->absmin, e->absmax))
			continue;

		switch (e->render)
		{
		default:
			ed->next = gworld.solid_chain;
			gworld.solid_chain = ed;
			break;
		case RENDER_TRANSPARENT:
			ed->next = gworld.transparent_chain;
			gworld.transparent_chain = ed;
			break;
		case RENDER_ALPHA:
			ed->next = gworld.alpha_chain;
			gworld.alpha_chain = ed;
			break;
		case RENDER_ADDTIVE:
			ed->next = gworld.addtive_chain;
			gworld.addtive_chain = ed;
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

static void world_shade_color(surf_s* s, entity_s* ent)
{
	if (gworld.shade->value > 0)
	{
		float dim = clamp(0, gworld.shade->value, 1);
		if (s->sign)
		{
			if (gworld.v_forward[s->itype] < 0)
				dim = (1.0f - dim);
			else
				dim = gworld.v_forward[s->itype] * dim + (1.0f - dim);
		}
		else
		{
			if (gworld.v_forward[s->itype] > 0)
				dim = (1.0f - dim);
			else
				dim = -gworld.v_forward[s->itype] * dim + (1.0f - dim);
		}

		if (ent->flags & FL_MODULATE)
			glColor4ub(dim * (s->color[0] * ent->color[0]) / 255, dim * (s->color[1] * ent->color[1]) / 255, dim * (s->color[2] * ent->color[2]) / 255, ent->renderamt);
		else
			glColor4ub(dim * s->color[0], dim * s->color[1], dim * s->color[2], ent->renderamt);
	}
	else
	{
		if (ent->flags & FL_MODULATE)
			glColor4ub((s->color[0] * ent->color[0]) / 255, (s->color[1] * ent->color[1]) / 255, (s->color[2] * ent->color[2]) / 255, ent->renderamt);
		else
			glColor4ub(s->color[0], s->color[1], s->color[2], ent->renderamt);
	}
}

static void world_draw_worldspawn()
{
	int i;
	surf_s* s;
	btexture_s* t;

	world_vbo();
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

	vid_rendermode(gentworld->render);
	for (i = 0; i < gbru.num_textures; i++)
	{
		t = gbru.textures + i;
		if (!t->chain)
			continue;

		s = t->chain;
		img_bind(t->t);
		while (s)
		{
			world_shade_color(s, gentworld);
			glDrawArrays(GL_TRIANGLE_FAN, s->offset, 4);
			s = s->next;
		}
	}

	if (gworld.wireframe->value)
	{
		glDisable(GL_TEXTURE_2D);
		vid_rendermode(RENDER_NORMAL);
		glColor4ub(255, 255, 255, 255);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		if (gworld.wireframe->value < 2)
			glDisable(GL_DEPTH_TEST);

		for (i = 0; i < gbru.num_textures; i++)
		{
			s = gbru.textures[i].chain;
			while (s)
			{
				glDrawArrays(GL_QUADS, s->offset, 4);
				s = s->next;
			}
		}

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glEnable(GL_TEXTURE_2D);
		if (gworld.wireframe->value < 2)
			glEnable(GL_DEPTH_TEST);
	}

	if (gvertbuf.buffer)
		glBindBuffer(GL_ARRAY_BUFFER, 0);
}

static void world_draw_ents(edict_s* ed)
{
	int i, m;
	surf_s* s;
	brush_s* b;
	entity_s* e;
	brushmodel_s* bm;
	bool_t move, rotate;
	vec3_t movevec, absmax, absmin;

	while (ed)
	{
		e = ed->e;
		if (e->model == BAD_HANDLE)
			continue;

		switch (gres[e->model].type)
		{
		case RES_BRUSH:
			bm = gres[e->model].data.brush;
			vec_sub(movevec, e->origin, bm->origin);

			move = !vec_is_zero(movevec);
			rotate = !vec_is_zero(e->angles);
			if (move || rotate)
			{
				glPushMatrix();
				if (rotate)
				{
					glTranslatef(e->origin[0] + bm->offset[0], e->origin[1] + bm->offset[1], e->origin[2] + bm->offset[2]);
					glRotatef(-e->angles[ROLL], 1, 0, 0);
					glRotatef(-e->angles[PITCH], 0, 1, 0);
					glRotatef(-e->angles[YAW], 0, 0, 1);
					glTranslatef(-e->origin[0] - bm->offset[0], -e->origin[1] - bm->offset[1], -e->origin[2] - bm->offset[2]);
					glEnable(GL_CULL_FACE);
				}	

				glTranslatef(movevec[0], movevec[1], movevec[2]);
			}

			glTexCoordPointer(2, GL_FLOAT, 0, gvertbuf.st);
			glVertexPointer(3, GL_FLOAT, 0, gvertbuf.xyz);
			if (rotate)
			{
				for (i = 0; i < bm->num_brushes; i++)
				{
					b = bm->brushes + i;
					for (m = 0; m < b->num_surfes; m++)
					{
						s = b->surfes + m;
						world_shade_color(s, e);
						img_bind(gbru.textures[s->texture].t);
						glDrawArrays(GL_TRIANGLE_FAN, s->offset, 4);
					}
				}
			}
			else
			{
				for (i = 0; i < bm->num_brushes; i++)
				{
					b = bm->brushes + i;
					vec_add(absmax, movevec, b->absmax);
					vec_add(absmin, movevec, b->absmin);
					if (frustum_clip(absmin, absmax))
						continue;

					for (m = 0; m < b->num_surfes; m++)
					{
						s = b->surfes + m;
						if ((s->sign || gworld.vieworg[s->itype] < absmax[s->itype]) && (!s->sign || gworld.vieworg[s->itype] > absmin[s->itype]))
							continue;

						world_shade_color(s, e);
						img_bind(gbru.textures[s->texture].t);
						glDrawArrays(GL_TRIANGLE_FAN, s->offset, 4);
					}
				}
			}

			if (move || rotate)
			{
				glPopMatrix();
				if (rotate)
					glDisable(GL_CULL_FACE);
			}
			break;
		}

		ed = ed->next;
	}
}

void world_draw()
{
	int i;

	if (gworld.lock->value < 2)
	{
		gworld.framecount++;
		gworld.solid_chain = NULL;
		gworld.transparent_chain = NULL;
		gworld.alpha_chain = NULL;
		gworld.addtive_chain = NULL;

		for (i = 0; i < gbru.num_textures; i++)
			gbru.textures[i].chain = NULL;

		if (!gworld.lock->value)
			frustum_adjust();

		world_area_visibles();
	}

	sky_draw();
	world_draw_worldspawn();

	if (gworld.solid_chain)
	{
		vid_rendermode(RENDER_NORMAL);
		world_draw_ents(gworld.solid_chain);
	}

	if (gworld.transparent_chain)
	{
		vid_rendermode(RENDER_TRANSPARENT);
		world_draw_ents(gworld.transparent_chain);
	}

	if (gworld.addtive_chain)
	{
		vid_rendermode(RENDER_ADDTIVE);
		world_draw_ents(gworld.addtive_chain);
	}

	if (gworld.alpha_chain)
	{
		vid_rendermode(RENDER_ALPHA);
		world_draw_ents(gworld.alpha_chain);
	}
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
		world_end_map(NULL, NULL);

	res_flush_temp();
	ghost.gametime = 0;
	if (!ghost.precache_once)
	{
		ghost.precache = PRE_PERS;
		ghost.precache_once = TRUE;
		ggame.game_precache();
	}

	ghost.precache = PRE_TEMP;	
	if (!bru_load(ghost.newmap))//TODO: reload only ent if map same
	{
		ghost.newmap[0] = '\0';
		ghost.precache = PRE_NOT;
		return;
	}

	mat_update();
	ggame.game_start();
	gworld.is_load = TRUE;
	ghost.newmap[0] = '\0';
	ghost.precache = PRE_NOT;
	con_printf(COLOR_WHITE, "Level Load Time: %ims", (int)(1000 * (util_time() - timer)));
}

void world_end_map(const char* arg1, const char* arg2)
{
	sky_free();
	bru_free();
	ggame.game_end();
	gworld.is_load = FALSE;
}