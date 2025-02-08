#ifndef _VIDINIT_H_
#define _VIDINIT_H_

#define WIN_MAX_MODES	32
#define WIN_CLASSNAME	" _cment_ "
#define WIN_FLAGS		(WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX)

#define GL_ARRAY_BUFFER		0x8892
#define GL_STATIC_DRAW		0x88E4
#define GL_CLAMP_TO_EDGE	0x812F

#define GL_TEXTURE_MAX_ANISOTROPY_EXT		0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT	0x84FF

typedef BOOL(WINAPI* PFNWGLSWAPBUFFERPROC) (HDC hdc);
typedef BOOL(WINAPI* PFNWGLSWAPINTERVALEXTPROC) (int interval);
typedef void (WINAPI* PFNGLGENBUFFERSPROC) (GLsizei n, GLuint* buffers);
typedef void (WINAPI* PFNGLBINDBUFFERPROC) (GLenum target, GLuint buffer);
typedef void (WINAPI* PFNGLBUFFERDATAPROC) (GLenum target, size_t size, const GLvoid* data, GLenum usage);
typedef void (WINAPI* PFNGLDELETEBUFFERSPROC) (GLsizei n, const GLuint* buffers);
typedef void (WINAPI* PFNGLBUFFERSUBDATAPROC) (GLenum target, size_t offset, size_t size, const GLvoid* data);

typedef struct
{
	HDC hdc;
	HWND hwnd;
	HGLRC hrc;
	HINSTANCE hinst;

	int num_modes;
	int modes[WIN_MAX_MODES][2];

	bool_t inactive;
	int width, height;
	int centr_x, centr_y;

	cvar_s* mode;
	cvar_s* fullscreen;

	render_e render;
}vid_s;

extern vid_s gvid;
extern PFNWGLSWAPBUFFERPROC wglSwapBuffers;
extern PFNGLGENBUFFERSPROC glGenBuffers;
extern PFNGLBINDBUFFERPROC glBindBuffer;
extern PFNGLBUFFERDATAPROC glBufferData;
extern PFNGLDELETEBUFFERSPROC glDeleteBuffers;
extern PFNGLBUFFERSUBDATAPROC glBufferSubData;

void vid_rendermode(render_e mode);
void vid_setup2d();

void vid_fullscreen();
void vid_init();
void vid_set_params();
void vid_dispose(bool_t dispose_hrc);

void vid_exist();
void vid_gen_modes();

#endif