#ifndef _UTIL_H_
#define _UTIL_H_

void* util_malloc(size_t size);
void* util_calloc(size_t count, size_t size);
void util_free(void* mem);
void util_musage();

FILE* util_open(const char* name, const char* mode);
void util_close(FILE* fp);

glpic_t util_tex_gen();
void util_tex_free(glpic_t t);

glbuf_t util_buf_gen();
void util_buf_free(glbuf_t buf);

void util_checksum();
bool_t util_exist(const char* filename);
byte* util_full(const char* filename, int* len);
const char* util_parse(const char* data, char* token);
ftime_t util_time();
char* util_fps();

void util_fatal(const char* text);
void util_fatalf(const char* text, ...);

char* util_get_clipboard();
void util_set_clipboard(const char* text);

void util_create_folder(const char* path);

char* util_get_timestamp();

float util_calc_fov(float* fov_x, float width, float height);
void util_adjust_fov(float* fov_x, float* fov_y, float width, float height);

hash_t util_hash_str(const char* string);

#endif