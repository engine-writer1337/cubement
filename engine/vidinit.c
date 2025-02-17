#include "cubement.h"

vid_s gvid;
PFNGLGENBUFFERSPROC glGenBuffers;
PFNGLBINDBUFFERPROC glBindBuffer;
PFNGLBUFFERDATAPROC glBufferData;
PFNGLDELETEBUFFERSPROC glDeleteBuffers;
PFNGLBUFFERSUBDATAPROC glBufferSubData;
PFNWGLSWAPBUFFERPROC wglSwapBuffers;
static PFNWGLSWAPINTERVALEXTPROC wglSwapInterval;
static PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormat;

void vid_rendermode(render_e mode)
{
	if (gvid.render == mode)
		return;

	gvid.render = mode;
	switch (mode)
	{
	default:
	case RENDER_NORMAL:
		glDisable(GL_BLEND);
		glDisable(GL_ALPHA_TEST);
		break;

	case RENDER_ALPHA:
		glEnable(GL_BLEND);
		glDisable(GL_ALPHA_TEST);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		break;

	case RENDER_TRANSPARENT:
		glDisable(GL_BLEND);
		glEnable(GL_ALPHA_TEST);
		break;

	case RENDER_ADDTIVE:
		glEnable(GL_BLEND);
		glDisable(GL_ALPHA_TEST);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		break;
	}
}

void vid_setup2d()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, gvid.width, gvid.height, 0, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_DEPTH_TEST);
	vid_rendermode(RENDER_NORMAL);
	glColor4ub(255, 255, 255, 255);
}

static void vid_msaa_init()
{
	glGetIntegerv(GL_MAX_SAMPLES, &gvid.max_msaa);
	if (gvid.max_msaa > 4)
		gvid.max_msaa = 4;

	if (gvid.max_msaa > 1)
		wglChoosePixelFormat = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
}

static void vid_glinit()
{
	int pixelformat = 0;
	PIXELFORMATDESCRIPTOR pfd = { 0 };

	pfd.nVersion = 1;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 24;
	pfd.cStencilBits = 8;
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;

	gvid.hdc = GetDC(gvid.hwnd);
	if (gvid.msaa->value && wglChoosePixelFormat)
	{
		dword numPixelFormats;
		int attribs[24] =
		{
			WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB, WGL_DRAW_TO_WINDOW_ARB, TRUE, WGL_SUPPORT_OPENGL_ARB, TRUE, WGL_DOUBLE_BUFFER_ARB,
			TRUE, WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB, WGL_COLOR_BITS_ARB, 32, WGL_ALPHA_BITS_ARB, 8, WGL_DEPTH_BITS_ARB, 24,
			WGL_STENCIL_BITS_ARB, 8, WGL_SAMPLE_BUFFERS_ARB, TRUE, WGL_SAMPLES_ARB, gvid.max_msaa, 0, 0
		};

		wglChoosePixelFormat(gvid.hdc, attribs, 0, 1, &pixelformat, &numPixelFormats);
		if (pixelformat)
			con_print(COLOR_GREEN, "MSAA x4 enabled");
	}

	if (!pixelformat)
		pixelformat = ChoosePixelFormat(gvid.hdc, &pfd);

	if (!pixelformat || !SetPixelFormat(gvid.hdc, pixelformat, &pfd))
		util_fatal("OpenGL driver not installed");

	if (gvid.hrc)
	{
		if (!wglMakeCurrent(gvid.hdc, gvid.hrc))
			util_fatal("Unable to update OpenGL context");
	}
	else
	{
		gvid.hrc = wglCreateContext(gvid.hdc);
		if (!gvid.hrc || !wglMakeCurrent(gvid.hdc, gvid.hrc))
			util_fatal("Unable to create OpenGL context");

		glDisable(GL_BLEND);
		glDisable(GL_CULL_FACE);
		glEnable(GL_TEXTURE_2D);
		glAlphaFunc(GL_GREATER, 0);
		glFrontFace(GL_CW);
		glShadeModel(GL_FLAT);
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LEQUAL);
		glClearColor(0.65f, 0.65f, 0.65f, 1);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glEnableClientState(GL_VERTEX_ARRAY);
		glLineWidth(3);

		glGenBuffers = (PFNGLGENBUFFERSPROC)wglGetProcAddress("glGenBuffers");
		glBindBuffer = (PFNGLBINDBUFFERPROC)wglGetProcAddress("glBindBuffer");
		glBufferData = (PFNGLBUFFERDATAPROC)wglGetProcAddress("glBufferData");
		glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)wglGetProcAddress("glDeleteBuffers");
		glBufferSubData = (PFNGLBUFFERSUBDATAPROC)wglGetProcAddress("glBufferSubData");
		wglSwapInterval = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");

		if (!glGenBuffers)
			util_fatal("VBO is not supported");

		if (!wglChoosePixelFormat)
			vid_msaa_init();
	}

	glViewport(0, 0, gvid.width, gvid.height);
	if (wglSwapInterval)
		wglSwapInterval(gvid.vsync->value ? 1 : 0);
}

void vid_fullscreen()
{
	DEVMODE	devmode;

	devmode.dmSize = sizeof(DEVMODE);
	devmode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;
	devmode.dmPelsWidth = gvid.modes[(int)gvid.mode->value][0];
	devmode.dmPelsHeight = gvid.modes[(int)gvid.mode->value][1];

	if (ChangeDisplaySettings(&devmode, CDS_FULLSCREEN))
	{
		con_print(COLOR_RED, "Couldn't set to fullscreen mode");
		gvid.fullscreen->value = FALSE;
		gvid.fullscreen->is_change = TRUE;
	}
}

static void vid_create()
{
	RECT r;
	HDC desk_hdc;
	int desk_w, desk_h;
	WNDCLASS wc = { 0 };

	wc.lpfnWndProc = (WNDPROC)in_keys;
	wc.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));
	wc.lpszClassName = WIN_CLASSNAME;
	gvid.hinst = wc.hInstance = GetModuleHandle(0);
	wc.hIcon = LoadIcon(wc.hInstance, MAKEINTRESOURCE(101));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);

	if (!RegisterClass(&wc))
		util_fatal("Couldn't register window");

	desk_hdc = GetDC(GetDesktopWindow());
	desk_w = GetDeviceCaps(desk_hdc, HORZRES);
	desk_h = GetDeviceCaps(desk_hdc, VERTRES);
	ReleaseDC(GetDesktopWindow(), desk_hdc);

	ChangeDisplaySettings(NULL, 0);
	if (gvid.fullscreen->value)
	{
		DEVMODE	devmode;

		r.left = r.top = 0;
		r.right = gvid.modes[(int)gvid.mode->value][0];
		r.bottom = gvid.modes[(int)gvid.mode->value][1];

		devmode.dmPelsWidth = r.right;
		devmode.dmPelsHeight = r.bottom;
		devmode.dmSize = sizeof(DEVMODE);
		devmode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;

		if (ChangeDisplaySettings(&devmode, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
		{
			con_print(COLOR_RED, "Couldn't set to fullscreen mode");
			gvid.fullscreen->value = FALSE;
			vid_create();
			return;
		}

		gvid.width = r.right;
		gvid.height = r.bottom;

		AdjustWindowRect(&r, WS_POPUP, FALSE);
		gvid.hwnd = CreateWindow(WIN_CLASSNAME, ggame.windowname, WS_POPUP, 0, 0, r.right - r.left, r.bottom - r.top, NULL, NULL, gvid.hinst, NULL);
	}
	else
	{
		int x, y;

		x = (desk_w >> 1) - (gvid.modes[(int)gvid.mode->value][0] >> 1);
		if (x < 0)
			x = 0;

		y = (desk_h >> 1) - (gvid.modes[(int)gvid.mode->value][1] >> 1);
		if (y < 0)
			y = 0;

		r.left = x;
		r.top = y;
		r.right = x + gvid.modes[(int)gvid.mode->value][0];
		r.bottom = y + gvid.modes[(int)gvid.mode->value][1];

		gvid.width = gvid.modes[(int)gvid.mode->value][0];
		gvid.height = gvid.modes[(int)gvid.mode->value][1];

		AdjustWindowRect(&r, WIN_FLAGS, FALSE);
		gvid.hwnd = CreateWindow(WIN_CLASSNAME, ggame.windowname, WIN_FLAGS, x, y, r.right - r.left, r.bottom - r.top, NULL, NULL, gvid.hinst, NULL);
	}

	if (!gvid.hwnd)
	{
		UnregisterClass(WIN_CLASSNAME, gvid.hinst);
		util_fatal("Couldn't create window");
	}

	gvid.mode->is_change = FALSE;
	gvid.fullscreen->is_change = FALSE;
	gvid.centr_x = r.left + ((r.right - r.left) >> 1);
	gvid.centr_y = r.top + ((r.bottom - r.top) >> 1);
	vid_glinit();
}

void vid_init()
{
	gvid.mode->value = clamp(0, gvid.mode->value, gvid.num_modes - 1);

	vid_create();

	UpdateWindow(gvid.hwnd);
	SetForegroundWindow(gvid.hwnd);
	SetFocus(gvid.hwnd);
	ShowWindow(gvid.hwnd, SW_NORMAL);
}

void vid_set_params()
{
	if (gvid.fullscreen->is_change || gvid.mode->is_change || gvid.msaa->is_change)
	{
		if (gvid.msaa->is_change)
		{
			bru_unload();
			res_unload();
			vid_dispose(TRUE);
		}
		else
			vid_dispose(FALSE);
		
		vid_init();		
		if (gvid.msaa->is_change)
		{
			bru_reload();
			res_reload();
		}
		else
			res_reload_font();

		gvid.msaa->is_change = FALSE;
	}

	if (gvid.vsync->is_change)
	{
		gvid.vsync->is_change = FALSE;
		if (wglSwapInterval)
			wglSwapInterval(gvid.vsync->value ? 1 : 0);
	}
}

void vid_dispose(bool_t dispose_hrc)
{
	if (dispose_hrc && gvid.hrc)
	{
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(gvid.hrc);
		gvid.hrc = NULL;
	}

	if (gvid.hdc)
		ReleaseDC(gvid.hwnd, gvid.hdc), gvid.hdc = NULL;

	if (gvid.hwnd)
		DestroyWindow(gvid.hwnd), gvid.hwnd = NULL;

	UnregisterClass(WIN_CLASSNAME, gvid.hinst);
	if (gvid.fullscreen->value)
		ChangeDisplaySettings(NULL, 0);
}

void vid_exist()
{
	HWND window;

	window = FindWindow(WIN_CLASSNAME, NULL);
	if (!window)
	{
		wglSwapBuffers = (PFNWGLSWAPBUFFERPROC)GetProcAddress(GetModuleHandle("opengl32.dll"), "wglSwapBuffers");
		if (!wglSwapBuffers)
			util_fatal("Unable to find wglSwapBuffers");
		return;
	}

	UpdateWindow(window);
	SetForegroundWindow(window);
	SetFocus(window);
	ShowWindow(window, SW_RESTORE);
	exit(EXIT_SUCCESS);
}

void vid_gen_modes()
{
	int i, j, t;
	DEVMODE mode;
	bool_t need_sort;

	j = t = 0;
	need_sort = FALSE;

	while (EnumDisplaySettings(NULL, j, &mode))
	{
		for (i = 0; i < gvid.num_modes; i++)
		{
			if (gvid.modes[i][0] == mode.dmPelsWidth && gvid.modes[i][1] == mode.dmPelsHeight)
				goto ignore;
		}

		if ((int)mode.dmPelsWidth < t)
			need_sort = TRUE;

		t = mode.dmPelsWidth;
		gvid.modes[gvid.num_modes][0] = mode.dmPelsWidth;
		gvid.modes[gvid.num_modes][1] = mode.dmPelsHeight;
		gvid.num_modes++;

		if (gvid.num_modes == WIN_MAX_MODES)
			break;

	ignore:
		j++;
	}

	if (!gvid.num_modes)
	{
		gvid.modes[gvid.num_modes][0] = 800;
		gvid.modes[gvid.num_modes][1] = 600;
		gvid.num_modes = 1;
	}

	if (need_sort)
	{
		for (i = 0; i < gvid.num_modes; i++)
		{
			for (j = i + 1; j < gvid.num_modes; j++)
			{
				if (gvid.modes[i][0] > gvid.modes[j][0])
				{
					t = gvid.modes[i][0];
					gvid.modes[i][0] = gvid.modes[j][0];
					gvid.modes[j][0] = t;

					t = gvid.modes[i][1];
					gvid.modes[i][1] = gvid.modes[j][1];
					gvid.modes[j][1] = t;
				}
			}
		}
	}
}

//==========================================================================//
// FAKE WINDOW
//==========================================================================//
static struct
{
	HDC HDC;
	HWND HWND;
	HGLRC HGLRC;
	WNDCLASSEX WND;
}_fake;

static void vid_fake_free()
{
	if (_fake.HGLRC)
	{
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(_fake.HGLRC);
	}

	if (_fake.HDC)
		ReleaseDC(_fake.HWND, _fake.HDC);

	if (_fake.HWND)
	{
		DestroyWindow(_fake.HWND);
		UnregisterClass("TestWindow", _fake.WND.hInstance);
	}
}

static bool_t vid_fake()
{
	int pixelFormat;
	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR), 1, PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, PFD_TYPE_RGBA,
		24, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 32, 8, 0, PFD_MAIN_PLANE, 0, 0, 0, 0
	};

	memset(&_fake.WND, 0, sizeof(WNDCLASSEX));
	_fake.HGLRC = NULL; _fake.HWND = NULL; _fake.HDC = NULL;

	_fake.WND.cbSize = sizeof(WNDCLASSEX);
	_fake.WND.lpfnWndProc = DefWindowProc;
	_fake.WND.hInstance = GetModuleHandle(NULL);
	_fake.WND.lpszClassName = "TestWindow";

	if (!RegisterClassEx(&_fake.WND))
		return FALSE;

	if (!(_fake.HWND = CreateWindowEx(0, "TestWindow", "FAKE", 0, 0, 0, 32, 32, 0, 0, _fake.WND.hInstance, 0)))
	{
		UnregisterClass("TestWindow", _fake.WND.hInstance);
		return FALSE;
	}

	if (!(_fake.HDC = GetDC(_fake.HWND)))
		return FALSE;

	if (!(pixelFormat = ChoosePixelFormat(_fake.HDC, &pfd)))
		return FALSE;

	if (!SetPixelFormat(_fake.HDC, pixelFormat, &pfd))
		return FALSE;

	if (!(_fake.HGLRC = wglCreateContext(_fake.HDC)))
		return FALSE;

	if (!wglMakeCurrent(_fake.HDC, _fake.HGLRC))
		return FALSE;

	return TRUE;
}

void vid_msaa_func()
{
	if (!gvid.msaa->value)
		return;

	if (!vid_fake())
	{
		vid_fake_free();
		return;
	}

	vid_msaa_init();
	vid_fake_free();
}