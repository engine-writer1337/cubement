#ifndef _CONSOLE_H_
#define _CONSOLE_H_

#define CON_PULSE		0.5f
#define CON_FOLDER		"misc/"

#define CON_MAX_HIST	24
#define CON_MAX_CVARS	512
#define CON_MAX_STRS	1024
#define CON_MAX_STRINGS	(CON_MAX_STRS + (CON_MAX_STRS >> 1))

typedef struct
{
	float value;
	bool_t is_change;
	bool_t should_save;

	float initial;
	conact_t action;

	hash_t hash;
	const char* name;
}convar_s;

typedef struct
{
	bool_t is_active;
	bool_t can_register;

	int num_lines, top_line;
	enum_t colors[CON_MAX_STRINGS];
	name_t lines[CON_MAX_STRINGS];

	int num_hist, cur_hist;
	name_t history[CON_MAX_HIST];

	int len, cursor;
	name_t buffer;

	float pulse;
	bool_t underline;

	int num_cvars;
	convar_s cvars[CON_MAX_CVARS];

	convar_s* test;
}console_s;

extern console_s gcon;

cvar_s* con_create_cvar2(const char* name, float initial, bool_t save);
void con_create_cmd(const char* name, conact_t action);

void con_print(color_e color, const char* text);
void con_printf(color_e color, const char* text, ...);

void con_line_execute(const char* line);
void con_keys(int key, bool_t down);
void con_put_char(int ch);

void con_draw();

void con_cfg_save();
void con_log_save(const char* error);
void con_init();

#endif