/****************************************************************************
 *            grloaddds.c
 *
 * Author: 2009  Xavier Bertaux <bertauxx@yahoo.fr>
 * Based on the code of Daniel Jungmann <dsj@gmx.net>
 * for the opensource game Eternal Land
 * 
 ****************************************************************************/

#include "grloaddds.h"

void unpack_dxt_color(DXTColorBlock *block, Uint8 *values, Uint32 dxt1)
{
	float colors[4][4];
	Uint32 i, j, index;

	colors[0][0] = (block->m_colors[0] & 0xF800) >> 11;
	colors[0][1] = (block->m_colors[0] & 0x07E0) >> 5;
	colors[0][2] = block->m_colors[0] & 0x001F;
	colors[0][3] = 255.0f;

	colors[0][0] *= 255.0f / 31.0f;
	colors[0][1] *= 255.0f / 63.0f;
	colors[0][2] *= 255.0f / 31.0f;

	colors[1][0] = (block->m_colors[1] & 0xF800) >> 11;
	colors[1][1] = (block->m_colors[1] & 0x07E0) >> 5;
	colors[1][2] = block->m_colors[1] & 0x001F;
	colors[1][3] = 255.0f;

	colors[1][0] *= 255.0f / 31.0f;
	colors[1][1] *= 255.0f / 63.0f;
	colors[1][2] *= 255.0f / 31.0f;

	if (dxt1 && (block->m_colors[0] <= block->m_colors[1]))
	{
		// 1-bit alpha
		// one intermediate colour, half way between the other two
		colors[2][0] = (colors[0][0] + colors[1][0]) / 2.0f;
		colors[2][1] = (colors[0][1] + colors[1][1]) / 2.0f;
		colors[2][2] = (colors[0][2] + colors[1][2]) / 2.0f;
		colors[2][3] = (colors[0][3] + colors[1][3]) / 2.0f;
		// transparent colour
		colors[3][0] = 0.0f;
		colors[3][1] = 0.0f;
		colors[3][2] = 0.0f;
		colors[3][3] = 0.0f;
	}
	else
	{
		// first interpolated colour, 1/3 of the way along
		colors[2][0] = (2.0f * colors[0][0] + colors[1][0]) / 3.0f;
		colors[2][1] = (2.0f * colors[0][1] + colors[1][1]) / 3.0f;
		colors[2][2] = (2.0f * colors[0][2] + colors[1][2]) / 3.0f;
		colors[2][3] = (2.0f * colors[0][3] + colors[1][3]) / 3.0f;
		// second interpolated colour, 2/3 of the way along
		colors[3][0] = (colors[0][0] + 2.0f * colors[1][0]) / 3.0f;
		colors[3][1] = (colors[0][1] + 2.0f * colors[1][1]) / 3.0f;
		colors[3][2] = (colors[0][2] + 2.0f * colors[1][2]) / 3.0f;
		colors[3][3] = (colors[0][3] + 2.0f * colors[1][3]) / 3.0f;
	}

	// Process 4x4 block of texels
	for (i = 0; i < 4; ++i)
	{
		for (j = 0; j < 4; ++j)
		{
			// LSB come first
			index = block->m_indices[i] >> (j * 2) & 0x3;

			values[((i * 4) + j) * 4 + 0] = colors[index][0];
			values[((i * 4) + j) * 4 + 1] = colors[index][1];
			values[((i * 4) + j) * 4 + 2] = colors[index][2];

			if (dxt1)
			{
				// Overwrite entire colour
				values[((i * 4) + j) * 4 + 3] = colors[index][3];
			}
		}
	}
}

void unpack_dxt_explicit_alpha(DXTExplicitAlphaBlock *block, Uint8 *values)
{
	float value;
	Uint32 i, j, index;

	index = 0;

	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 4; j++)
		{
			value = block->m_alphas[i] >> (j * 4) & 0xF;
			values[index] = value * 17.0f;	// = (value * 255.0f) / 15.0f;
			index++;
		}
	}
}

void unpack_dxt_interpolated_alpha(DXTInterpolatedAlphaBlock *block, Uint8 *values)
{
	float alphas[8];
	float scale, f0, f1;
	Uint32 i, index, idx0, idx1;

	alphas[0] = block->m_alphas[0];
	alphas[1] = block->m_alphas[1];

	if (block->m_alphas[0] > block->m_alphas[1])
	{
		scale = 1.0f / 7.0f;

		for (i = 0; i < 6; i++)
		{
			f0 = (6 - i) * scale;
			f1 = (i + 1) * scale;
			alphas[i + 2] = (f0 * block->m_alphas[0]) + (f1 * block->m_alphas[1]);
		}
	}
	else
	{
		// 4 interpolated alphas, plus zero and one
		// full range including extremes at [0] and [5]
		// we want to fill in [1] through [4] at weights ranging
		// from 1/5 to 4/5
		scale = 1.0f / 5.0f;

		for (i = 0; i < 4; i++)
		{
			f0 = (4 - i) * scale;
			f1 = (i + 1) * scale;
			alphas[i + 2] = (f0 * block->m_alphas[0]) + (f1 * block->m_alphas[1]);
		}

		alphas[6] = 0.0f;
		alphas[7] = 1.0f;
	}

	for (i = 0; i < 16; i++)
	{
		idx0 = (i * 3) / 8;
		idx1 = (i * 3) % 8;
		index = (block->m_indices[idx0] >> idx1) & 0x07;

		if (idx1 > 5)
		{
			index |= (block->m_indices[idx0 + 1] << (8 - idx1)) & 0x07;
		}

		values[i] = alphas[index];
	}
}

void unpack_dxt1(DXTColorBlock *block, Uint8 *values)
{
	unpack_dxt_color(block, values, 1);
}

void unpack_dxt3(DXTExplicitAlphaBlock *alpha_block, DXTColorBlock *color_block, Uint8 *values)
{
	Uint8 alpha_values[16];
	Uint32 i;

	unpack_dxt_color(color_block, values, 0);
	unpack_dxt_explicit_alpha(alpha_block, alpha_values);

	for (i = 0; i < 16; i++)
	{
		values[i * 4 + 3] = alpha_values[i];
	}
}

void unpack_dxt5(DXTInterpolatedAlphaBlock *alpha_block, DXTColorBlock *color_block, Uint8 *values)
{
	Uint8 alpha_values[16];
	Uint32 i;

	unpack_dxt_color(color_block, values, 0);
	unpack_dxt_interpolated_alpha(alpha_block, alpha_values);

	for (i = 0; i < 16; i++)
	{
		values[i * 4 + 3] = alpha_values[i];
	}
}

void unpack_ati1(DXTInterpolatedAlphaBlock *block, Uint8 *values)
{
	Uint8 alpha_values[16];
	Uint32 i;

	unpack_dxt_interpolated_alpha(block, alpha_values);

	for (i = 0; i < 16; i++)
	{
		values[i * 4 + 0] = alpha_values[i];
		values[i * 4 + 1] = alpha_values[i];
		values[i * 4 + 2] = alpha_values[i];
		values[i * 4 + 3] = alpha_values[i];
	}
}

void unpack_ati2(DXTInterpolatedAlphaBlock *first_block, DXTInterpolatedAlphaBlock *second_block,
	Uint8 *values)
{
	Uint8 first_values[16], second_values[16];
	Uint32 i;

	unpack_dxt_interpolated_alpha(first_block, first_values);
	unpack_dxt_interpolated_alpha(second_block, second_values);

	for (i = 0; i < 16; i++)
	{
		values[i * 4 + 0] = first_values[i];
		values[i * 4 + 1] = first_values[i];
		values[i * 4 + 2] = first_values[i];
		values[i * 4 + 3] = second_values[i];
	}
}

static Uint32 popcount(const Uint32 x)
{
	Uint32 r;

	r = x - ((x >> 1) & 033333333333) - ((x >> 2) & 011111111111);

	return ((r + (r >> 3)) & 030707070707) % 63;
}

Uint32 check_dds(const Uint8 *ID)
{
	return (ID[0] == 'D') && (ID[1] == 'D') && (ID[2] == 'S') && (ID[3] == ' ');
}

static Uint32 validate_header(DdsHeader *header, const char* file)
{
	Uint32 bit_count;

	if (header->m_size != DDS_HEADER_SIZE)
	{
		printf("File '%s' is invalid. Size of header is"
			" %d bytes, but must be %d bytes for valid DDS files.",
			file, header->m_size, DDS_HEADER_SIZE);
		return 0;
	}

	if (header->m_pixel_format.m_size != DDS_PIXEL_FORMAT_SIZE)
	{
		printf("File '%s' is invalid. Size of pixe format header is %d bytes, but must"
			" be %3% bytes for valid DDS files.", file,
			header->m_pixel_format.m_size, DDS_PIXEL_FORMAT_SIZE);
		return 0;
	}

	if ((header->m_flags & DDSD_MIN_FLAGS) != DDSD_MIN_FLAGS)
	{
		printf("File '%s' is invalid. At least the "
			"DDSD_CAPS, DDSD_PIXELFORMAT, DDSD_WIDTH and DDSD_HEIGHT flags"
			" must be set for a valid DDS file.", file);
		return 0;
	}

	if ((header->m_caps.m_caps1 & DDSCAPS_TEXTURE) != DDSCAPS_TEXTURE)
	{
		printf("File '%' is invalid. At least "
			"DDSCAPS_TEXTURE cap must be set for a valid DDS file.",
			file);
		return 0;
	}

	if (((header->m_caps.m_caps2 & DDSCAPS2_CUBEMAP) == DDSCAPS2_CUBEMAP) &&
		((header->m_caps.m_caps2 & DDSCAPS2_CUBEMAP_ALL_FACES) == 0))
	{
		printf("File '%s' is invalid. At least one cube"
			" map face must be set for a valid cube map DDS file.", file);
		return 0;
	}

	if (((header->m_caps.m_caps2 & DDSCAPS2_CUBEMAP) == DDSCAPS2_CUBEMAP) &&
		((header->m_caps.m_caps2 & DDSCAPS2_VOLUME) == DDSCAPS2_VOLUME))
	{
		printf("File '%s' is invalid. A valid DDS file "
			"can only be a cube map or a volume, not both.", file);
		return 0;
	}

	if (((header->m_flags & DDSD_DEPTH) == DDSD_DEPTH) &&
		((header->m_caps.m_caps2 & DDSCAPS2_VOLUME) != DDSCAPS2_VOLUME))
	{
		printf("File '%s' is invalid. Only volmue "
			"textures can have a detph value in a valid DDS file.", file);
		return 0;
	}

	if (((header->m_caps.m_caps2 & DDSCAPS2_CUBEMAP) == DDSCAPS2_CUBEMAP) &&
		((header->m_caps.m_caps1 & DDSCAPS_COMPLEX) != DDSCAPS_COMPLEX))
	{
		printf("File '%s' is invalid. DDSCAPS_COMPLEX cap "
			"should be set for a valid cube map DDS file.", file);
	}

	if (((header->m_caps.m_caps2 & DDSCAPS2_VOLUME) == DDSCAPS2_VOLUME) &&
		((header->m_caps.m_caps1 & DDSCAPS_COMPLEX) != DDSCAPS_COMPLEX))
	{
		printf("File '%s' is invalid. DDSCAPS_COMPLEX cap "
			"should be set for a valid volume DDS file.", file);
	}

	if (((header->m_caps.m_caps1 & DDSCAPS_MIPMAP) == DDSCAPS_MIPMAP) &&
		((header->m_caps.m_caps1 & DDSCAPS_COMPLEX) != DDSCAPS_COMPLEX))
	{
		printf("File '%s' is invalid. DDSCAPS_COMPLEX cap "
			"should be set for a valid DDS file with mipmaps.", file);
	}

	if (((header->m_caps.m_caps1 & DDSCAPS_COMPLEX) == DDSCAPS_COMPLEX) &&
		((header->m_caps.m_caps1 & DDSCAPS_MIPMAP) != DDSCAPS_MIPMAP) &&
		((header->m_caps.m_caps2 & DDSCAPS2_VOLUME) != DDSCAPS2_VOLUME)
		&& ((header->m_caps.m_caps2 & DDSCAPS2_CUBEMAP) != DDSCAPS2_CUBEMAP))
	{
		printf("File '%s' is invalid. DDSCAPS_COMPLEX cap "
			"should be set only if the DDS file is a cube map, a volume and/or"
			" has mipmaps.", file);
	}

	if (((header->m_pixel_format.m_flags & DDPF_FOURCC) == DDPF_FOURCC) &&
		((header->m_pixel_format.m_flags & DDPF_RGB) == DDPF_RGB))
	{
		printf("File '%s' is invalid. A valid DDS file "
			"must set either DDPF_FORCC or DDPF_RGB as pixel format flags.",
			file);
		return 0;
	}

	if ((header->m_pixel_format.m_flags & DDPF_LUMINANCE) == DDPF_LUMINANCE)
	{
		printf("File '%s' is invalid. A valid DDS file must "
			"not set DDPF_LUMINANCE as pixel format flags.", file);
	}

	if (((header->m_pixel_format.m_flags & DDPF_FOURCC) != DDPF_FOURCC) &&
		((header->m_pixel_format.m_flags & DDPF_RGB) != DDPF_RGB))
	{
		if ((header->m_pixel_format.m_flags & DDPF_LUMINANCE) == DDPF_LUMINANCE)
		{
			header->m_pixel_format.m_green_mask = 0;
			header->m_pixel_format.m_blue_mask = 0;
		}
		else
		{
			printf("File '%s' is invalid. A valid "
				"DDS file must set either DDPF_FORCC or DDPF_RGB as pixe"
				" format flags.", file);
			return 0;
		}
	}

	if ((header->m_flags & DDSD_DEPTH) != DDSD_DEPTH)
	{
		header->m_depth = 1;
	}
	else
	{
		if (header->m_depth == 0)
		{
			printf("File '%s' is invalid. Volmue "
				"textures must have a depth greather than zero.", file);
			return 0;
		}
	}

	if ((header->m_pixel_format.m_flags & DDPF_ALPHAPIXELS) != DDPF_ALPHAPIXELS)
	{
		if (header->m_pixel_format.m_alpha_mask != 0)
		{
			printf("File '%s' is invalid. Non alpha pixe"
				" formats must have a zero alpha mask to be a valid DDS "
				"files", file);
			header->m_pixel_format.m_alpha_mask = 0;
		}
	}

	bit_count = popcount(header->m_pixel_format.m_red_mask |
		header->m_pixel_format.m_blue_mask |
		header->m_pixel_format.m_green_mask |
		header->m_pixel_format.m_alpha_mask);

	if (((header->m_pixel_format.m_flags & DDPF_RGB) == DDPF_RGB) &&
		(header->m_pixel_format.m_bit_count != bit_count))
	{
		header->m_pixel_format.m_alpha_mask = 0xFFFFFFFF;
		header->m_pixel_format.m_alpha_mask ^= header->m_pixel_format.m_red_mask;
		header->m_pixel_format.m_alpha_mask ^= header->m_pixel_format.m_blue_mask;
		header->m_pixel_format.m_alpha_mask ^= header->m_pixel_format.m_green_mask;
	}

	return 1;
}

static Uint32 init_dds_image(buf, DdsHeader *header, const char* file)
{
	Uint8 magic[4];

	el_read(file, sizeof(magic), magic);

	if (!check_dds(magic))
	{
		printf("File '%s' is invalid. Wrong magic number for a valid DDS.", file);
		return 0;
	}

	el_read(file, sizeof(DdsHeader), header);

	header->m_size = SDL_SwapLE32(header->m_size);
	header->m_flags = SDL_SwapLE32(header->m_flags);
	header->m_height = SDL_SwapLE32(header->m_height);
	header->m_width = SDL_SwapLE32(header->m_width);
	header->m_size_or_pitch = SDL_SwapLE32(header->m_size_or_pitch);
	header->m_depth = SDL_SwapLE32(header->m_depth);
	header->m_mipmap_count = SDL_SwapLE32(header->m_mipmap_count);

	header->m_reserved1[0] = SDL_SwapLE32(header->m_reserved1[0]);
	header->m_reserved1[1] = SDL_SwapLE32(header->m_reserved1[1]);
	header->m_reserved1[2] = SDL_SwapLE32(header->m_reserved1[2]);
	header->m_reserved1[3] = SDL_SwapLE32(header->m_reserved1[3]);
	header->m_reserved1[4] = SDL_SwapLE32(header->m_reserved1[4]);
	header->m_reserved1[5] = SDL_SwapLE32(header->m_reserved1[5]);
	header->m_reserved1[6] = SDL_SwapLE32(header->m_reserved1[6]);
	header->m_reserved1[7] = SDL_SwapLE32(header->m_reserved1[7]);
	header->m_reserved1[8] = SDL_SwapLE32(header->m_reserved1[8]);
	header->m_reserved1[9] = SDL_SwapLE32(header->m_reserved1[9]);
	header->m_reserved1[10] = SDL_SwapLE32(header->m_reserved1[10]);

	header->m_pixel_format.m_size = SDL_SwapLE32(header->m_pixel_format.m_size);
	header->m_pixel_format.m_flags = SDL_SwapLE32(header->m_pixel_format.m_flags);
	header->m_pixel_format.m_fourcc = SDL_SwapLE32(header->m_pixel_format.m_fourcc);
	header->m_pixel_format.m_bit_count = SDL_SwapLE32(header->m_pixel_format.m_bit_count);
	header->m_pixel_format.m_red_mask = SDL_SwapLE32(header->m_pixel_format.m_red_mask);
	header->m_pixel_format.m_green_mask = SDL_SwapLE32(header->m_pixel_format.m_green_mask);
	header->m_pixel_format.m_blue_mask = SDL_SwapLE32(header->m_pixel_format.m_blue_mask);
	header->m_pixel_format.m_alpha_mask = SDL_SwapLE32(header->m_pixel_format.m_alpha_mask);

	header->m_caps.m_caps1 = SDL_SwapLE32(header->m_caps.m_caps1);
	header->m_caps.m_caps2 = SDL_SwapLE32(header->m_caps.m_caps2);
	header->m_caps.m_caps3 = SDL_SwapLE32(header->m_caps.m_caps3);
	header->m_caps.m_caps4 = SDL_SwapLE32(header->m_caps.m_caps4);

	header->m_reserved2 = SDL_SwapLE32(header->m_reserved2);

	return validate_header(header, file);
}

static void read_colors_block(file, DXTColorBlock *colors)
{
	Uint32 i;

	el_read(file, sizeof(DXTColorBlock), colors);

	for (i = 0; i < 2; i++)
	{
		colors->m_colors[i] = SDL_SwapLE16(colors->m_colors[i]);
	}
}

static void read_interpolated_alphas_block(file, DXTInterpolatedAlphaBlock *alphas)
{
	el_read(file, sizeof(DXTInterpolatedAlphaBlock), alphas);
}

static void read_explicit_alphas_block(buf, DXTExplicitAlphaBlock *alphas)
{
	Uint32 i;

	el_read(file, sizeof(DXTExplicitAlphaBlock), alphas);

	for (i = 0; i < 4; i++)
	{
		alphas->m_alphas[i] = SDL_SwapLE16(alphas->m_alphas[i]);
	}
}

static void read_and_uncompress_dxt1_block(buf, Uint8 *values)
{
	DXTColorBlock colors;

	read_colors_block(file, &colors);

	unpack_dxt1(&colors, values);
}

static void read_and_uncompress_dxt3_block(buf, Uint8 *values)
{
	DXTExplicitAlphaBlock alphas;
	DXTColorBlock colors;

	read_explicit_alphas_block(file, &alphas);
	read_colors_block(file, &colors);

	unpack_dxt3(&alphas, &colors, values);
}

static void read_and_uncompress_dxt5_block(buf, Uint8 *values)
{
	DXTInterpolatedAlphaBlock alphas;
	DXTColorBlock colors;

	read_interpolated_alphas_block(file, &alphas);
	read_colors_block(file, &colors);

	unpack_dxt5(&alphas, &colors, values);
}

static void read_and_uncompress_ati1_block(buf, Uint8 *values)
{
	DXTInterpolatedAlphaBlock alphas;

	read_interpolated_alphas_block(file, &alphas);

	unpack_ati1(&alphas, values);
}

static void read_and_uncompress_ati2_block(buf, Uint8 *values)
{
	DXTInterpolatedAlphaBlock first_block;
	DXTInterpolatedAlphaBlock second_block;

	read_interpolated_alphas_block(file, &first_block);
	read_interpolated_alphas_block(file, &second_block);

	unpack_ati2(&first_block, &second_block, values);
}

static void uncompress_block(el_file_ptr file, Uint32 format, Uint32 x, Uint32 width,
	Uint32 dst_pitch, Uint32 dst_pitch_minus_4, Uint32 *idx, Uint8 *dst)
{
	Uint8 values[64];
	Uint32 i, j, index;

	switch (format)
	{
		case DDSFMT_DXT1:
			read_and_uncompress_dxt1_block(file, values);
			break;
		case DDSFMT_DXT2:
		case DDSFMT_DXT3:
			read_and_uncompress_dxt3_block(file, values);
			break;
		case DDSFMT_DXT4:
		case DDSFMT_DXT5:
			read_and_uncompress_dxt5_block(file, values);
			break;
		case DDSFMT_ATI1:
			read_and_uncompress_ati1_block(file, values);
			break;
		case DDSFMT_ATI2:
			read_and_uncompress_ati2_block(file, values);
			break;
	}

	index = *idx;

	// write 4x4 block to uncompressed version
	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 4; j++)
		{
			dst[index + 0] = values[(i * 4 + j) * 4 + 0];
			dst[index + 1] = values[(i * 4 + j) * 4 + 1];
			dst[index + 2] = values[(i * 4 + j) * 4 + 2];
			dst[index + 3] = values[(i * 4 + j) * 4 + 3];
			index += 4;
		}
		// advance to next row
		index += dst_pitch_minus_4;
	}
	// next block. Our dest pointer is 4 lines down
	// from where it started
	if ((x + 1) == (width / 4))
	{
		// Jump back to the start of the line
		index -= dst_pitch_minus_4;
	}
	else
	{
		// Jump back up 4 rows and 4 pixels to the
		// right to be at the next block to the right
		index += (4 - dst_pitch) * 4;
	}

	*idx = index;
}

static void* uncompress(el_file_ptr file, DdsHeader *header, const char* file_name)
{
	Uint32 width, height, size, format;
	Uint32 x, y;
	Uint32 dst_pitch, dst_pitch_minus_4, index;
	Uint8 *dst;

	if ((header->m_height % 4) != 0)
	{
		printf("Can`t uncompressed DDS file %s because height is %d and not a "
			"multible of four.", file_name, header->m_height);
		return 0;
	}

	if ((header->m_width % 4) != 0)
	{
		printf("Can`t uncompressed DDS file %s because width is %d and not a "
			"multible of four.", file_name, header->m_width);
		return 0;
	}

	format = header->m_pixel_format.m_fourcc;

	if ((format != DDSFMT_DXT1) && (format != DDSFMT_DXT2) && (format != DDSFMT_DXT3) &&
		(format != DDSFMT_DXT4) && (format != DDSFMT_DXT5) && (format != DDSFMT_ATI1) &&
		(format != DDSFMT_ATI2))
	{
		return 0;
	}

	index = 0;

	width = header->m_width;
	height = header->m_height;

	size = width * height * 4;

	dst = malloc(size * sizeof(GLubyte));

	dst_pitch = width * 4;
	dst_pitch_minus_4 = dst_pitch - 4 * 4;

	// 4x4 blocks in x/y
	for (y = 0; y < (height / 4); y++)
	{
		for (x = 0; x < (width / 4); x++)
		{
			uncompress_block(file, format, x, width, dst_pitch, dst_pitch_minus_4,
				&index, dst);
		}
	}

	return dst;
}

void* load_dds(buf, const char* file, Uint32 *width, Uint32 *height)
{
	DdsHeader header;

	if (file == 0)
	{
		return 0;
	}

	if (init_dds_image(file, &header, file_name) != 0)
	{
		*width = header.m_width;
		*height = header.m_height;

		return uncompress(file, &header, file_name);
	}
	else
	{
		return 0;
	}
}

