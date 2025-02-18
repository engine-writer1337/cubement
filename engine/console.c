#include "cubement.h"

console_s gcon;
const byte gcolors[COLOR_GREEN + 1][4] =
{
	{255, 255, 255, 255},	//COLOR_WHITE
	{255, 170, 30, 255},	//COLOR_ORANGE
	{64, 64, 255, 255},		//COLOR_CYAN
	{255, 64, 64, 255},		//COLOR_RED
	{64, 255, 64, 255},		//COLOR_GREEN
};

static convar_s* con_find_cvar(const char* name)
{
	int i;
	convar_s* c;
	hash_t hash;

	hash = util_hash_str(name);
	for (i = 0; i < CON_MAX_CVARS; i++)
	{
		c = gcon.cvars + i;
		if (!c->name || c->hash != hash)
			continue;

		if (strcmpi(c->name, name))
			return c;
	}

	return NULL;
}

static convar_s* con_create_cvar(const char* name, float initial, bool_t save)
{
	convar_s* cvar;

	if (!gcon.can_register)
	{
		con_printf(COLOR_RED, "%s - cvar registration is not allowed", name);
		return NULL;
	}

	if (strnull(name))
		return NULL;

	if (con_find_cvar(name))
	{
		con_printf(COLOR_RED, "%s - already define", name);
		return NULL;
	}

	cvar = &gcon.cvars[gcon.num_cvars++];
	cvar->name = name;
	cvar->value = initial;
	cvar->initial = initial;
	cvar->should_save = save;
	cvar->hash = util_hash_str(cvar->name);
	return cvar;
}

cvar_s* con_create_cvar2(const char* name, float initial, bool_t save)
{
	return (cvar_s*)con_create_cvar(name, initial, save);
}

void con_create_cmd(const char* name, conact_t action)
{
	convar_s* cvar;

	if (!gcon.can_register)
	{
		con_printf(COLOR_RED, "%s - cvar registration is not allowed", name);
		return;
	}

	if (strnull(name))
		return;

	if (con_find_cvar(name))
	{
		con_printf(COLOR_RED, "%s - already define", name);
		return;
	}

	cvar = &gcon.cvars[gcon.num_cvars++];
	cvar->name = name;
	cvar->action = action;
	cvar->hash = util_hash_str(cvar->name);
}

void con_print(color_e color, const char* text)
{
	if (gcon.num_lines >= CON_MAX_STRINGS)
	{
		memcpy(gcon.lines, gcon.lines[CON_MAX_STRS / 2], sizeof(constr_t) * CON_MAX_STRS);
		memcpy(gcon.colors, &gcon.colors[CON_MAX_STRS / 2], sizeof(enum_t) * CON_MAX_STRS);
		gcon.num_lines = CON_MAX_STRS;
	}

	strcpyn(gcon.lines[gcon.num_lines], text);
	gcon.colors[gcon.num_lines] = color;

	gcon.num_lines++;
	gcon.top_line = gcon.num_lines;
}

void con_printf(color_e color, const char* text, ...)
{
	string_t str;
	va_list argptr;

	if (strnull(text))
		return;

	va_start(argptr, text);
	vsnprintf(str, sizeof(str), text, argptr);
	va_end(argptr);

	con_print(color, str);
}

void con_line_execute(const char* line)
{
	int k, state;
	convar_s* cvar;
	constr_t args[3];
	bool_t space, quote;

	if (strnull(line))
		return;

	k = 0;
	state = 0;
	space = FALSE;
	quote = FALSE;
	args[0][0] = args[1][0] = args[2][0] = '\0';

	for (; *line; line++)
	{
		if (*line == '\n' || *line == '\r')
			continue;

		if (*line == '\"')
		{
			quote = !quote;
			continue;
		}

		if (!quote && (*line == ' ' || *line == '\t'))
		{
			if (k)
				space = TRUE;
			continue;
		}

		if (space)
		{
			args[state][k] = '\0';

			k = 0;
			state++;
			space = FALSE;
			if (state == 3)
				break;
		}

		if (*line == '\t')
			args[state][k++] = ' ';
		else
			args[state][k++] = *line;
	}

	if (state != 3)
		args[state][k] = '\0';

	cvar = con_find_cvar(args[0]);
	if (!cvar)
	{
		con_printf(COLOR_RED, "%s - unknown command", args[0]);
		return;
	}

	if (cvar->action)
		cvar->action(args[1], args[2]);
	else if (!args[1][0])
		con_printf(COLOR_WHITE, "%s = %g", cvar->name, cvar->value);
	else
	{
		float value = atof(args[1]);
		if (value != cvar->value)
		{
			con_printf(COLOR_GREEN, "%s set %g to %g", cvar->name, cvar->value, value);
			cvar->is_change = !gcon.can_register;
			cvar->value = value;
		}
	}
}

void con_keys(int key, bool_t down)
{
	if (!down)
		return;

	switch (key)
	{
	case 'c':
		if (gkeys[K_CTRL].down && gcon.len > 0)
			util_set_clipboard(gcon.buffer);
		break;

	case 'x':
		if (gkeys[K_CTRL].down && gcon.len > 0)
		{
			util_set_clipboard(gcon.buffer);
			memzero(gcon.buffer, sizeof(gcon.buffer));
			gcon.len = gcon.cursor = 0;
		}
		break;

	case 'v':
		if (gkeys[K_CTRL].down)
		{
			strcpyn(gcon.buffer, util_get_clipboard());
			gcon.len = (int)strlen(gcon.buffer);
			gcon.cursor = gcon.len;
		}
		break;

	case K_ESCAPE:
		gcon.cur_hist = gcon.num_hist - 1;
		gcon.is_active = FALSE;
		break;

	case K_ENTER:
		if (!gcon.len)
			break;

		if (gcon.num_hist >= CON_MAX_HIST)
		{
			memcpy(gcon.history, gcon.history[1], sizeof(constr_t) * (CON_MAX_HIST - 1));
			strcpyn(gcon.history[gcon.num_hist - 1], gcon.buffer);
			gcon.cur_hist = gcon.num_hist - 1;
		}
		else
		{
			gcon.cur_hist = gcon.num_hist;
			strcpyn(gcon.history[gcon.num_hist], gcon.buffer);
			gcon.num_hist++;
		}

		con_printf(COLOR_ORANGE, "> %s", gcon.buffer);
		con_line_execute(gcon.buffer);

		memzero(gcon.buffer, sizeof(gcon.buffer));
		gcon.len = gcon.cursor = 0;
		break;

	case K_BACKSPACE:
		if (gcon.cursor > 0 && gcon.len > 0)
		{
			int sz;
			constr_t tmp;

			strcpyn(tmp, &gcon.buffer[gcon.cursor]);
			sz = (int)strlen(&gcon.buffer[gcon.cursor]);
			strncpy(&gcon.buffer[gcon.cursor - 1], tmp, sz);
			gcon.buffer[gcon.cursor + sz - 1] = '\0';

			gcon.cursor--;
			gcon.len--;
		}
		break;

	case K_DEL:
		if (gcon.cursor < gcon.len && gcon.len > 0)
		{
			int sz;
			constr_t tmp;

			strcpyn(tmp, &gcon.buffer[gcon.cursor + 1]);
			sz = (int)strlen(&gcon.buffer[gcon.cursor]);
			strncpy(&gcon.buffer[gcon.cursor], tmp, sz);
			gcon.buffer[gcon.cursor + sz] = '\0';

			gcon.len--;
		}
		break;

	case K_LEFTARROW:
		gcon.cursor--;
		if (gcon.cursor < 0)
			gcon.cursor = 0;
		break;

	case K_RIGHTARROW:
		gcon.cursor++;
		if (gcon.cursor >= gcon.len)
			gcon.cursor = gcon.len;
		break;

	case K_DOWNARROW:
		if (!gcon.num_hist)
			break;

		strcpyn(gcon.buffer, gcon.history[gcon.cur_hist++]);
		if (gcon.cur_hist >= gcon.num_hist)
			gcon.cur_hist = 0;

		gcon.len = (int)strlen(gcon.buffer);
		gcon.cursor = gcon.len;
		break;

	case K_UPARROW:
		if (!gcon.num_hist)
			break;

		strcpyn(gcon.buffer, gcon.history[gcon.cur_hist--]);
		if (gcon.cur_hist < 0)
			gcon.cur_hist = gcon.num_hist - 1;

		gcon.len = (int)strlen(gcon.buffer);
		gcon.cursor = gcon.len;
		break;

	case K_MWHEELUP:
		gcon.top_line--;
		if (gcon.top_line < 1)
			gcon.top_line = 1;
		break;

	case K_MWHEELDOWN:
		gcon.top_line++;
		if (gcon.top_line > gcon.num_lines)
			gcon.top_line = gcon.num_lines;
		break;
	}
}

void con_put_char(int ch)
{
	int sz;
	constr_t tmp;

	if (gcon.len > (sizeof(tmp) - 2) || gcon.cursor > (sizeof(tmp) - 2))
		return;

	strcpyn(tmp, &gcon.buffer[gcon.cursor]);
	sz = (int)strlen(&gcon.buffer[gcon.cursor]);
	gcon.buffer[gcon.cursor] = (char)tolower(ch);
	strncpy(&gcon.buffer[gcon.cursor + 1], tmp, sz);
	gcon.buffer[gcon.cursor + sz + 1] = '\0';

	gcon.cursor++;
	gcon.len++;
}

void con_draw()
{
	int i, x, y, size;

	if (!gcon.is_active)
		return;

	size = font_height(gconfont);
	x = font_print(gconfont, "]", 4, gvid.height - size - 4, RENDER_TRANSPARENT, gcolors[COLOR_ORANGE][0], gcolors[COLOR_ORANGE][1], gcolors[COLOR_ORANGE][2], 255);
	if (gcon.len)
		font_print(gconfont, gcon.buffer, x, gvid.height - size - 4, RENDER_TRANSPARENT, gcolors[COLOR_ORANGE][0], gcolors[COLOR_ORANGE][1], gcolors[COLOR_ORANGE][2], 255);

	if (gcon.underline)
	{
		if (gcon.cursor)
		{
			constr_t tmp;

			strncpy(tmp, gcon.buffer, gcon.cursor);
			tmp[gcon.cursor] = '\0';
			x += font_len(gconfont, tmp);
		}

		font_print(gconfont, "_", x, gvid.height - size - 2, RENDER_TRANSPARENT, gcolors[COLOR_ORANGE][0], gcolors[COLOR_ORANGE][1], gcolors[COLOR_ORANGE][2], 255);
	}

	y = gvid.height - (size << 1) - 2;
	if (gcon.top_line != gcon.num_lines)
	{
		font_print(gconfont, "^ ^ ^", 4, gvid.height - (size << 1) - 2, RENDER_TRANSPARENT, gcolors[COLOR_CYAN][0], gcolors[COLOR_CYAN][1], gcolors[COLOR_CYAN][2], 255);
		y -= size;
	}

	y -= 2;
	for (i = gcon.top_line - 1; y > -size && i >= 0; y -= size, i--)
	{
		if (gcon.lines[i][0])
			font_print(gconfont, gcon.lines[i], 4, y, RENDER_TRANSPARENT, gcolors[gcon.colors[i]][0], gcolors[gcon.colors[i]][1], gcolors[gcon.colors[i]][2], 255);
	}

	gcon.pulse -= ghost.frametime;
	if (gcon.pulse < 0)
	{
		gcon.pulse = CON_PULSE;
		gcon.underline = !gcon.underline;
	}
}

static void con_cfg_read()
{
	FILE* fp;
	constr_t line;

	fp = util_open(CON_FOLDER"config.cfg", "r");
	if (!fp)
		return;

	line[0] = '\0';
	line[sizeof(line) - 1] = '\0';
	line[sizeof(line) - 2] = '\0';

	while (fgets(line, sizeof(line), fp))
		con_line_execute(line);

	util_close(fp);
}

void con_cfg_save()
{
	int i;
	FILE* fp;
	convar_s* cvar;

	fp = util_open(CON_FOLDER"config.cfg", "w");
	if (!fp)
		return;

	in_binds_save(fp);
	for (i = 0; i < gcon.num_cvars; i++)
	{
		cvar = &gcon.cvars[i];
		if (!cvar->should_save)
			continue;

		if (cvar->value != cvar->initial)
			fprintf(fp, "%s %g\n", cvar->name, cvar->value);
	}

	util_close(fp);
}

void con_log_save(const char* error)
{
	int i;
	FILE* fp;
	string_t name;

	sprintf(name, "crash%s.log", util_get_timestamp());
	fp = fopen(name, "w");
	if (!fp)
		return;

	for (i = 0; i < gcon.num_lines; i++)
	{
		fputs(gcon.lines[i], fp);
		fputc('\n', fp);
	}

	fputs(error, fp);
	fclose(fp);
}

static void con_quit(const char* arg1, const char* arg2)
{
	host_shutdown();
}

static void con_list(const char* arg1, const char* arg2)
{
	int i;
	convar_s* cvar;

	for (i = 0; i < gcon.num_cvars; i++)
	{
		cvar = &gcon.cvars[i];
		if (!cvar->name)
			continue;

		if (cvar->action)
			con_printf(COLOR_WHITE, "[cmd ] %s", cvar->name);
		else
			con_printf(COLOR_WHITE, "[cvar] %s = %g (%g)", cvar->name, cvar->value, cvar->initial);
	}
}

static void con_modes(const char* arg1, const char* arg2)
{
	int i;

	for (i = 0; i < gvid.num_modes; i++)
		con_printf(COLOR_WHITE, "%i = %i x %i", i, gvid.modes[i][0], gvid.modes[i][1]);
}

static void con_usage(const char* arg1, const char* arg2)
{
	util_musage();
	con_print(COLOR_WHITE, " ");
	con_print(COLOR_WHITE, "RESOURCES");
	//TODO: bru limits
	con_printf(COLOR_WHITE, "%4i / %4i Console strings", gcon.num_lines, CON_MAX_STRINGS);
	con_printf(COLOR_WHITE, "%4i / %4i Console vars", gcon.num_cvars, CON_MAX_CVARS);
}

static void con_clear(const char* arg1, const char* arg2)
{
	gcon.num_lines = gcon.top_line = 0;
}

void con_init()
{//TODO: open command
	gcon.can_register = TRUE;
	util_create_folder("misc");

	con_create_cmd("map", world_map_cmd);
	con_create_cmd("bind", in_bind_cmd);
	con_create_cmd("unbind", in_unbind_cmd);
	con_create_cmd("unbindall", in_unding_all_cmd);
	con_create_cmd("bindlist", in_bindlist_cmd);
	con_create_cmd("scrshot", img_scrshot_cmd);
	con_create_cmd("quit", con_quit);
	con_create_cmd("list", con_list);
	con_create_cmd("modes", con_modes);
	con_create_cmd("endgame", world_end_map);
	con_create_cmd("usage", con_usage);
	con_create_cmd("clear", con_clear);

	gvid.mode = con_create_cvar("vid_mode", 0, TRUE);
	gvid.msaa = con_create_cvar("vid_msaa", FALSE, TRUE);
	gvid.fullscreen = con_create_cvar("vid_fullscreen", FALSE, TRUE);
	gvid.vsync = con_create_cvar("vid_vsync", FALSE, TRUE);

	gimg.aniso = con_create_cvar("r_aniso", TRUE, TRUE);
	gimg.nofilter = con_create_cvar("r_nearest", FALSE, TRUE);
	gworld.vbo = con_create_cvar("r_vbo", TRUE, TRUE);
	gworld.shade = con_create_cvar("r_shade", 0.2f, TRUE);
	ghost.fps = con_create_cvar("r_fps", 100, TRUE);

	gcon.test = con_create_cvar("test", FALSE, FALSE);

	gsnd.volume = con_create_cvar("s_volume", SND_DEF_VOL, TRUE);

	gworld.lock = con_create_cvar("dbg_lock", FALSE, FALSE);
	gworld.wireframe = con_create_cvar("dbg_wireframe", FALSE, FALSE);

	ggame.cvar_init();
	con_cfg_read();
	gcon.can_register = FALSE;
}