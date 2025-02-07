#include "cubement.h"

world_s gworld;

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

	img_start();
	if (!bru_load(ghost.newmap))
	{
		img_end();
		ghost.newmap[0] = '\0';
		return;
	}

	gworld.is_load = TRUE;
	ghost.newmap[0] = '\0';

	img_end();
	con_printf(COLOR_WHITE, "Level Load Time: %ims", (int)(1000 * (util_time() - timer)));
}