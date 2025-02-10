#include "cubement.h"

game_s ggame;
host_s ghost;
static engine_s gengine;
byte gbuffer[IMG_MAX_SIZE * 4];

__declspec(dllexport) BOOL NvOptimusEnablement = TRUE;
__declspec(dllexport) BOOL AmdPowerXpressRequestHighPerformance = TRUE;

static long _stdcall host_crash(PEXCEPTION_POINTERS info)
{
	string_t error;
	MEMORY_BASIC_INFORMATION mbi;

	if (gvid.fullscreen->value)
		ChangeDisplaySettings(NULL, 0);

	con_cfg_save();
	if (VirtualQuery(info->ExceptionRecord->ExceptionAddress, &mbi, sizeof(mbi)) == sizeof(mbi))
	{
		string_t module;

		if (GetModuleFileName((HMODULE)mbi.AllocationBase, module, sizeof(module)))
		{
			sprintf(error, "Runtime error: 0x%x code at '%s' - 0x%zx", info->ExceptionRecord->ExceptionCode, module,
				(size_t)info->ExceptionRecord->ExceptionAddress - (size_t)mbi.AllocationBase);

			con_log_save(error);
			util_fatal(error);
			return EXCEPTION_EXECUTE_HANDLER;
		}
	}

	sprintf(error, "Runtime error: 0x%x code", info->ExceptionRecord->ExceptionCode);
	con_log_save(error);
	util_fatal(error);
	return EXCEPTION_EXECUTE_HANDLER;
}

static void host_command(LPSTR lpCmdLine)
{
	int i, argc = 0;
	const char* argv[4];

	while (*lpCmdLine && argc < 4)
	{
		while (*lpCmdLine && ((*lpCmdLine <= 32) || (*lpCmdLine > 126)))
			lpCmdLine++;

		if (*lpCmdLine)
		{
			argv[argc++] = lpCmdLine;
			while (*lpCmdLine && ((*lpCmdLine > 32) && (*lpCmdLine <= 126)))
				lpCmdLine++;

			if (*lpCmdLine)
			{
				*lpCmdLine = '\0';
				lpCmdLine++;
			}
		}
	}

	for (i = 0; i < argc; i++)
	{
		if (strcmpi(argv[i], "-map"))
		{
			if (i + 1 < argc)
				strcpyn(ghost.newmap, argv[i + 1]);
		}
	}
}

static void engine_int()
{
	gengine.register_entity = ent_register;

	gengine.create_cmd = con_create_cmd;
	gengine.create_cvar = con_create_cvar2;

	gengine.reset_cursor_pos = in_reset_cursor_pos;
	gengine.show_cursor = in_show_cursor;
	gengine.set_cursor_pos = in_set_cursor_pos;
	gengine.get_cursor_pos = in_get_cursor_pos;

	gengine.precache_resource = res_precache;
	gengine.precache_resource_ex = res_precache_ex;
	gengine.get_resource_handle = res_find;

	gengine.font_height = font_height;
	gengine.font_len = font_len;
	gengine.font_print = font_print;

	gengine.pic_draw = img_pic_draw;

	gengine.set_view_fov = world_set_fov;
	gengine.set_view_org = world_view_org;
	gengine.set_view_ang = world_view_ang;

	gengine.sky_load = sky_load;
}

static void engine_update()
{
	gengine.entities = gents;
	gengine.num_entities = gnuments;

	gengine.time = ghost.time;
	gengine.frametime = ghost.frametime;
	gengine.gametime = ghost.gametime;

	gengine.world_load = gworld.is_load;
	gengine.console_active = gcon.is_active;

	gengine.width = gvid.width;
	gengine.height = gvid.height;
	gengine.centr_x = gvid.centr_x;
	gengine.centr_y = gvid.centr_y;
}

static void engine_fps()
{
	int w;
	char* str = util_fps();

	w = font_len(gconfont, str);
	font_print(gconfont, str, gvid.width - w, 0, RENDER_TRANSPARENT, 255, 255, 255, 255);
}

EXPORTFUNC void cubement(engine_s** e, game_s* g)
{
	static ftime_t oldtime, newtime;

	newtime = util_time();
	host_command(GetCommandLine());

	engine_int();
	*e = &gengine;
	memcpy(&ggame, g, sizeof(ggame));

	vid_exist();
	vid_gen_modes();

	con_init();
	SetUnhandledExceptionFilter(host_crash);

	vid_msaa_func();
	vid_init();
	ghost.precache = PRE_PERS;

	img_init();
	gconfont = res_precache("console.fnt");
	ggame.engine_init();

	ghost.precache = PRE_NOT;
	con_printf(COLOR_WHITE, "Game Load Time: %ims", (int)(1000 * (util_time() - newtime)));
	oldtime = util_time() - 0.01f;

	while (TRUE)
	{
		if (gvid.inactive)
			Sleep(1);

		newtime = util_time();
		ghost.frametime = newtime - oldtime;
		if (ghost.fps->value > 4 && ghost.frametime < (1.f / ghost.fps->value))
			continue;

		oldtime = newtime;
		ghost.time += ghost.frametime;

		if (ghost.newmap[0])
			world_load_map();

		in_gather();
		vid_set_params();
		img_set_param();
		//snd_set_param();

		engine_update();
		if (gworld.is_load && ggame.draw_world())
		{
			if (!ggame.pause_world())
			{
				ghost.gametime += ghost.frametime;
				ent_think();
			}

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			world_setup3d();
			world_draw();
			ggame.draw_3d();
		}
		else
			glClear(GL_COLOR_BUFFER_BIT);

		vid_setup2d();
		ggame.draw_2d();
		con_draw();

		engine_fps();
		wglSwapBuffers(gvid.hdc);
		//snd_update();
	}
}

void host_shutdown()
{
	con_cfg_save();
	//snd_shutdown();

	sky_free();
	bru_free();

	img_free();
	res_free_all();
	vid_dispose(TRUE);

	util_checksum();

	PostQuitMessage(EXIT_SUCCESS);
	exit(EXIT_SUCCESS);
}