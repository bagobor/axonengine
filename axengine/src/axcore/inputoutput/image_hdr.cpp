/*
Copyright (c) 2009 AxonEngine Team

This file is part of the AxonEngine project, and may only be used, modified, 
and distributed under the terms of the AxonEngine project license, license.txt.  
By continuing to use, modify, or distribute this file you indicate that you have
read the license and understand and accept it fully.
*/

#include "../private.h"

AX_BEGIN_NAMESPACE

typedef struct {
	int valid;            /* indicate which fields are valid */
	char programtype[16];	/* listed at beginning of file to identify it 
							 * after "#?".  defaults to "RGBE" */ 
	float gamma;			/* image has already been gamma corrected with 
							 * given gamma.  defaults to 1.0 (no correction) */
	float exposure;			/* a value of 1.0 in an image corresponds to
							 * <exposure> watts/steradian/m^2. 
							 * defaults to 1.0 */
} rgbe_header_info;

/* flags indicating which fields in an rgbe_header_info are valid */
#define RGBE_VALID_PROGRAMTYPE 0x01
#define RGBE_VALID_GAMMA       0x02
#define RGBE_VALID_EXPOSURE    0x04

/* return codes for rgbe routines */
#define RGBE_RETURN_SUCCESS 0
#define RGBE_RETURN_FAILURE -1

/* read or write headers */
/* you may set rgbe_header_info to null if you want to */
int RGBE_WriteHeader(File *fp, int width, int height, rgbe_header_info *info);
int RGBE_ReadHeader(File *fp, int *width, int *height, rgbe_header_info *info);

/* read or write pixels */
/* can read or write pixels in chunks of any size including single pixels*/
int RGBE_WritePixels(File *fp, float *data, int numpixels);
int RGBE_ReadPixels(File *fp, float *data, int numpixels);

/* read or write run length encoded files */
/* must be called to read or write whole scanlines */
int RGBE_WritePixels_RLE(File *fp, float *data, int scanline_width, int num_scanlines);
int RGBE_ReadPixels_RLE(File *fp, float *data, int scanline_width, int num_scanlines);

int RGBE_ReadPixels_Raw_RLE(File *fp, byte_t *data, int scanline_width, int num_scanlines);


inline void float2rgbe(byte_t rgbe[4], float red, float green, float blue);
inline void rgbe2float(float *red, float *green, float *blue, byte_t rgbe[4]);

/* This file contains code to read and write four byte rgbe file format
	developed by Greg Ward.  It handles the conversions between rgbe and
	pixels consisting of floats.  The data is assumed to be an array of floats.
	By default there are three floats per pixel in the order red, green, blue.
	(RGBE_DATA_??? values control this.)  Only the mimimal header reading and 
	writing is implemented.  Each routine does error checking and will return
	a status value as defined below.  This code is intended as a skeleton so
	feel free to modify it to suit your needs.

	(Place notice here if you modified the code.)
	posted to http://www.graphics.cornell.edu/~bjw/
	written by Bruce Walter  (bjw@graphics.cornell.edu)  5/26/95
	based on code written by Greg Ward
*/

/* offsets to red, green, and blue components in a data (float) pixel */
#define RGBE_DATA_RED 0
#define RGBE_DATA_GREEN 1
#define RGBE_DATA_BLUE 2
#define RGBE_DATA_ALPHA 3

/* number of floats per pixel */
#define RGBE_DATA_SIZE 4

enum rgbe_error_codes {
	rgbe_read_error,
	rgbe_write_error,
	rgbe_format_error,
	rgbe_memory_error,
};

/* default error routine.  change this to change error handling */
static int rgbe_error(int rgbe_error_code, char *msg) {
	switch (rgbe_error_code) {
	case rgbe_read_error:
		perror("RGBE read error");
		break;
	case rgbe_write_error:
		perror("RGBE write error");
		break;
	case rgbe_format_error:
		fprintf(stderr, "RGBE bad file format: %s\n",msg);
		break;
	default:
	case rgbe_memory_error:
		fprintf(stderr, "RGBE error: %s\n",msg);
	}
	return RGBE_RETURN_FAILURE;
}

/* standard conversion from float pixels to rgbe pixels */
/* note: you can remove the "inline"s if your compiler complains about it */
inline void 
float2rgbe(byte_t rgbe[4], float red, float green, float blue) {
	float v;
	int e;

	v = red;
	if (green > v) v = green;
	if (blue > v) v = blue;
	if (v < 1e-32) {
		rgbe[0] = rgbe[1] = rgbe[2] = rgbe[3] = 0;
	} else {
		v = frexp(v,&e) * 256.0/v;
		rgbe[0] = (byte_t) (red * v);
		rgbe[1] = (byte_t) (green * v);
		rgbe[2] = (byte_t) (blue * v);
		rgbe[3] = (byte_t) (e + 128);
	}
}

/* standard conversion from rgbe to float pixels */
/* note: Ward uses ldexp(col+0.5,exp-(128+8)).  However we wanted pixels */
/*       in the range [0,1] to map back into the range [0,1].            */
inline void 
rgbe2float(float *red, float *green, float *blue, byte_t rgbe[4]) {
	float f;
	static byte_t e_min = 255;

	if (rgbe[3]) {   /*nonzero pixel*/
		f = ldexp(1.0,rgbe[3]-(int)(128+8));

		// HACK
		if (f > 255.f) f = 255.f;
		if (rgbe[3] < e_min)
			e_min = rgbe[3];

		*red = rgbe[0] * f;
		*green = rgbe[1] * f;
		*blue = rgbe[2] * f;
	} else
		*red = *green = *blue = 0.0;
}

/* default minimal header. modify if you want more information in header */
int
RGBE_WriteHeader(File *fp, int width, int height, rgbe_header_info *info) {
	char *programtype = "RGBE";

	if (info && (info->valid & RGBE_VALID_PROGRAMTYPE))
		programtype = info->programtype;
	if (fp->printf("#?%s\n", programtype) < 0)
		return rgbe_error(rgbe_write_error,NULL);
	/* The #? is to identify file type, the programtype is optional. */
	if (info && (info->valid & RGBE_VALID_GAMMA)) {
		if (fp->printf("GAMMA=%g\n",info->gamma) < 0)
			return rgbe_error(rgbe_write_error,NULL);
	}
	if (info && (info->valid & RGBE_VALID_EXPOSURE)) {
		if (fp->printf("EXPOSURE=%g\n", info->exposure) < 0)
			return rgbe_error(rgbe_write_error,NULL);
	}
	if (fp->printf("FORMAT=32-bit_rle_rgbe\n\n") < 0)
		return rgbe_error(rgbe_write_error,NULL);
	if (fp->printf("-Y %d +X %d\n", height, width) < 0)
		return rgbe_error(rgbe_write_error,NULL);
	return RGBE_RETURN_SUCCESS;
}

/* minimal header reading.  modify if you want to parse more information */
int
RGBE_ReadHeader(File *fp, int *width, int *height, rgbe_header_info *info) {
	char buf[128];
	int found_format;
	float tempf;
	size_t i;

	found_format = 0;
	if (info) {
		info->valid = 0;
		info->programtype[0] = 0;
		info->gamma = info->exposure = 1.0;
	}
	if (fp->readLine(buf, sizeof(buf)/sizeof(buf[0])) == NULL)
		return rgbe_error(rgbe_read_error, NULL);

	if ((buf[0] != '#')||(buf[1] != '?')) {
		/* if you want to require the magic token then uncomment the next line */
		/*return rgbe_error(rgbe_format_error,"bad initial token"); */
	} else if (info) {
		info->valid |= RGBE_VALID_PROGRAMTYPE;
		for (i=0; i<sizeof(info->programtype)-1; i++) {
			if ((buf[i+2] == 0) || isspace(buf[i+2]))
				break;
			info->programtype[i] = buf[i+2];
		}
		info->programtype[i] = 0;
		if (fp->readLine(buf, sizeof(buf)/sizeof(buf[0])) == 0)
			return rgbe_error(rgbe_read_error,NULL);
	}

	for (;;) {
		if ((buf[0] == 0) || (buf[0] == '\n'))
			return rgbe_error(rgbe_format_error, "no FORMAT specifier found");
		else if (strcmp(buf, "FORMAT=32-bit_rle_rgbe\n") == 0)
			break;       /* format found so break out of loop */
		else if (info && (sscanf(buf,"GAMMA=%g", &tempf) == 1)) {
			info->gamma = tempf;
			info->valid |= RGBE_VALID_GAMMA;
		} else if (info && (sscanf(buf, "EXPOSURE=%g", &tempf) == 1)) {
			info->exposure = tempf;
			info->valid |= RGBE_VALID_EXPOSURE;
		}
		if (fp->readLine(buf, sizeof(buf) / sizeof(buf[0])) == 0)
			return rgbe_error(rgbe_read_error, NULL);
	}

#if 0
	if (fgets(buf,sizeof(buf)/sizeof(buf[0]),fp) == 0)
		return rgbe_error(rgbe_read_error,NULL);
	if (strcmp(buf,"\n") != 0)
		return rgbe_error(rgbe_format_error,
		"missing blank line after FORMAT specifier");
#endif

	for (;;) {
		if (fp->readLine(buf, sizeof(buf) / sizeof(buf[0])) == 0)
			return rgbe_error(rgbe_read_error,NULL);

		if (sscanf(buf, "-Y %d +X %d", height, width) == 2)
			break;
	}

	return RGBE_RETURN_SUCCESS;
}

/* simple write routine that does not use run length encoding */
/* These routines can be made faster by allocating a larger buffer and
	fread-ing and fwrite-ing the data in larger chunks */
int RGBE_WritePixels(File *fp, float *data, int numpixels) {
	byte_t rgbe[4];

	while (numpixels-- > 0) {
		float2rgbe(rgbe, data[RGBE_DATA_RED], data[RGBE_DATA_GREEN], data[RGBE_DATA_BLUE]);
		data[RGBE_DATA_ALPHA] = 1.0f;
		data += RGBE_DATA_SIZE;
		if (fp->write(rgbe, sizeof(rgbe) * 1) < 1)
			return rgbe_error(rgbe_write_error,NULL);
	}
	return RGBE_RETURN_SUCCESS;
}

/* simple read routine.  will not correctly handle run length encoding */
int RGBE_ReadPixels(File *fp, float *data, int numpixels) {
	byte_t rgbe[4];

	while (numpixels-- > 0) {
		if (fp->read(rgbe, sizeof(rgbe) * 1) < 1)
			return rgbe_error(rgbe_read_error,NULL);
		rgbe2float(&data[RGBE_DATA_RED],&data[RGBE_DATA_GREEN], &data[RGBE_DATA_BLUE], rgbe);
		data[RGBE_DATA_ALPHA] = 1.0f;
		data += RGBE_DATA_SIZE;
	}
	return RGBE_RETURN_SUCCESS;
}


int RGBE_ReadPixels_Raw(File *fp, byte_t *data, int numpixels) {
	if (fp->read(data, 4 * numpixels) < size_t(4 * numpixels))
		return rgbe_error(rgbe_read_error,NULL);

	return RGBE_RETURN_SUCCESS;
}



/* The code below is only needed for the run-length encoded files. */
/* Run length encoding adds considerable complexity but does */
/* save some space.  For each scanline, each channel (r,g,b,e) is */
/* encoded separately for better compression. */

static int RGBE_WriteBytes_RLE(File *fp, byte_t *data, int numbytes) {
#define MINRUNLENGTH 4
	int cur, beg_run, run_count, old_run_count, nonrun_count;
	byte_t buf[2];

	cur = 0;
	while (cur < numbytes) {
		beg_run = cur;
		/* find next run of length at least 4 if one exists */
		run_count = old_run_count = 0;
		while ((run_count < MINRUNLENGTH) && (beg_run < numbytes)) {
			beg_run += run_count;
			old_run_count = run_count;
			run_count = 1;
			while ((beg_run + run_count < numbytes) && (run_count < 127)
				&& (data[beg_run] == data[beg_run + run_count]))
				run_count++;
		}
		/* if data before next big run is a short run then write it as such */
		if ((old_run_count > 1)&&(old_run_count == beg_run - cur)) {
			buf[0] = 128 + old_run_count;   /*write short run*/
			buf[1] = data[cur];
			if (fp->write(buf, sizeof(buf[0])*2 * 1) < 1)
				return rgbe_error(rgbe_write_error,NULL);
			cur = beg_run;
		}
		/* write out bytes until we reach the start of the next run */
		while (cur < beg_run) {
			nonrun_count = beg_run - cur;
			if (nonrun_count > 128) 
				nonrun_count = 128;
			buf[0] = nonrun_count;
			if (fp->write(buf, sizeof(buf[0]) * 1) < 1)
				return rgbe_error(rgbe_write_error,NULL);
			if (fp->write(&data[cur], sizeof(data[0])*nonrun_count * 1) < 1)
				return rgbe_error(rgbe_write_error,NULL);
			cur += nonrun_count;
		}
		/* write out next run if one was found */
		if (run_count >= MINRUNLENGTH) {
			buf[0] = 128 + run_count;
			buf[1] = data[beg_run];
			if (fp->write(buf, sizeof(buf[0]) * 2 * 1) < 1)
				return rgbe_error(rgbe_write_error,NULL);
			cur += run_count;
		}
	}
	return RGBE_RETURN_SUCCESS;
#undef MINRUNLENGTH
}

int RGBE_WritePixels_RLE(File *fp, float *data, int scanline_width, int num_scanlines) {
	byte_t rgbe[4];
	byte_t *buffer;
	int i, err;

	if ((scanline_width < 8)||(scanline_width > 0x7fff))
		/* run length encoding is not allowed so write flat*/
		return RGBE_WritePixels(fp,data,scanline_width*num_scanlines);
	buffer = new byte_t[4*scanline_width];
	if (buffer == NULL) 
		/* no buffer space so write flat */
		return RGBE_WritePixels(fp,data,scanline_width*num_scanlines);
	while (num_scanlines-- > 0) {
		rgbe[0] = 2;
		rgbe[1] = 2;
		rgbe[2] = scanline_width >> 8;
		rgbe[3] = scanline_width & 0xFF;
		if (fp->write(rgbe, sizeof(rgbe) * 1) < 1) {
			free(buffer);
			return rgbe_error(rgbe_write_error,NULL);
		}
		for (i=0;i<scanline_width;i++) {
			float2rgbe(rgbe,data[RGBE_DATA_RED],
				data[RGBE_DATA_GREEN],data[RGBE_DATA_BLUE]);
			buffer[i] = rgbe[0];
			buffer[i+scanline_width] = rgbe[1];
			buffer[i+2*scanline_width] = rgbe[2];
			buffer[i+3*scanline_width] = rgbe[3];
			data[RGBE_DATA_ALPHA] = 1.0f;
			data += RGBE_DATA_SIZE;
		}
		/* write out each of the four channels separately run length encoded */
		/* first red, then green, then blue, then exponent */
		for (i=0;i<4;i++) {
			if ((err = RGBE_WriteBytes_RLE(fp,&buffer[i*scanline_width],
				scanline_width)) != RGBE_RETURN_SUCCESS) {
					free(buffer);
					return err;
			}
		}
	}
	delete[] (buffer);
	return RGBE_RETURN_SUCCESS;
}

int RGBE_ReadPixels_RLE(File *fp, float *data, int scanline_width, int num_scanlines) {
	byte_t rgbe[4], *scanline_buffer, *ptr, *ptr_end;
	int i, count;
	byte_t buf[2];

	if ((scanline_width < 8)||(scanline_width > 0x7fff))
		/* run length encoding is not allowed so read flat*/
		return RGBE_ReadPixels(fp, data, scanline_width*num_scanlines);
	scanline_buffer = NULL;
	/* read in each successive scanline */
	while (num_scanlines > 0) {
		if (fp->read(rgbe, sizeof(rgbe)*1) < 1) {
			free(scanline_buffer);
			return rgbe_error(rgbe_read_error,NULL);
		}
		if ((rgbe[0] != 2)||(rgbe[1] != 2)||(rgbe[2] & 0x80)) {
			/* this file is not run length encoded */
			rgbe2float(&data[0],&data[1],&data[2],rgbe);
			data[RGBE_DATA_ALPHA] = 1.0f;
			data += RGBE_DATA_SIZE;
			free(scanline_buffer);
			return RGBE_ReadPixels(fp, data, scanline_width*num_scanlines-1);
		}
		if ((((int)rgbe[2])<<8 | rgbe[3]) != scanline_width) {
			free(scanline_buffer);
			return rgbe_error(rgbe_format_error,"wrong scanline width");
		}
		if (scanline_buffer == NULL)
			scanline_buffer = new byte_t[4*scanline_width];
		if (scanline_buffer == NULL) 
			return rgbe_error(rgbe_memory_error,"unable to allocate buffer space");

		ptr = &scanline_buffer[0];
		/* read each of the four channels for the scanline into the buffer */
		for (i=0; i<4; i++) {
			ptr_end = &scanline_buffer[(i+1)*scanline_width];
			while (ptr < ptr_end) {
				if (fp->read(buf, sizeof(buf[0])*2*1) < 1) {
					free(scanline_buffer);
					return rgbe_error(rgbe_read_error,NULL);
				}
				if (buf[0] > 128) {
					/* a run of the same value */
					count = buf[0]-128;
					if ((count == 0)||(count > ptr_end - ptr)) {
						free(scanline_buffer);
						return rgbe_error(rgbe_format_error,"bad scanline data");
					}
					while (count-- > 0)
						*ptr++ = buf[1];
				} else {
					/* a non-run */
					count = buf[0];
					if ((count == 0)||(count > ptr_end - ptr)) {
						free(scanline_buffer);
						return rgbe_error(rgbe_format_error,"bad scanline data");
					}
					*ptr++ = buf[1];
					if (--count > 0) {
						if (fp->read(ptr, sizeof(*ptr)*count*1) < 1) {
							free(scanline_buffer);
							return rgbe_error(rgbe_read_error,NULL);
						}
						ptr += count;
					}
				}
			}
		}
		/* now convert data from buffer into floats */
		for (i=0; i<scanline_width; i++) {
			rgbe[0] = scanline_buffer[i];
			rgbe[1] = scanline_buffer[i+scanline_width];
			rgbe[2] = scanline_buffer[i+2*scanline_width];
			rgbe[3] = scanline_buffer[i+3*scanline_width];
			rgbe2float(&data[RGBE_DATA_RED], &data[RGBE_DATA_GREEN], &data[RGBE_DATA_BLUE], rgbe);
			data[RGBE_DATA_ALPHA] = 1.0f;
			data += RGBE_DATA_SIZE;
		}
		num_scanlines--;
	}
	delete[] (scanline_buffer);
	return RGBE_RETURN_SUCCESS;
}


int RGBE_ReadPixels_Raw_RLE(File *fp, byte_t *data, int scanline_width, int num_scanlines) {
	byte_t rgbe[4], *scanline_buffer, *ptr, *ptr_end;
	int i, count;
	byte_t buf[2];

	if ((scanline_width < 8)||(scanline_width > 0x7fff))
		/* run length encoding is not allowed so read flat*/
		return RGBE_ReadPixels_Raw(fp,data,scanline_width*num_scanlines);

	scanline_buffer = NULL;
	/* read in each successive scanline */
	while (num_scanlines > 0) {
		if (fp->read(rgbe,sizeof(rgbe)*1) < 1) {
			free(scanline_buffer);
			return rgbe_error(rgbe_read_error,NULL);
		}

		if ((rgbe[0] != 2)||(rgbe[1] != 2)||(rgbe[2] & 0x80)) {
			/* this file is not run length encoded */
			data[0] = rgbe[0];
			data[1] = rgbe[1];
			data[2] = rgbe[2];
			data[3] = rgbe[3];
			data += RGBE_DATA_SIZE;
			free(scanline_buffer);
			return RGBE_ReadPixels_Raw(fp,data,scanline_width*num_scanlines-1);
		}

		if ((((int)rgbe[2])<<8 | rgbe[3]) != scanline_width) {
			free(scanline_buffer);
			return rgbe_error(rgbe_format_error,"wrong scanline width");
		}

		if (scanline_buffer == NULL)
			scanline_buffer = new byte_t[4*scanline_width];

		if (scanline_buffer == NULL) 
			return rgbe_error(rgbe_memory_error,"unable to allocate buffer space");

		ptr = &scanline_buffer[0];
		/* read each of the four channels for the scanline into the buffer */
		for (i=0;i<4;i++) {
			ptr_end = &scanline_buffer[(i+1)*scanline_width];
			while (ptr < ptr_end) {
				if (fp->read(buf, sizeof(buf[0])*2*1) < 1) {
					free(scanline_buffer);
					return rgbe_error(rgbe_read_error,NULL);
				}
				if (buf[0] > 128) {
					/* a run of the same value */
					count = buf[0]-128;
					if ((count == 0)||(count > ptr_end - ptr)) {
						free(scanline_buffer);
						return rgbe_error(rgbe_format_error,"bad scanline data");
					}
					while (count-- > 0)
						*ptr++ = buf[1];
				}
				else {
					/* a non-run */
					count = buf[0];
					if ((count == 0)||(count > ptr_end - ptr)) {
						free(scanline_buffer);
						return rgbe_error(rgbe_format_error,"bad scanline data");
					}
					*ptr++ = buf[1];
					if (--count > 0) {
						if (fp->read(ptr,sizeof(*ptr)*count*1) < 1) {
							free(scanline_buffer);
							return rgbe_error(rgbe_read_error,NULL);
						}
						ptr += count;
					}
				}
			}
		}
		/* copy byte data to output */
		for (i=0; i<scanline_width; i++) {
			data[0] = scanline_buffer[i];
			data[1] = scanline_buffer[i+scanline_width];
			data[2] = scanline_buffer[i+2*scanline_width];
			data[3] = scanline_buffer[i+3*scanline_width];
			data += 4;
		}
		num_scanlines--;
	}
	delete[] (scanline_buffer);
	return RGBE_RETURN_SUCCESS;
}

bool
Image::loadFile_hdr(const std::string &filename) {
	clear();

	/* load file */ 
	File *file = g_fileSystem->openFileRead(filename);

	std::auto_ptr<File> fileptr(file);

	if (!file) {
		return false;
	}

	rgbe_header_info header;
	if (RGBE_ReadHeader(file, &m_width, &m_height, &header)) {
		return false;
	}

#if 0
	if (m_loadFlags & Image::RgbeRaw) {
		byte_t *pixels = new byte_t[m_width * m_height * 4];
		if (RGBE_ReadPixels_Raw_RLE(file, pixels, m_width, m_height)) {
			SafeFree(pixels);
			return false;
		}

		m_format = TexFormat::RGBA8;
		m_datas.push_back(DataBuffer(pixels));

		return true;
	}
#endif

	byte_t *pixels = new byte_t[m_width * m_height * RGBE_DATA_SIZE * sizeof(float)];
	if (RGBE_ReadPixels_RLE(file, (float*)pixels, m_width, m_height)) {
		SafeFree(pixels);
		return false;
	}

	m_format = TexFormat::RGBA32F;
	m_datas.push_back(DataBuffer(pixels));

	return m_dataPresent = true;
}

AX_END_NAMESPACE
