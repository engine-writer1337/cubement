#include "cubement.h"

key_s gkeys[256];
static int gmouse_oldbuttons;
static const int gmouse_buttons[IN_MOUSE_BUTTONS] =
{
	MK_LBUTTON,
	MK_RBUTTON,
	MK_MBUTTON,
	MK_XBUTTON1,
	MK_XBUTTON2,
	MK_XBUTTON3,
	MK_XBUTTON4,
	MK_XBUTTON5
};

static const byte gscan_to_key[128] =
{
	0,27,'1','2','3','4','5','6','7','8','9','0','-','=',K_BACKSPACE,9,
	'q','w','e','r','t','y','u','i','o','p','[',']', 13 , K_CTRL,
	'a','s','d','f','g','h','j','k','l',';','\'','`',
	K_SHIFT,'\\','z','x','c','v','b','n','m',',','.','/',K_SHIFT,
	'*',K_ALT,' ',K_CAPSLOCK,
	K_F1,K_F2,K_F3,K_F4,K_F5,K_F6,K_F7,K_F8,K_F9,K_F10,
	K_PAUSE,K_SCROLLOCK,K_HOME,K_UPARROW,K_PGUP,K_KP_MINUS,K_LEFTARROW,K_KP_5,
	K_RIGHTARROW,K_KP_PLUS,K_END,K_DOWNARROW,K_PGDN,K_INS,K_DEL,
	0,0,0,K_F11,K_F12,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

static const keyname_s gkeynames[] =
{
	{"TAB", K_TAB}, {"ENTER", K_ENTER}, {"ESCAPE", K_ESCAPE}, {"SPACE", K_SPACE}, {"BACKSPACE", K_BACKSPACE},
	{"UPARROW", K_UPARROW}, {"DOWNARROW", K_DOWNARROW}, {"LEFTARROW", K_LEFTARROW}, {"RIGHTARROW", K_RIGHTARROW},
	{"ALT", K_ALT}, {"CTRL", K_CTRL}, {"SHIFT", K_SHIFT}, {"CAPSLOCK", K_CAPSLOCK}, {"SCROLLOCK", K_SCROLLOCK},
	{"F1", K_F1}, {"F2", K_F2}, {"F3", K_F3}, {"F4", K_F4}, {"F5", K_F5}, {"F6", K_F6}, {"F7", K_F7}, {"F8", K_F8},
	{"F9", K_F9}, {"F10", K_F10}, {"F11", K_F11}, {"F12", K_F12}, {"INS", K_INS}, {"DEL", K_DEL}, {"PGDN", K_PGDN},
	{"PGUP", K_PGUP}, {"HOME", K_HOME}, {"END", K_END}, {"MOUSE1", K_MOUSE1}, {"MOUSE2", K_MOUSE2}, {"MOUSE3", K_MOUSE3},
	{"MOUSE4", K_MOUSE4}, {"MOUSE5", K_MOUSE5}, {"MOUSE5", K_MOUSE6}, {"MOUSE5", K_MOUSE7}, {"MOUSE5", K_MOUSE8},
	{"MWHEELUP", K_MWHEELUP}, {"MWHEELDOWN", K_MWHEELDOWN}, {"KP_HOME", K_KP_HOME}, {"KP_UPARROW", K_KP_UPARROW},
	{"KP_PGUP", K_KP_PGUP}, {"KP_LEFTARROW", K_KP_LEFTARROW}, {"KP_5", K_KP_5}, {"KP_RIGHTARROW", K_KP_RIGHTARROW},
	{"KP_END", K_KP_END}, {"KP_DOWNARROW", K_KP_DOWNARROW}, {"KP_PGDN", K_KP_PGDN}, {"KP_ENTER", K_KP_ENTER},
	{"KP_INS", K_KP_INS}, {"KP_DEL", K_KP_DEL}, {"KP_SLASH", K_KP_SLASH}, {"KP_MINUS", K_KP_MINUS}, {"KP_PLUS", K_KP_PLUS},
	{"PAUSE", K_PAUSE}, {"SEMICOLON", ';'},
	{NULL, 0},
};

static int in_mapkey(int key)
{
	int	result, modified;
	bool_t is_extended = FALSE;

	modified = (key >> 16) & 255;
	if (modified > 127)
		return 0;

	if (key & (1 << 24))
		is_extended = TRUE;

	result = gscan_to_key[modified];
	if (!is_extended)
	{
		switch (result)
		{
		case K_HOME:		return K_KP_HOME;
		case K_UPARROW:		return K_KP_UPARROW;
		case K_PGUP:		return K_KP_PGUP;
		case K_LEFTARROW:	return K_KP_LEFTARROW;
		case K_RIGHTARROW:	return K_KP_RIGHTARROW;
		case K_END:			return K_KP_END;
		case K_DOWNARROW:	return K_KP_DOWNARROW;
		case K_PGDN:		return K_KP_PGDN;
		case K_INS:			return K_KP_INS;
		case K_DEL:			return K_KP_DEL;
		default:			return result;
		}
	}
	else
	{
		switch (result)
		{
		case K_PAUSE:	return K_KP_NUMLOCK;
		case 0x0D:		return K_KP_ENTER;
		case 0x2F:		return K_KP_SLASH;
		case 0xAF:		return K_KP_PLUS;
		default:		return result;
		}
	}
}

static bool_t in_allow_repeat(int key)
{
	if (gcon.is_active)
	{
		if (key == K_LEFTARROW || key == K_RIGHTARROW)
			return TRUE;
	}

	switch (key)
	{
	case K_BACKSPACE:
	case K_PAUSE:
	case K_PGUP:
	case K_KP_PGUP:
	case K_PGDN:
	case K_KP_PGDN:	return TRUE;
	default:		return FALSE;
	}
}

int in_string_to_key(const char* str)
{
	const keyname_s* kn;

	if (!str || !str[0])
		return -1;
	if (!str[1])
		return str[0];

	if (str[0] == '0' && str[1] == 'x' && strlen(str) == 4)
	{
		int	n1, n2;

		n1 = str[2];
		if (n1 >= '0' && n1 <= '9')
			n1 -= '0';
		else if (n1 >= 'a' && n1 <= 'f')
			n1 = n1 - 'a' + 10;
		else 
			n1 = 0;

		n2 = str[3];
		if (n2 >= '0' && n2 <= '9')
			n2 -= '0';
		else if (n2 >= 'a' && n2 <= 'f')
			n2 = n2 - 'a' + 10;
		else
			n2 = 0;

		return (n1 << 4) + n2;
	}

	for (kn = gkeynames; kn->name; kn++)
	{
		if (strcmpi(str, kn->name))
			return kn->keynum;
	}

	return -1;
}

const char* in_key_to_string(int keynum)
{
	int i, j;
	const keyname_s* kn;
	static char	tinystr[8];

	if (keynum < 0 || keynum > 255)
		return "-";

	if (keynum > 32 && keynum < 127 && keynum != '"' && keynum != ';' && keynum != K_SCROLLOCK)
	{
		tinystr[0] = keynum;
		tinystr[1] = '\0';
		return tinystr;
	}

	for (kn = gkeynames; kn->name; kn++)
	{
		if (keynum == kn->keynum)
			return kn->name;
	}

	i = keynum >> 4;
	j = keynum & 15;

	tinystr[0] = '0';
	tinystr[1] = 'x';
	tinystr[2] = i > 9 ? i - 10 + 'a' : i + '0';
	tinystr[3] = j > 9 ? j - 10 + 'a' : j + '0';
	tinystr[4] = '\0';
	return tinystr;
}

void in_set_bind(int keynum, const char* binding)
{
	if (keynum != -1)
		strcpyn(gkeys[keynum].binding, binding);
}

void in_binds_save(FILE* fp)
{
	int i;

	for (i = 0; i < 256; i++)
	{
		if (!gkeys[i].binding[0])
			continue;

		fputs("bind ", fp);
		fputs(in_key_to_string(i), fp);
		fputc(' ', fp);
		fputs(gkeys[i].binding, fp);
		fputc('\n', fp);
	}
}

void in_bind_cmd(const char* arg1, const char* arg2)
{
	int	b;

	if (arg1[0] && !arg2[0])
	{
		b = in_string_to_key(arg1);
		if (b == -1)
			con_printf(COLOR_RED, "\"%s\" isn't a valid key", arg1);
		else
		{
			if (gkeys[b].binding[0])
				con_printf(COLOR_ORANGE, "\"%s\" = \"%s\"", arg1, gkeys[b].binding);
			else 
				con_printf(COLOR_RED, "\"%s\" is not bound", arg1);
		}
		return;
	}

	if (!arg1[0] || !arg2[0])
	{
		con_print(COLOR_RED, "bind <key> [command] : attach a command to a key");
		return;
	}

	b = in_string_to_key(arg1);
	if (b == -1)
		con_printf(COLOR_RED, "\"%s\" isn't a valid key", arg1);
	else
		in_set_bind(b, arg2);
}

void in_unbind_cmd(const char* arg1, const char* arg2)
{
	int	b;

	if (!arg1[0])
	{
		con_printf(COLOR_RED, "unbind <key> : remove commands from a key");
		return;
	}

	b = in_string_to_key(arg1);
	if (b == -1)
		con_printf(COLOR_RED, "\"%s\" isn't a valid key", arg1);
	else
		gkeys[b].binding[0] = '\0';
}

void in_unding_all_cmd(const char* arg1, const char* arg2)
{
	int	i;

	for (i = 0; i < 256; i++)
	{
		if (gkeys[i].binding[0])
			gkeys[i].binding[0] = '\0';
	}
}

void in_bindlist_cmd(const char* arg1, const char* arg2)
{
	int	i;

	for (i = 0; i < 256; i++)
	{
		if (gkeys[i].binding[0])
			con_printf(COLOR_ORANGE, "%s \"%s\"", in_key_to_string(i), gkeys[i].binding);	
	}
}

static void in_game(int key, bool_t down)
{
	if (!gkeys[key].binding[0])
		return;

	if (gkeys[key].binding[0] == '+')
	{
		name_t cmd;
		char number[16];

		number[0] = ' ';
		_itoa(key, number + 1, 10);

		if (down)
		{
			strcpy(cmd, gkeys[key].binding);
			strcatn(cmd, number);
		}
		else
		{
			cmd[0] = '-';
			strcpy(cmd + 1, gkeys[key].binding + 1);
			strcatn(cmd, number);
		}
		con_line_execute(cmd);
	}
	else if (down)
		con_line_execute(gkeys[key].binding);
}

static void in_events(int key, bool_t down)
{
	gkeys[key].down = down;
	if (down)
	{
		if (!in_allow_repeat(key) && gkeys[key].hold)
			return;

		gkeys[key].hold = TRUE;
	}
	else
		gkeys[key].hold = FALSE;

	if (key == '`' || key == '~')
	{
		if (!down)
			return;

		if (gcon.is_active)
		{
			gcon.cur_hist = gcon.num_hist - 1;
			gcon.is_active = FALSE;
			in_show_cursor(TRUE);
		}
		else
			gcon.is_active = TRUE;
		return;
	}

	if (ggame.key_events(key, down))
		return;

	if (gcon.is_active)
		con_keys(key, down);
	else if (gworld.is_load && !ggame.pause_world())
		in_game(key, down);
}

static void in_mouse_event(int flags)
{
	int i;

	for (i = 0; i < IN_MOUSE_BUTTONS; i++)
	{
		if ((flags & (1 << i)) && !(gmouse_oldbuttons & (1 << i)))
			in_events(K_MOUSE1 + i, TRUE);

		if (!(flags & (1 << i)) && (gmouse_oldbuttons & (1 << i)))
			in_events(K_MOUSE1 + i, FALSE);
	}

	gmouse_oldbuttons = flags;
}

void in_reset_cursor_pos()
{//TODO: window caption grab fix
	SetCursorPos(gvid.centr_x, gvid.centr_y);
}

void in_set_cursor_pos(int x, int y)
{
	SetCursorPos(x, y);
}

void in_get_cursor_pos(int* x, int* y, bool_t client)
{
	POINT current_pos;

	GetCursorPos(&current_pos);
	if (client)
		ScreenToClient(gvid.hwnd, &current_pos);

	if (x)
		*x = current_pos.x;
	if (y)
		*y = current_pos.y;
}

void in_show_cursor(bool_t show)
{
	if (show)
		while (ShowCursor(TRUE) < 0);
	else
	{
		while (ShowCursor(FALSE) >= 0);
		in_reset_cursor_pos();
	}
}

LRESULT in_keys(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	int i, temp = 0;

	if (!gvid.hrc)
		return DefWindowProc(hwnd, umsg, wparam, lparam);

	switch (umsg)
	{
	case WM_CLOSE:
		host_shutdown();
		break;

	case WM_NCLBUTTONDBLCLK:
		return WM_NULL;

	case WM_SYSCOMMAND:
		if (wparam == SC_RESTORE && !IsIconic(gvid.hwnd))
			return WM_NULL;
		break;

	case WM_ACTIVATE:
		if (wparam == WA_INACTIVE && !gvid.inactive)
		{
			ggame.window_inactive();
			gvid.inactive = TRUE;
			if (gvid.fullscreen->value == 1)
			{
				ShowWindow(gvid.hwnd, SW_MINIMIZE);
				ChangeDisplaySettings(NULL, 0);
			}
		}
		else if ((wparam == WA_ACTIVE || wparam == WA_CLICKACTIVE) && gvid.inactive)
		{
			ggame.window_active();
			gvid.inactive = FALSE;
			if (gvid.fullscreen->value == 1)
			{
				ShowWindow(gvid.hwnd, SW_RESTORE);
				vid_fullscreen();
			}
		}
		break;

	case WM_MOUSEWHEEL:
		if (GET_WHEEL_DELTA_WPARAM(wparam) > 0)
		{
			in_events(K_MWHEELUP, TRUE);
			in_events(K_MWHEELUP, FALSE);
		}
		else
		{
			in_events(K_MWHEELDOWN, TRUE);
			in_events(K_MWHEELDOWN, FALSE);
		}
		break;

	case WM_MOVE:
		if (!gvid.fullscreen->value)
		{
			RECT rect;

			GetWindowRect(gvid.hwnd, &rect);
			AdjustWindowRect(&rect, GetWindowLong(gvid.hwnd, GWL_STYLE), FALSE);

			gvid.centr_x = rect.left + ((rect.right - rect.left) >> 1);
			gvid.centr_y = rect.top + ((rect.bottom - rect.top) >> 1);
		}
		break;

	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	case WM_MOUSEMOVE:
		for (i = 0; i < IN_MOUSE_BUTTONS; i++)
		{
			if (wparam & gmouse_buttons[i])
				temp |= (1 << i);
		}

		in_mouse_event(temp);
		break;

	case WM_KEYUP:
		in_events(in_mapkey(lparam), FALSE);
		break;

	case WM_KEYDOWN:
		in_events(in_mapkey(lparam), TRUE);
		break;

	case WM_CHAR:
		if (wparam == '`' || wparam == '~' || wparam == 168 || wparam == 184 || wparam < 32)
			break;

		if (ggame.char_events(wparam))
			break;

		if (gcon.is_active)
			con_put_char(wparam);
		break;
	}

	return DefWindowProc(hwnd, umsg, wparam, lparam);
}

void in_gather()
{
	MSG msg;

	while (PeekMessage(&msg, 0, 0, 0, PM_NOREMOVE))
	{
		if (!GetMessage(&msg, 0, 0, 0))
			host_shutdown();

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}