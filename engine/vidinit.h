#ifndef _VIDINIT_H_
#define _VIDINIT_H_

#define WIN_MAX_MODES	32
#define WIN_CLASSNAME	" _cment_ "
#define WIN_FLAGS		(WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX)

#define GL_CLAMP_TO_EDGE	0x812F
#define GL_MAX_SAMPLES		0x8D57

#define WGL_ACCELERATION_ARB		0x2003
#define WGL_FULL_ACCELERATION_ARB	0x2027
#define WGL_DRAW_TO_WINDOW_ARB		0x2001
#define WGL_SUPPORT_OPENGL_ARB		0x2010
#define WGL_DOUBLE_BUFFER_ARB		0x2011
#define WGL_PIXEL_TYPE_ARB			0x2013
#define WGL_TYPE_RGBA_ARB			0x202B
#define WGL_COLOR_BITS_ARB			0x2014
#define WGL_DEPTH_BITS_ARB			0x2022
#define WGL_SAMPLE_BUFFERS_ARB		0x2041
#define WGL_SAMPLES_ARB				0x2042
#define WGL_ALPHA_BITS_ARB			0x201B
#define WGL_STENCIL_BITS_ARB		0x2023

#define GL_TEXTURE_MAX_ANISOTROPY_EXT		0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT	0x84FF

typedef BOOL(WINAPI* PFNWGLSWAPBUFFERPROC) (HDC hdc);
typedef BOOL(WINAPI* PFNWGLSWAPINTERVALEXTPROC) (int interval);
typedef BOOL(WINAPI* PFNWGLCHOOSEPIXELFORMATARBPROC) (HDC hdc, const int* piAttribIList, const FLOAT* pfAttribFList, UINT nMaxFormats, int* piFormats, UINT* nNumFormats);

typedef struct
{
	HDC hdc;
	HWND hwnd;
	HGLRC hrc;
	HINSTANCE hinst;

	int max_msaa;

	int num_modes;
	int modes[WIN_MAX_MODES][2];

	bool_t inactive;
	int width, height;
	int centr_x, centr_y;

	convar_s* msaa;
	convar_s* mode;
	convar_s* fullscreen;
	convar_s* vsync;

	render_e render;
}vid_s;

extern vid_s gvid;
extern PFNWGLSWAPBUFFERPROC wglSwapBuffers;

void vid_rendermode(render_e mode);
void vid_setup2d();

void vid_fullscreen();
void vid_init();
void vid_set_params();
void vid_dispose(bool_t dispose_hrc);

void vid_exist();
void vid_gen_modes();

void vid_msaa_func();

#endif