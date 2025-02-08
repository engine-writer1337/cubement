#ifndef _INPUT_H_
#define _INPUT_H_

#define IN_MOUSE_BUTTONS	8
#define MK_XBUTTON1			0x0020
#define MK_XBUTTON2			0x0040
#define MK_XBUTTON3			0x0080
#define MK_XBUTTON4			0x0100
#define MK_XBUTTON5			0x0200
#define WM_XBUTTONUP		0x020C
#define WM_XBUTTONDOWN		0x020B

typedef struct
{
	constr_t binding;
	bool_t down, hold;
}key_s;

typedef struct
{
	const char* name;
	int	keynum;
}keyname_s;

extern key_s gkeys[256];

int in_string_to_key(const char* str);
const char* in_key_to_string(int keynum);
void in_binds_save(FILE* fp);
void in_bind_cmd(const char* arg1, const char* arg2);
void in_unbind_cmd(const char* arg1, const char* arg2);
void in_unding_all_cmd(const char* arg1, const char* arg2);
void in_bindlist_cmd(const char* arg1, const char* arg2);

void in_reset_cursor_pos();
void in_set_cursor_pos(int x, int y);
void in_get_cursor_pos(int* x, int* y, bool_t client);
void in_show_cursor(bool_t show);

LRESULT in_keys(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam);
void in_gather();

#endif