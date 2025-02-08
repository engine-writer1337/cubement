/*
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.
*/
#include "miniz.h"
#include "crclib.h"
#include "minipng.h"

#define ntohl(n) ( (((n) & 0xFF000000) >> 24) | (((n) & 0x00FF0000) >> 8) \
	 | (((n) & 0x0000FF00) << 8) | (((n) & 0x000000FF) << 24) )

static const char png_sign[] = { 0x89, 'P', 'N', 'G', '\r', '\n', 0x1a, '\n' };
static const char ihdr_sign[] = { 'I', 'H', 'D', 'R' };
static const char trns_sign[] = { 't', 'R', 'N', 'S' };
static const char plte_sign[] = { 'P', 'L', 'T', 'E' };
static const char idat_sign[] = { 'I', 'D', 'A', 'T' };
static const char iend_sign[] = { 'I', 'E', 'N', 'D' };
static const int  iend_crc32 = 0xAE426082;

const char* minipng_size(const byte* buffer, int filesize, int* out_width, int* out_height)
{
	png_t png_hdr;

	if (filesize < sizeof(png_hdr))
		return "Bad size";

	memcpy(&png_hdr, buffer, sizeof(png_t));
	if (memcmp(png_hdr.sign, png_sign, sizeof(png_sign)))
		return "Invalid PNG signature";

	png_hdr.ihdr_len = ntohl(png_hdr.ihdr_len);
	if (png_hdr.ihdr_len != sizeof(png_ihdr_t))
		return "Invalid IHDR chunk size";
	if (memcmp(png_hdr.ihdr_sign, ihdr_sign, sizeof(ihdr_sign)))
		return "IHDR chunk corrupted";

	*out_height = ntohl(png_hdr.ihdr_chunk.height);
	*out_width = ntohl(png_hdr.ihdr_chunk.width);
	return NULL;
}

const char* minipng_load(const byte* buffer, int filesize, byte* rgba, int* has_alpha)
{
	int ret;
	png_t png_hdr;
	int has_iend_chunk = 0;
	z_stream stream = { 0 };
	short p, a, b, c, pa, pb, pc;
	byte* pallete = NULL, * trns = NULL;
	byte* buf_p, * pixbuf, * raw, * prior, * idat_buf = NULL, * uncompressed_buffer = NULL;
	uint32_t chunk_len, trns_len, plte_len, crc32, crc32_check, oldsize = 0, newsize = 0, rowsize;
	uint32_t uncompressed_size, pixel_size, pixel_count, i, y, filter_type, chunk_sign, r_alpha, g_alpha, b_alpha, width, height;

	if (filesize < sizeof(png_hdr))
		return "Bad size";

	buf_p = (byte*)buffer;
	memcpy(&png_hdr, buffer, sizeof(png_t));
	if (memcmp(png_hdr.sign, png_sign, sizeof(png_sign)))
		return "Invalid PNG signature";

	png_hdr.ihdr_len = ntohl(png_hdr.ihdr_len);
	if (png_hdr.ihdr_len != sizeof(png_ihdr_t))
		return "Invalid IHDR chunk size";
	if (memcmp(png_hdr.ihdr_sign, ihdr_sign, sizeof(ihdr_sign)))
		return "IHDR chunk corrupted";

	height = png_hdr.ihdr_chunk.height = ntohl(png_hdr.ihdr_chunk.height);
	width = png_hdr.ihdr_chunk.width = ntohl(png_hdr.ihdr_chunk.width);
	if (png_hdr.ihdr_chunk.height == 0 || png_hdr.ihdr_chunk.width == 0)
		return "Invalid image size";
	if (png_hdr.ihdr_chunk.bitdepth != 8)
		return "Only 8-bit images is supported";
	if (!(png_hdr.ihdr_chunk.colortype == PNG_CT_RGB || png_hdr.ihdr_chunk.colortype == PNG_CT_RGBA
		|| png_hdr.ihdr_chunk.colortype == PNG_CT_GREY || png_hdr.ihdr_chunk.colortype == PNG_CT_ALPHA
		|| png_hdr.ihdr_chunk.colortype == PNG_CT_PALLETE))
		return "Unknown color type";
	if (png_hdr.ihdr_chunk.compression > 0)
		return "Unknown compression method";
	if (png_hdr.ihdr_chunk.filter > 0)
		return "Unknown filter type";
	if (png_hdr.ihdr_chunk.interlace == 1)
		return "Adam7 Interlacing not supported";
	if (png_hdr.ihdr_chunk.interlace > 0)
		return "Unknown interlacing type";

	CRC32_Init(&crc32_check);
	CRC32_ProcessBuffer(&crc32_check, buf_p + sizeof(png_hdr.sign) + sizeof(png_hdr.ihdr_len), png_hdr.ihdr_len + sizeof(png_hdr.ihdr_sign));
	crc32_check = CRC32_Final(crc32_check);
	if (ntohl(png_hdr.ihdr_crc32) != crc32_check)
		return "IHDR chunk has wrong CRC32 sum";

	buf_p += sizeof(png_hdr);
	while (!has_iend_chunk && (buf_p - buffer) < filesize)
	{
		memcpy(&chunk_len, buf_p, sizeof(chunk_len));
		chunk_len = ntohl(chunk_len);
		if (chunk_len > INT_MAX)
		{
			if (idat_buf) 
				free(idat_buf);
			return "Found chunk with wrong size";
		}
		if ((int)chunk_len > filesize - (buf_p - buffer))
		{
			if (idat_buf)
				free(idat_buf);
			return "Found chunk with size past file size";
		}

		buf_p += sizeof(chunk_len);
		if (!memcmp(buf_p, trns_sign, sizeof(trns_sign)))
		{
			trns = buf_p + sizeof(trns_sign);
			trns_len = chunk_len;
		}
		else if (!memcmp(buf_p, plte_sign, sizeof(plte_sign)))
		{
			pallete = buf_p + sizeof(plte_sign);
			plte_len = chunk_len / 3;
		}
		else if (!memcmp(buf_p, idat_sign, sizeof(idat_sign)))
		{
			newsize = oldsize + chunk_len;
			if (!idat_buf)
				idat_buf = malloc(newsize);
			else
				idat_buf = realloc(idat_buf, newsize);
			memcpy(idat_buf + oldsize, buf_p + sizeof(idat_sign), chunk_len);
			oldsize = newsize;
		}
		else if (!memcmp(buf_p, iend_sign, sizeof(iend_sign)))
			has_iend_chunk = 1;

		CRC32_Init(&crc32_check);
		CRC32_ProcessBuffer(&crc32_check, buf_p, chunk_len + sizeof(idat_sign));
		crc32_check = CRC32_Final(crc32_check);

		buf_p += sizeof(chunk_sign);
		buf_p += chunk_len;
		memcpy(&crc32, buf_p, sizeof(crc32));
		if (ntohl(crc32) != crc32_check)
		{
			if (idat_buf) 
				free(idat_buf);
			return "Found chunk with wrong CRC32 sum";
		}

		buf_p += sizeof(crc32);
	}

	if (!oldsize)
		return "Couldn't find IDAT chunks";
	if (png_hdr.ihdr_chunk.colortype == PNG_CT_PALLETE && !pallete)
	{
		free(idat_buf);
		return "PLTE chunk not found";
	}
	if (!has_iend_chunk)
	{
		free(idat_buf);
		return "IEND chunk not found";
	}
	if (chunk_len != 0)
	{
		free(idat_buf);
		return "IEND chunk has wrong size";
	}

	switch (png_hdr.ihdr_chunk.colortype)
	{
	case PNG_CT_GREY:
	case PNG_CT_PALLETE:
		pixel_size = 1;
		break;
	case PNG_CT_ALPHA:
		pixel_size = 2;
		break;
	case PNG_CT_RGB:
		pixel_size = 3;
		break;
	case PNG_CT_RGBA:
		pixel_size = 4;
		break;
	default:
		pixel_size = 0;
		break;
	}

	*has_alpha = 0;
	pixel_count = height * width;
	if (trns || (png_hdr.ihdr_chunk.colortype & PNG_CT_ALPHA))
		*has_alpha = 1;

	rowsize = pixel_size * width;
	uncompressed_size = height * (rowsize + 1);
	uncompressed_buffer = malloc(uncompressed_size);

	stream.next_in = idat_buf;
	stream.total_in = stream.avail_in = newsize;
	stream.next_out = uncompressed_buffer;
	stream.total_out = stream.avail_out = uncompressed_size;

	if (inflateInit2(&stream, MAX_WBITS) != Z_OK)
	{
		free(uncompressed_buffer);
		free(idat_buf);
		return "IDAT chunk decompression failed";
	}

	ret = inflate(&stream, Z_NO_FLUSH);
	inflateEnd(&stream);
	free(idat_buf);
	if (ret != Z_OK && ret != Z_STREAM_END)
	{
		free(uncompressed_buffer);
		return "IDAT chunk decompression failed";
	}

	i = 0;
	prior = pixbuf = rgba;
	raw = uncompressed_buffer;
	if (png_hdr.ihdr_chunk.colortype != PNG_CT_RGBA)
		prior = pixbuf = raw;

	filter_type = *raw++;
	switch (filter_type)
	{
	case PNG_F_NONE:
	case PNG_F_UP:
		for (; i < rowsize; i++)
			pixbuf[i] = raw[i];
		break;
	case PNG_F_SUB:
	case PNG_F_PAETH:
		for (; i < pixel_size; i++)
			pixbuf[i] = raw[i];

		for (; i < rowsize; i++)
			pixbuf[i] = raw[i] + pixbuf[i - pixel_size];
		break;
	case PNG_F_AVERAGE:
		for (; i < pixel_size; i++)
			pixbuf[i] = raw[i];

		for (; i < rowsize; i++)
			pixbuf[i] = raw[i] + (pixbuf[i - pixel_size] >> 1);
		break;
	default:
		free(uncompressed_buffer);
		return "Found unknown filter type";
	}

	for (y = 1; y < height; y++)
	{
		i = 0;
		pixbuf += rowsize;
		raw += rowsize;
		filter_type = *raw++;

		switch (filter_type)
		{
		case PNG_F_NONE:
			for (; i < rowsize; i++)
				pixbuf[i] = raw[i];
			break;
		case PNG_F_SUB:
			for (; i < pixel_size; i++)
				pixbuf[i] = raw[i];

			for (; i < rowsize; i++)
				pixbuf[i] = raw[i] + pixbuf[i - pixel_size];
			break;
		case PNG_F_UP:
			for (; i < rowsize; i++)
				pixbuf[i] = raw[i] + prior[i];
			break;
		case PNG_F_AVERAGE:
			for (; i < pixel_size; i++)
				pixbuf[i] = raw[i] + (prior[i] >> 1);

			for (; i < rowsize; i++)
				pixbuf[i] = raw[i] + ((pixbuf[i - pixel_size] + prior[i]) >> 1);
			break;
		case PNG_F_PAETH:
			for (; i < pixel_size; i++)
				pixbuf[i] = raw[i] + prior[i];

			for (; i < rowsize; i++)
			{
				a = pixbuf[i - pixel_size];
				b = prior[i];
				c = prior[i - pixel_size];
				p = a + b - c;
				pa = abs(p - a);
				pb = abs(p - b);
				pc = abs(p - c);

				pixbuf[i] = raw[i];

				if (pc < pa && pc < pb)
					pixbuf[i] += c;
				else if (pb < pa)
					pixbuf[i] += b;
				else
					pixbuf[i] += a;
			}
			break;
		default:
			free(uncompressed_buffer);
			return "Found unknown filter type";
		}

		prior = pixbuf;
	}

	pixbuf = rgba;
	raw = uncompressed_buffer;
	switch (png_hdr.ihdr_chunk.colortype)
	{
	case PNG_CT_RGB:
		if (trns)
		{
			r_alpha = trns[0] << 8 | trns[1];
			g_alpha = trns[2] << 8 | trns[3];
			b_alpha = trns[4] << 8 | trns[5];
		}

		for (y = 0; y < pixel_count; y++, raw += pixel_size, pixbuf += 4)
		{
			pixbuf[0] = raw[0];
			pixbuf[1] = raw[1];
			pixbuf[2] = raw[2];

			if (trns && r_alpha == raw[0] && g_alpha == raw[1] && b_alpha == raw[2])
				pixbuf[3] = 0;
			else
				pixbuf[3] = 0xFF;
		}
		break;
	case PNG_CT_GREY:
		if (trns)
			r_alpha = trns[0] << 8 | trns[1];

		for (y = 0; y < pixel_count; y++, raw += pixel_size, pixbuf += 4)
		{
			pixbuf[0] = raw[0];
			pixbuf[1] = raw[0];
			pixbuf[2] = raw[0];

			if (trns && r_alpha == raw[0])
				pixbuf[3] = 0;
			else
				pixbuf[3] = 0xFF;
		}
		break;
	case PNG_CT_ALPHA:
		for (y = 0; y < pixel_count; y++, raw += pixel_size, pixbuf += 4)
		{
			pixbuf[0] = raw[0];
			pixbuf[1] = raw[0];
			pixbuf[2] = raw[0];
			pixbuf[3] = raw[1];
		}
		break;
	case PNG_CT_PALLETE:
		for (y = 0; y < pixel_count; y++, raw += pixel_size, pixbuf += 4)
		{
			if (raw[0] < plte_len)
			{
				pixbuf[0] = pallete[3 * raw[0] + 0];
				pixbuf[1] = pallete[3 * raw[0] + 1];
				pixbuf[2] = pallete[3 * raw[0] + 2];

				if (trns && raw[0] < trns_len)
					pixbuf[3] = trns[raw[0]];
				else
					pixbuf[3] = 0xFF;
			}
			else
			{
				pixbuf[0] = 0;
				pixbuf[1] = 0;
				pixbuf[2] = 0;
				pixbuf[3] = 0xFF;
			}
		}
		break;
	default:
		break;
	}

	free(uncompressed_buffer);
	return NULL;
}