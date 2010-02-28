/***************************************************************************
                          guitexture.cpp -- Images manipulation
                             -------------------
    created              : Tue Aug 17 20:13:08 CEST 1999
    copyright            : (C) 1999 by Eric Espie
    email                : torcs@free.fr
    version              : $Id$
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/** @file   
    		Images manipulation tools.
		Load and store png images with easy interface.
    @author	<a href=mailto:torcs@free.fr>Eric Espie</a>
    @version	$Id$
    @ingroup	img		
*/

#ifdef WIN32
#include <windows.h>

#define XMD_H
#endif

#include "png.h"

#include "tgfclient.h"
#include <stdlib.h>
#include <stdio.h>


extern "C"
{
    #include <jpeglib.h>
}

#ifdef WIN32
#include <direct.h>
#endif
#include "GL/glu.h"



static char buf[1024];

#define PNG_BYTES_TO_CHECK 4

int getClosestPowerof2(int size)
{
	static const int nSizes = 7;
	static const int sizes[nSizes] = { 2, 4, 16, 128, 256, 512, 1024 };

	for (int i = 0; i < nSizes; i++)
	{
		if (size <= sizes[i])
			return sizes[i];
	}

	// Do not allow textures larger than this for memory usage and performance reasons
	return sizes[nSizes - 1];
}

void
GfScaleImagePowerof2(unsigned char *pSrcImg,int srcW,int srcH,GLenum format,GLuint &texId)
{
	int destH = 128;
	int destW = 128;

	destH = getClosestPowerof2(srcH);
	destW = getClosestPowerof2(srcW);

	if ( destH != srcH || destW != srcW)
	{
	
		unsigned char *texData = NULL;
		if (format == GL_RGB)
		{
			texData = new unsigned char[destW*destH*3];
		}
		else if(format == GL_RGBA)
		{
			texData = new unsigned char[destW*destH*4];

		}

		int r = gluScaleImage(format, srcW,srcH,GL_UNSIGNED_BYTE,(void*)pSrcImg,destW,destH,GL_UNSIGNED_BYTE,(void*)texData);
		if (r!=0)
			printf("Error trying to scale image\n");

		glGenTextures(1, &texId);
		glBindTexture(GL_TEXTURE_2D, texId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0,  GL_RGBA, destW, destH, 0, format, GL_UNSIGNED_BYTE, (GLvoid *)(texData));
		delete [] texData;
	}
	else
	{
		glGenTextures(1, &texId);
		glBindTexture(GL_TEXTURE_2D, texId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, format, destW, destH, 0, format, GL_UNSIGNED_BYTE, (GLvoid *)(pSrcImg));
	}
}

/** Load a PNG image from disk to a buffer in RGBA mode (if specified, enforce 2^N x 2^P size for the target buffer, to suit with low-end OpenGL hardwares/drivers poor texture support).
    @ingroup	img		
    @param	filename	name of the image to load
    @param	widthp		original width of the read image (left aligned in target buffer)
    @param	heightp		original height of the read image (top aligned in target buffer)
    @param	screen_gamma	gamma correction value
    @param	pow2_widthp	if not 0, pointer to 2^N width of the target image buffer
    @param	pow2_heightp	if not 0, pointer to 2^N height of the target image buffer
    @return	Pointer on the buffer containing the image
		<br>NULL Error
 */
unsigned char *
GfTexReadPng(const char *filename, int *widthp, int *heightp, float screen_gamma, int *pow2_widthp, int *pow2_heightp)
{
	unsigned char buf[PNG_BYTES_TO_CHECK];
	FILE *fp;
	png_structp png_ptr;
	png_infop info_ptr;
	png_uint_32 src_width, src_height;
	png_uint_32 tgt_width, tgt_height;
	int bit_depth, color_type, interlace_type;
	
	double gamma;
	png_bytep *row_pointers;
	unsigned char *image_ptr, *cur_ptr;
	png_uint_32 src_rowbytes, tgt_rowbytes;
	png_uint_32 i;
	
	if ((fp = fopen(filename, "rb")) == NULL) {
		GfTrace("Can't open file %s for reading\n", filename);
		return (unsigned char *)NULL;
	}
	
	if (fread(buf, 1, PNG_BYTES_TO_CHECK, fp) != PNG_BYTES_TO_CHECK) {
		GfTrace("Can't read file %s\n", filename);
		fclose(fp);
		return (unsigned char *)NULL;
	}
	
	if (png_sig_cmp(buf, (png_size_t)0, PNG_BYTES_TO_CHECK) != 0) {
		GfTrace("File %s not in png format\n", filename);
		fclose(fp);
		return (unsigned char *)NULL;
	}
	
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, (png_error_ptr)NULL, (png_error_ptr)NULL);
	if (png_ptr == NULL) {
		GfTrace("Img Failed to create read_struct\n");
		fclose(fp);
		return (unsigned char *)NULL;
	}
	
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		fclose(fp);
		png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
		return (unsigned char *)NULL;
	}
	
	if (setjmp(png_ptr->jmpbuf))
	{
		/* Free all of the memory associated with the png_ptr and info_ptr */
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
		fclose(fp);
		/* If we get here, we had a problem reading the file */
		return (unsigned char *)NULL;
	}
	
	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, PNG_BYTES_TO_CHECK);
	png_read_info(png_ptr, info_ptr);
	png_get_IHDR(png_ptr, info_ptr, &src_width, &src_height, &bit_depth, &color_type, &interlace_type, NULL, NULL);
	*widthp = (int)src_width;
	*heightp = (int)src_height;
	
	// Compute target 2^N x 2^P image buffer size if specified.
	// Note: This 2^N x 2^P stuff is needed by some low-end OpenGL hardware/drivers
	//       that don't support non  2^N x 2^P textures (or at extremely low frame rates).
	if (pow2_widthp && pow2_heightp) {
	    tgt_width = 2;
	    while(tgt_width < src_width) 
		tgt_width *= 2;
	    tgt_height = 2;
	    while(tgt_height < src_height) 
		tgt_height *= 2;
	    *pow2_widthp = (int)tgt_width;
	    *pow2_heightp = (int)tgt_height;
	} else {
	    tgt_width = (int)src_width;
	    tgt_height = (int)src_height;
	}

	if (bit_depth == 1 && color_type == PNG_COLOR_TYPE_GRAY) 
	    png_set_invert_mono(png_ptr);

	if (bit_depth == 16) {
		png_set_swap(png_ptr);
		png_set_strip_16(png_ptr);
	}
	
	if (bit_depth < 8) {
		png_set_packing(png_ptr);
	}
	
	if (color_type == PNG_COLOR_TYPE_PALETTE) {
		png_set_expand(png_ptr);
	}
	
	if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) {
		png_set_expand(png_ptr);
	}
	
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
		png_set_expand(png_ptr);
	}
	
	if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
		png_set_gray_to_rgb(png_ptr);
	}
	
	if (bit_depth == 8 && color_type == PNG_COLOR_TYPE_RGB) {
		png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
	}
	
	if (png_get_gAMA(png_ptr, info_ptr, &gamma)) {
		png_set_gamma(png_ptr, screen_gamma, gamma);
	} else {
		png_set_gamma(png_ptr, screen_gamma, 0.50);
	}
	
	png_read_update_info(png_ptr, info_ptr);
	src_rowbytes = png_get_rowbytes(png_ptr, info_ptr);
	tgt_rowbytes = src_rowbytes;
	if (pow2_widthp && pow2_heightp) 
	    tgt_rowbytes = tgt_width * src_rowbytes / src_width;
	
	// RGBA expected.
	if (src_rowbytes != (4 * src_width)) {
		GfTrace("%s bad byte count... %lu instead of %lu\n", filename, (unsigned long)src_rowbytes, (unsigned long)(4 * src_width));
		fclose(fp);
		png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
		return (unsigned char *)NULL;
	}
	
	row_pointers = (png_bytep*)malloc(tgt_height * sizeof(png_bytep));
	if (row_pointers == NULL) {
		fclose(fp);
		png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
		return (unsigned char *)NULL;
	}
	
	image_ptr = (unsigned char *)malloc(tgt_height * tgt_rowbytes);
	//memset(image_ptr, 0x00, tgt_height * tgt_rowbytes); // Real 0-padding not needed ...
	if (image_ptr == NULL) {
		fclose(fp);
		png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
		return (unsigned char *)NULL;
	}
	
	for (i = 0, cur_ptr = image_ptr + (tgt_height - 1) * tgt_rowbytes ; 
	     i < tgt_height; i++, cur_ptr -= tgt_rowbytes) {
		row_pointers[i] = cur_ptr;
	}
	
	png_read_image(png_ptr, row_pointers);

	png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
	free(row_pointers);
	
	fclose(fp);
	return image_ptr;
}

/** Write a buffer to a png image on disk.
    @ingroup	img
    @param	img		image data (RGB)
    @param	filename	filename of the png file
    @param	width		width of the image
    @param	height		height of the image
    @return	0 Ok
		<br>-1 Error
 */
int
GfTexWritePng(unsigned char *img, const char *filename, int width, int height)
{
	FILE *fp;
	png_structp	png_ptr;
	png_infop info_ptr;
	png_bytep *row_pointers;
	png_uint_32 rowbytes;
	int i;
	unsigned char *cur_ptr;

#define ReadGammaFromSettingsFile 0
#if (ReadGammaFromSettingsFile)
    void		*handle;
#endif
	float		screen_gamma;
	
	fp = fopen(filename, "wb");
	if (fp == NULL) {
		GfTrace("Can't open file %s for writing\n", filename);
		return -1;
	}
	
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, (png_error_ptr)NULL, (png_error_ptr)NULL);
	if (png_ptr == NULL) {
		return -1;
	}
	
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		return -1;
	}
	
	if (setjmp(png_ptr->jmpbuf)) {    
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(fp);
		return -1;
	}
	
	png_init_io(png_ptr, fp);
	png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB,
			PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
#if (ReadGammaFromSettingsFile)
    handle = GfParmReadFile(GFSCR_CONF_FILE, GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);
    screen_gamma = (float)GfParmGetNum(handle, GFSCR_SECT_PROP, GFSCR_ATT_GAMMA, (char*)NULL, 2.0);
    GfParmReleaseHandle(handle);
#else
	screen_gamma = 2.0;
#endif
	png_set_gAMA(png_ptr, info_ptr, screen_gamma);
	/* png_set_bgr(png_ptr);    TO INVERT THE COLORS !!!! */
	png_write_info(png_ptr, info_ptr);
	png_write_flush(png_ptr);
	
	rowbytes = width * 3;
	row_pointers = (png_bytep*)malloc(height * sizeof(png_bytep));
	
	if (row_pointers == NULL) {
		fclose(fp);
		png_destroy_write_struct(&png_ptr, &info_ptr);
		return -1;
	}
	
	for (i = 0, cur_ptr = img + (height - 1) * rowbytes ; i < height; i++, cur_ptr -= rowbytes) {
		row_pointers[i] = cur_ptr;
	}
	
	png_write_image(png_ptr, row_pointers);
	png_write_end(png_ptr, (png_infop)NULL);
	png_destroy_write_struct(&png_ptr, &info_ptr);
	fclose(fp);
	free(row_pointers);
	return 0;
}

/** Free the texture
    @ingroup	img
    @param	tex	texture to free
    @return	none
*/
void
GfTexFreeTex(GLuint tex)
{
	if (tex != 0) {
		glDeleteTextures(1, &tex);
	}
}

/** Read a png image into a texture.
    @ingroup	img
    @param	filename	file name of the image
    @return	None.
 */
GLuint
GfTexReadTex(const char *filename)
{
	int w, h;
	return GfTexReadTex(filename, w, h);
}

GLuint
GfTexReadTex(const char *filename, int &width, int &height)
{
	void *handle;
	float screen_gamma;
	GLbyte *tex;
	GLuint retTex;

	sprintf(buf, "%s%s", GetLocalDir(), GFSCR_CONF_FILE);
	handle = GfParmReadFile(buf, GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);
	screen_gamma = (float)GfParmGetNum(handle, GFSCR_SECT_PROP, GFSCR_ATT_GAMMA, (char*)NULL, 2.0);
	tex = (GLbyte*)GfTexReadPng(filename, &width, &height, screen_gamma, 0, 0);

	if (!tex) {
		GfParmReleaseHandle(handle);
		return 0;
	}

	glGenTextures(1, &retTex);
	glBindTexture(GL_TEXTURE_2D, retTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid *)(tex));

	free(tex);

	GfParmReleaseHandle(handle);
	return retTex;
}

/** Load an JPEG image from disk to a buffer in RGBA mode (if specified, enforce 2^N x 2^P size for the target buffer, to suit with low-end OpenGL hardwares/drivers poor texture support).
    @ingroup	img		
    @param	filename	name of the image to load
    @param	widthp		original width of the read image (left aligned in target buffer)
    @param	heightp		original height of the read image (top aligned in target buffer)
    @param	screen_gamma	gamma correction value
    @param	pow2_widthp	if not 0, pointer to 2^N width of the target image buffer
                                WARNING: Not yet implemented ; = *widthp
    @param	pow2_heightp	if not 0, pointer to 2^N height of the target image buffer
                                WARNING: Not yet implemented ; = *heightp
    @return	Pointer on the buffer containing the image
		<br>NULL Error
 */
struct my_error_mgr {
  struct jpeg_error_mgr pub;	/* "public" fields */

  jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

/*
 * Here's the routine that will replace the standard error_exit method:
 */

METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  my_error_ptr myerr = (my_error_ptr) cinfo->err;

  /* Always display the message. */
  /* We could postpone this until after returning, if we chose. */
  (*cinfo->err->output_message) (cinfo);

  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);
}

unsigned char *
GfTexReadJpeg(const char *filename, int *widthp, int *heightp, float screen_gamma, 
	      int *pow2_widthp, int *pow2_heightp)
{
  unsigned char *pBuffer = NULL;
  struct jpeg_decompress_struct cinfo;

  struct my_error_mgr jerr;

  FILE * infile;		/* source file */
  JSAMPARRAY buffer;		/* Output row buffer */
  int row_stride;		/* physical row width in output buffer */

  if ((infile = fopen(filename, "rb")) == NULL) 
  {
    fprintf(stderr, "can't open %s\n", filename);
    return 0;
  }

  cinfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = my_error_exit;

  if (setjmp(jerr.setjmp_buffer)) 
  {
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
    return 0;
  }

  /* Now we can initialize the JPEG decompression object. */
  jpeg_create_decompress(&cinfo);

  jpeg_stdio_src(&cinfo, infile);

  (void) jpeg_read_header(&cinfo, TRUE);

  (void) jpeg_start_decompress(&cinfo);
  row_stride = cinfo.output_width * cinfo.output_components;
  buffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

  pBuffer = (unsigned char*)malloc(cinfo.output_height*cinfo.output_width*3);
  memset(pBuffer,0,cinfo.output_height*cinfo.output_width*3);

  *heightp = cinfo.output_height;
  *widthp = cinfo.output_width;

  unsigned char *pLine = pBuffer +row_stride*(cinfo.output_height-1);

  while (cinfo.output_scanline < cinfo.output_height) 
  {
    (void) jpeg_read_scanlines(&cinfo, buffer, 1);
	memcpy(pLine,buffer[0],row_stride);
	pLine-=row_stride;
  }

  (void) jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);
  fclose(infile);

  // WARNING: For the moment, no real power of 2 management ; may fail on low end hardwares.
  *pow2_widthp = *widthp;
  *pow2_heightp = *heightp;

  return pBuffer;
}

