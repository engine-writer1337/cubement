#include <windows.h>
#include <conio.h>
#include <stdio.h>

#define FONT_DIMENSION	16
#define FONT_LETTERS	256

static int gpointsize;
static int gcanvspt;
static int gweight;
static int goutline;
static int gshadow;
static int gitalic;
static char goutname[1024];
static char gfontname[1024];

static const wchar_t goverride[32] =
{
	L' ', L'☺', L'☻', L'♥', L'♦', L'♣', L'♠', L'•', L'◘', L'○', L'◙', L'♂', L'♀', L'♪', L'♫',
	L'☼', L'►', L'◄', L'↕', L'‼', L'¶', L'§', L'▬', L'↨', L'↑', L'↓', L'→', L'←', L'∟', L'↔',
	L'▲', L'▼'
};

static void bmp_save(const char* name, byte* base, int width, int height, int outline, int shadow, int canvspt)
{
	FILE* fp;
	char outname[1024];
	byte* buffer, * p, * buf, * linedata, * combine, * dst, * b;
	int i, j, outsize, stride, color, count, checksum, m, l, k, w_end, x_start, w1, h1, x, y;

	outsize = width * height + 1024 + 54;
	buffer = calloc(outsize, 1);

	buffer[0] = 'B';
	buffer[1] = 'M';
	*(int*)(buffer + 2) = outsize;
	*(int*)(buffer + 10) = 1024 + 54;
	*(int*)(buffer + 14) = 40;
	*(int*)(buffer + 18) = width;
	*(int*)(buffer + 22) = height;
	*(short*)(buffer + 26) = 1;
	*(short*)(buffer + 28) = 8;
	*(int*)(buffer + 30) = BI_RGB;
	*(int*)(buffer + 34) = width * height;
	*(int*)(buffer + 46) = 256;
	*(int*)(buffer + 54) = 0xFFFFFFFF;
	*(int*)(buffer + 54 + 1020) = 0xFF0000FF;

	if (outline)
	{
		linedata = malloc(width * height);
		combine = malloc(width * height);
		memcpy(linedata, base, width * height);
		for (k = 0, i = 0; i < height; i++)
		{
			for (j = 0; j < width; j++, k++)
			{
				if (base[k])
					continue;

				for (m = -outline; m <= outline; m++)
				{
					if ((i + m) < 0 || (i + m) >= height)
						continue;

					p = linedata + ((i + m) * width) + j;
					for (l = -outline; l <= outline; l++)
					{
						if ((j + l) < 0 || (j + l) >= width)
							continue;

						*(p + l) = 127;
					}
				}
			}
		}

		memcpy(combine, linedata, width * height);
		for (k = 0, i = 0; i < height; i++)
		{
			for (j = 0; j < width; j++, k++)
			{
				if (!base[k])
					combine[k] = base[k];
			}
		}

		memcpy(buffer + 1024 + 54, combine, width * height);
		memset(linedata, 0, width * height);
	}
	else if (shadow)
	{
		linedata = malloc(width * height);
		combine = malloc(width * height);
		memcpy(linedata, base, width * height);
		for (k = 0, i = 0; i < height; i++)
		{
			for (j = 0; j < width; j++, k++)
			{
				if (base[k])
					continue;

				for (m = -shadow; m <= 0; m++)
				{
					if ((i + m) < 0 || (i + m) >= height)
						continue;

					p = linedata + ((i + m) * width) + j;
					for (l = 0; l <= shadow; l++)
					{
						if ((j + l) < 0 || (j + l) >= width)
							continue;

						*(p + l) = 127;
					}
				}
			}
		}

		memcpy(combine, linedata, width * height);
		for (k = 0, i = 0; i < height; i++)
		{
			for (j = 0; j < width; j++, k++)
			{
				if (!base[k])
					combine[k] = base[k];
			}
		}

		memcpy(buffer + 1024 + 54, combine, width * height);
		memset(linedata, 0, width * height);
	}
	else
		memcpy(buffer + 1024 + 54, base, width * height);

	sprintf(outname, "%s.bmp", name);
	fp = fopen(outname, "wb");
	if (fp)
	{
		fwrite(buffer, outsize, 1, fp);
		fclose(fp);
		printf("SAVE %s\n", outname);
	}

	free(buffer);
	outsize = (width * height) << 1;
	buffer = calloc(outsize, 1);

	memset(buffer, 0, outsize);
	*(int*)buffer = 'TNOF';
	*(unsigned short*)(buffer + 4) = (unsigned short)width;

	w1 = h1 = 0;
	p = buffer + 6;
	stride = -width;
	if (outline || shadow)
		buf = combine + (height - 1) * width;
	else
		buf = base + (height - 1) * width;

	for (i = 0; i < FONT_LETTERS; i++)
	{
		w_end = 0;
		x_start = 9999;

		dst = buf + (h1 * stride * canvspt) + (w1 * canvspt);
		if (++w1 >= FONT_DIMENSION)
			w1 = 0, h1++;

		for (y = 0; y < canvspt; y++)
		{
			for (x = 0; x < canvspt; x++)
			{
				b = dst + y * stride + x;
				if (b[0] != 255)
				{
					if (x < x_start)
						x_start = x;
					if (x > w_end)
						w_end = x;
				}
			}
		}

		if (!w_end || x_start == 9999)
			p[i] = 0;
		else
			p[i] = (byte)((w_end - x_start) + 1);
	}

	count = 0;
	color = -1;
	checksum = 0;
	p += FONT_LETTERS;

	for (i = 0; i < height; i++)
	{
		for (j = 0; j < width; j++)
		{
			if (color != buf[j])
			{
				if (count > 0)
				{
					p[0] = count;
					if (color == 255)
						p[1] = 0;
					else if (color == 127)
						p[1] = 2;
					else
						p[1] = 1;

					checksum += count;
					p += 2;
				}

				count = 0;
				color = buf[j];
			}

			if (color == buf[j])
			{
				if (count >= 255)
				{
					p[0] = count;
					if (color == 255)
						p[1] = 0;
					else if (color == 127)
						p[1] = 2;
					else
						p[1] = 1;

					checksum += count;
					p += 2;
					count = 0;
				}

				count++;
			}
		}

		buf += stride;
	}

	if (count > 0)
	{
		p[0] = count;
		if (color == 255)
			p[1] = 0;
		else if (color == 127)
			p[1] = 2;
		else
			p[1] = 1;

		checksum += count;
		p += 2;
	}

	if (checksum != width * height)
		printf("Checksum is bad. Got %i, Should %i\n", checksum, width * height);

	sprintf(outname, "%s.fnt", name);
	fp = fopen(outname, "wb");
	if (fp)
	{
		fwrite(buffer, p - buffer, 1, fp);
		fclose(fp);
		printf("SAVE %s\n", outname);
	}

	free(buffer);
	if (outline || shadow)
	{
		free(combine);
		free(linedata);
	}
}

static void fmake(const char* name, const char* outname, int pointsize, int canvspt, int weight, int outline, int shadow, int italic)
{
	char k;
	RECT rect;
	HFONT hfont;
	BITMAPINFO bmidib;
	HBITMAP bmpsection;
	HDC hdc, hdcsection;
	int i, w, h, x, y, j;
	byte* dibbase, * src, * base, * p;

	w = canvspt * FONT_DIMENSION;
	w = (w >> 2) << 2;
	h = w;

	hdc = GetDC(GetDesktopWindow());
	hfont = CreateFont(-pointsize, 0, 0, 0, weight, italic > 0, 0, 0, RUSSIAN_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, FF_DONTCARE | FIXED_PITCH, name);
	if (!hfont)
		return;

	memset(&bmidib, 0, sizeof(BITMAPINFO));
	bmidib.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmidib.bmiHeader.biWidth = w;
	bmidib.bmiHeader.biHeight = h;
	bmidib.bmiHeader.biPlanes = 1;
	bmidib.bmiHeader.biBitCount = 8;
	bmidib.bmiHeader.biCompression = BI_RGB;
	bmpsection = CreateDIBSection(hdc, &bmidib, DIB_PAL_COLORS, (void**)&dibbase, NULL, 0);
	if (!bmpsection)
	{
		DeleteObject(hfont);
		ReleaseDC(GetDesktopWindow(), hdc);
		printf("fmake: bmp section no create\n");
		return;
	}

	if (!(hdcsection = CreateCompatibleDC(hdc)))
	{
		DeleteObject(bmpsection);
		DeleteObject(hfont);
		ReleaseDC(GetDesktopWindow(), hdc);
		printf("fmake: DIB Init error!\n");
		return;
	}

	SelectObject(hdcsection, bmpsection);
	SelectObject(hdcsection, hfont);
	memset(dibbase, 255, w * h);
	SetBkMode(hdcsection, TRANSPARENT);

	x = y = 0;
	for (i = 0; i < FONT_LETTERS; i++)
	{
		k = (byte)i;
		rect.left = canvspt * x;
		rect.top = canvspt * y;
		rect.right = canvspt + rect.left;
		rect.bottom = canvspt + rect.top;
		if (i < 32)
			DrawTextW(hdcsection, &goverride[i], 1, &rect, DT_NOPREFIX | DT_VCENTER | DT_CENTER | DT_SINGLELINE);
		else
			DrawText(hdcsection, &k, 1, &rect, DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER | DT_CENTER);

		if (++x >= FONT_DIMENSION)
			x = 0, ++y;
	}

	src = dibbase;
	p = base = malloc(w * h);
	for (i = 0; i < h; i++)
	{
		for (j = 0; j < w; j++)
		{
			if (*src++ > 127)
				*p++ = 255;
			else
				*p++ = 0;
		}
	}

	bmp_save(outname, base, w, h, outline, shadow, canvspt);
	free(base);

	DeleteDC(hdcsection);
	DeleteObject(bmpsection);
	ReleaseDC(GetDesktopWindow(), hdc);
	DeleteObject(hfont);
}

static void fnt_default(void)
{
	strcpy(goutname, "font");
	strcpy(gfontname, "Arial");
	gpointsize = 8;
	gcanvspt = 12;
	gweight = goutline = gshadow = gitalic = 0;
}

int main(int argc, char* argv[])
{
	FILE* fp;
	int i, j;
	const char* tmp;
	char buffer[1024], option[1024], value[1024];

	if (argc == 1)
	{
		printf("Drag & Drop *.*\n");
		_getch();
		return 0;
	}

	fp = fopen(argv[1], "r");
	if (!fp)
	{
		printf("Can't open file %s\n", argv[1]);
		_getch();
		return 0;
	}

	fnt_default();
	while (fgets(buffer, sizeof(buffer) - 1, fp))
	{
		if (buffer[0] != '$')
			continue;

		sscanf(buffer, "%s", option);
		if (option[0] != '$')
			continue;

		i = 0;
		tmp = buffer + strlen(option);
		while (tmp[i] && isspace(tmp[i]))
			i++;

		tmp = tmp + i;
		strcpy(value, tmp);

		j = (int)strlen(value);
		if (value[j - 1] == '\n')
			value[j - 1] = '\0';

		if (!strcmp(option, "$outname"))
			strcpy(goutname, value);
		else if (!strcmp(option, "$fontname"))
			strcpy(gfontname, value);
		else if (!strcmp(option, "$pointsize"))
		{
			gpointsize = (atoi(value) >> 1) << 1;
			if (gpointsize < 8) gpointsize = 8;
			if (gpointsize > 64) gpointsize = 64;
		}
		else if (!strcmp(option, "$canvaspt"))
		{
			gcanvspt = (atoi(value) >> 1) << 1;
			if (gcanvspt < 8) gcanvspt = 8;
			if (gcanvspt > 64) gcanvspt = 64;
		}
		else if (!strcmp(option, "$weight"))
			gweight = atoi(value);
		else if (!strcmp(option, "$outline"))
		{
			goutline = atoi(value);
			if (goutline < 0) goutline = 0;
			if (goutline > 8) goutline = 8;
		}
		else if (!strcmp(option, "$shadow"))
		{
			gshadow = atoi(value);
			if (gshadow < 0) gshadow = 0;
			if (gshadow > 8) gshadow = 8;
		}
		else if (!strcmp(option, "$italic"))
			gitalic = atoi(value);
		else if (!strcmp(option, "$append"))
			fmake(gfontname, goutname, gpointsize, gcanvspt, gweight, goutline, gshadow, gitalic);
		else
			printf("bad command %s\n", option);
	}

	fclose(fp);
	printf("DONE\n");
	_getch();
	return 0;
}