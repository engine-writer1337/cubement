#include "cubement.h"
#include <sys/stat.h>
#include <direct.h>
#include <psapi.h>

void* util_malloc(size_t size)
{
	if (size < 1)
		return NULL;

	ghost.memory++;
	return malloc(size);
}

void* util_calloc(size_t count, size_t size)
{
	if (size < 1 || count < 1)
		return NULL;

	ghost.memory++;
	return calloc(count, size);
}

void util_free(void* mem)
{
	if (!mem)
		return;

	ghost.memory--;
	free(mem);
}

void util_musage()
{
	PROCESS_MEMORY_COUNTERS pmc;

	if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
	{
		con_print(COLOR_WHITE, "MEMORY");
		con_print(COLOR_WHITE, "Using / Peak");
		con_printf(COLOR_WHITE, "%5i /%5i MB", pmc.WorkingSetSize >> 20, pmc.PeakWorkingSetSize >> 20);
		con_print(COLOR_WHITE, " ");
	}

	con_print(COLOR_WHITE, "CHECK SUMS");
	con_printf(COLOR_WHITE, "%4i memory", ghost.memory);
	con_printf(COLOR_WHITE, "%4i files", ghost.files);
	con_printf(COLOR_WHITE, "%4i textures", ghost.textures);
	con_printf(COLOR_WHITE, "%4i buffers", ghost.buffers);
}

FILE* util_open(const char* name, const char* mode)
{
	FILE* fp;

	fp = fopen(name, mode);
	if (!fp)
		return NULL;

	ghost.files++;
	return fp;
}

void util_close(FILE* fp)
{
	if (!fp)
		return;

	ghost.files--;
	fclose(fp);
}

glpic_t util_tex_gen()
{
	glpic_t gen;

	ghost.textures++;
	glGenTextures(1, &gen);
	return gen;
}

void util_tex_free(glpic_t t)
{
	if (!t)
		return;

	ghost.textures--;
	glDeleteTextures(1, &t);
}

glbuf_t util_buf_gen()
{
	glbuf_t buf;

	ghost.buffers++;
	glGenBuffers(1, &buf);
	return buf;
}

void util_buf_free(glbuf_t buf)
{
	if (!buf)
		return;

	ghost.buffers--;
	glDeleteBuffers(1, &buf);
}

void util_checksum()
{
	string_t msg;

	if (!ghost.textures && !ghost.memory && !ghost.files && !ghost.buffers)
		return;

	sprintf(msg, "Textures: %i\nMemory: %i\nFiles: %i\nVBOs: %i", ghost.textures, ghost.memory, ghost.files, ghost.buffers);
	MessageBox(NULL, msg, "Error checksum", MB_ICONERROR | MB_OK | MB_SYSTEMMODAL);
}

bool_t util_exist(const char* filename)
{
	struct stat buf;

	return stat(filename, &buf) != -1;
}

byte* util_full(const char* filename, int* len)
{
	FILE* fp;
	byte* data;
	int length;

	fp = util_open(filename, "rb");
	if (!fp)
		return NULL;

	fseek(fp, 0, SEEK_END);
	length = ftell(fp);
	if (!length)
	{
		util_close(fp);
		return NULL;
	}

	data = util_malloc(length);
	fseek(fp, 0, SEEK_SET);
	fread(data, sizeof(byte), length, fp);
	util_close(fp);
	if (len)
		*len = length;

	return data;
}

const char* util_parse(const char* data, char* token)
{
	int	c, len;

	if (!token)
		return NULL;

	len = 0;
	token[0] = 0;
	if (!data)
		return NULL;

skipwhite:
	while ((c = ((byte)*data)) <= ' ')
	{
		if (c == '\0')
			return NULL;// end of file
		data++;
	}

	if (c == '/' && data[1] == '/')
	{// skip // comments
		while (*data && *data != '\n')
			data++;
		goto skipwhite;
	}

	if (c == '\"')
	{// handle quoted strings specially
		data++;
		while (TRUE)
		{
			c = (byte)*data;
			if (!c)
			{// unexpected line end
				token[len] = 0;
				return data;
			}

			data++;
			if (c == '\\' && *data == '"')
			{
				token[len++] = *data++;
				continue;
			}

			if (c == '\"')
			{
				token[len] = 0;
				return data;
			}

			token[len++] = c;
		}
	}

	if (c == '{' || c == '}' || c == ')' || c == '(' || c == '\'' || c == ',')
	{// parse single characters
		token[len++] = c;
		token[len] = 0;
		return data + 1;
	}

	do
	{// parse a regular word
		token[len++] = c;
		data++;
		c = ((byte)*data);
		if (c == '{' || c == '}' || c == ')' || c == '(' || c == '\'' || c == ',')
			break;

	} while (c > 32);

	token[len] = 0;
	return data;
}

ftime_t util_time()
{
	LARGE_INTEGER current_time;
	static LARGE_INTEGER performance_frequency, clock_start;
	
	if (!performance_frequency.QuadPart)
	{
		QueryPerformanceFrequency(&performance_frequency);
		QueryPerformanceCounter(&clock_start);
	}

	QueryPerformanceCounter(&current_time);
	return (ftime_t)(current_time.QuadPart - clock_start.QuadPart) / (ftime_t)(performance_frequency.QuadPart);
}

char* util_fps()
{
	ftime_t newtime;
	static name_t fps;
	static int framecount;
	static ftime_t nexttime, framerate, lasttime;

	newtime = util_time();
	if (newtime >= nexttime)
	{
		framerate = framecount / (newtime - lasttime);
		lasttime = newtime;
		nexttime = max(nexttime + 1.0, lasttime - 1.0);
		framecount = 0;
	}

	framecount++;
	strcat(_itoa(framerate, fps, 10), "fps");
	return fps;
}

void util_fatal(const char* text)
{
	MessageBox(NULL, text, "Error", MB_ICONERROR | MB_OK | MB_SYSTEMMODAL);
	exit(1);
}

void util_fatalf(const char* text, ...)
{
	va_list	args;
	string_t error;

	va_start(args, text);
	_vsnprintf(error, sizeof(error), text, args);
	va_end(args);

	MessageBox(NULL, text, "Error", MB_ICONERROR | MB_OK | MB_SYSTEMMODAL);
	exit(1);
}

char* util_get_clipboard()
{
	ghost.string[0] = '\0';
	if (OpenClipboard(NULL))
	{
		HANDLE hclipboarddata = GetClipboardData(CF_TEXT);
		if (hclipboarddata)
		{
			char* cliptext = (char*)GlobalLock(hclipboarddata);
			if (cliptext)
			{
				strcpyn(ghost.string, cliptext);
				GlobalUnlock(hclipboarddata);
			}
		}

		CloseClipboard();
	}

	return ghost.string;
}

void util_set_clipboard(const char* text)
{
	int len;

	len = (int)strlen(text) + 1;
	if (OpenClipboard(NULL))
	{
		HGLOBAL hresult = GlobalAlloc(GMEM_MOVEABLE, len);
		char* buffercopy = (char*)GlobalLock(hresult);

		strcpy(buffercopy, text);
		GlobalUnlock(hresult);
		EmptyClipboard();

		if (!SetClipboardData(CF_TEXT, hresult))
		{
			GlobalFree(hresult);
			con_print(COLOR_RED, "Unable to copy text");
		}
		else
			con_print(COLOR_GREEN, "Text copied");

		CloseClipboard();
	}
}

void util_create_folder(const char* path)
{
	struct stat st;

	if (stat(path, &st) == -1)
	{
		if (_mkdir(path) == -1)
			con_printf(COLOR_RED, "Can't create %s folder", path);
	}
}

char* util_get_timestamp()
{
	FILETIME filet;
	SYSTEMTIME syst;

	GetSystemTimeAsFileTime(&filet);
	FileTimeToSystemTime(&filet, &syst);

	sprintf(ghost.string, "_%02d-%02d-%04d_%02d-%02d-%02d-%03d", syst.wDay, syst.wMonth, syst.wYear, syst.wHour, syst.wMinute, syst.wSecond, syst.wMilliseconds);
	return ghost.string;
}

float util_calc_fov(float* fov_x, float width, float height)
{
	float x, half_fov_y;

	if (*fov_x < 1.0f || *fov_x > 179.0f)
		*fov_x = 90.0f;

	x = width / tanf(deg2rad(*fov_x) * 0.5f);
	half_fov_y = atanf(height / x);
	return rad2deg(half_fov_y) * 2;
}

void util_adjust_fov(float* fov_x, float* fov_y, float width, float height)
{
	float x, y;

	if (width * 3 == 4 * height || width * 4 == height * 5)
		return;

	y = util_calc_fov(fov_x, 640, 480);
	x = *fov_x;

	*fov_x = util_calc_fov(&y, height, width);
	if (*fov_x < x)
		*fov_x = x;
	else
		*fov_y = y;
}

hash_t util_hash_str(const char* string)
{
	hash_t i, hashkey = 0;

	for (i = 0; string[i]; i++)
		hashkey = (hashkey + i) * 37 + tolower(string[i]);
	return hashkey;
}