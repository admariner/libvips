/* TIFF parts: Copyright (c) 1988, 1990 by Sam Leffler.
 * All rights reserved.
 *
 * This file is provided for unrestricted use provided that this
 * legend is included on all tape media and as a part of the
 * software program in whole or part.  Users may copy, modify or
 * distribute this file at will.
 * -----------------------------
 * Modifications for VIPS:  Kirk Martinez 1994
 * 22/11/94 JC
 *	- more general
 *	- memory leaks fixed
 * 20/3/95 JC
 *	- TIFF error handler added
 *	- read errors detected correctly
 *
 * Modified to handle LAB in tiff format.
 * It convert LAB-tiff format to VIPS_INTERPRETATION_LABQ in vips format.
 *  Copyright July-1995 Ahmed Abbood.
 *
 *
 * 19/9/95 JC
 *	- now calls TIFFClose ... stupid
 * 25/1/96 JC
 *	- typo on MINISBLACK ...
 * 7/4/97 JC
 *	- completely redone for TIFF 6
 *	- now full baseline TIFF 6 reader, and does CIELAB as well
 * 11/4/97 JC
 *	- added partial read for tiled images
 * 23/4/97 JC
 *	- extra subsample parameter
 *	- im_istiffpyramid() added
 * 5/12/97 JC
 *	- if loading YCbCr, convert to VIPS_CODING_LABQ
 * 1/5/98 JC
 *	- now reads 16-bit greyscale and RGB
 * 26/10/98 JC
 *	- now used "rb" mode on systems that need binary open
 * 12/11/98 JC
 *	- no sub-sampling if sub == 1
 * 26/2/99 JC
 *	- ooops, else missing for subsample stuff above
 * 2/10/99 JC
 *	- tiled 16-bit greyscale read was broken
 *	- added mutex for TIFFReadTile() calls
 * 11/5/00 JC
 *	- removed TIFFmalloc/TIFFfree usage
 * 23/4/01 JC
 *	- HAVE_TIFF turns on TIFF goodness
 * 24/5/01 JC
 *	- im_tiff2vips_header() added
 * 11/7/01 JC
 *	- subsample now in input filename
 *	- ... and it's a page number (from 0) instead
 * 21/8/02 JC
 *	- now reads CMYK
 *	- hmm, dpi -> ppm conversion was wrong!
 * 10/9/02 JC
 *	- oops, handle TIFF errors better
 * 2/12/02 JC
 *	- reads 8-bit RGBA
 * 12/12/02 JC
 *	- reads 16-bit LAB
 * 13/2/03 JC
 *	- pixels/cm res read was wrong
 * 17/11/03 Andrey Kiselev
 *	- read 32-bit float greyscale and rgb
 * 5/4/04
 *	- better handling of edge tiles (thanks Ruven)
 * 16/4/04
 *	- cleanup
 *	- added broken tile read mode
 * 18/5/04 Andrey Kiselev
 *	- better no resolution diagnostic
 * 26/5/04
 *	- reads 16 bit RGBA
 * 28/7/04
 *	- arrg, 16bit RGB was broken, thanks haida
 * 26/11/04
 *	- add a TIFF warning handler, stops occasional libMagick exceptions
 * 9/3/05
 *	- load 32-bit float LAB
 * 8/4/05
 *	- onebit read no longer reads one byte too many on multiple of 8 wide
 *	  images
 * 22/6/05
 *	- 16 bit LAB read was broken
 * 9/9/05
 * 	- read any ICCPROFILE tag
 * 8/5/06
 * 	- set RGB16 and GREY16 Type
 * 21/5/06
 * 	- use external im_tile_cache() operation for great code shrinkage
 * 	- less RAM usage too, esp. with >1 CPU
 * 	- should be slightly faster
 * 	- removed 'broken' read option
 * 18/7/07 Andrey Kiselev
 * 	- remove "b" option on TIFFOpen()
 * 9/4/08
 * 	- set VIPS_META_RESOLUTION_UNIT
 * 17/4/08
 * 	- allow CMYKA (thanks Doron)
 * 17/7/08
 * 	- convert YCbCr to RGB on read (thanks Ole)
 * 15/8/08
 * 	- reorganise for image format system
 * 20/12/08
 * 	- dont read with mmap: no performance advantage with libtiff, chews up
 * 	  VM wastefully
 * 13/1/09
 * 	- read strip-wise, not scanline-wise ... works with more compression /
 * 	  subsampling schemes (esp. subsampled YCbCr), and it's a bit quicker
 * 4/2/10
 * 	- gtkdoc
 * 12/12/10
 * 	- oops, we can just memcpy() now heh
 * 	- avoid unpacking via buffers if we can: either read a tile directly
 * 	  into the output region, or writeline directly from the tiff buffer
 * 4/4/11
 * 	- argh int/uint mixup for rows_per_strip, thanks Bubba
 * 21/4/11
 * 	- palette read can do 1,2,4,8 bits per sample
 * 	- palette read can do mono images
 * 5/12/11
 * 	- make into a simple function call ready to be wrapped as a new-style
 * 	  VipsForeign class
 * 18/2/12
 * 	- switch to sequential read
 * 	- remove the lock ... tilecache does this for us
 * 3/6/12
 * 	- always offer THINSTRIP ... later stages can ask for something more
 * 	  relaxed if they wish
 * 7/6/12
 * 	- clip rows_per_strip down to image height to avoid overflows for huge
 * 	  values (thanks Nicolas)
 * 	- better error msg for not PLANARCONFIG_CONTIG images
 * 16/9/13
 * 	- support alpha for 8, 16 and 32-bit greyscale images, thanks Robert
 * 17/9/13
 * 	- support separate planes for strip read
 * 	- big cleanup
 * 	- support for many more formats, eg. 32-bit int etc.
 * 11/4/14
 * 	- support 16 bits per sample palette images
 * 	- palette images can have an alpha
 * 22/4/14
 * 	- add read from buffer
 * 30/4/14
 * 	- 1/2/4 bit palette images can have alpha
 * 27/10/14 Lovell
 * 	- better istiff detector spots bigtiff
 * 3/12/14
 * 	- read any XMP metadata
 * 19/1/15
 * 	- try to handle 8-bit colormaps
 * 26/2/15
 * 	- close the read down early for a header read ... this saves an
 * 	  fd during file read, handy for large numbers of input images
 * 29/9/15
 * 	- load IPTC metadata
 * 	- load photoshop metadata
 * 21/12/15
 * 	- load TIFFTAG_IMAGEDESCRIPTION
 * 11/4/16
 * 	- non-int RGB images are tagged as scRGB ... matches photoshop
 * 	  convention
 * 26/5/16
 * 	- add autorotate support
 * 17/11/16
 * 	- add multi-page read
 * 17/1/17
 * 	- invalidate operation on read error
 * 27/1/17
 * 	- if rows_per_strip is large, read with scanline API instead
 * 9/5/17
 * 	- remove missing res warning
 * 19/5/17
 * 	- page > 0 could break edge tiles or strips
 * 26/4/18
 * 	- add n-pages metadata item
 * 21/7/18
 * 	- check for non-byte-multiple bits_per_sample [HongxuChen]
 * 16/8/18
 * 	- shut down the input file as soon as we can [kleisauke]
 * 28/3/19 omira-sch
 * 	- better buffer sizing
 * 	- ban chroma-subsampled, non-jpg compressed images
 * 7/6/19
 * 	- istiff reads the first directory rather than just testing the magic
 * 	  number, so it ignores more TIFF-like, but not TIFF images
 * 17/10/19
 * 	- switch to source input
 * 18/11/19
 * 	- support ASSOCALPHA in any alpha band
 * 27/1/20
 * 	- read logluv images as XYZ
 * 11/4/20 petoor
 * 	- better handling of aligned reads in multipage tiffs
 * 28/5/20
 * 	- add subifd
 * 6/6/20 MathemanFlo
 * 	- support 2 and 4 bit greyscale load
 * 27/3/21
 * 	- add jp2k decompression
 * 24/7/21
 * 	- add fail_on
 * 30/9/21
 * 	- fix tiled + packed formats
 * 31/7/22
 *  - move jp2k decompress outside the lock
 *  - move jpeg decode outside the lock
 *  - fix demand hinting
 * 3/2/23 MathemanFlo
 * 	- add bits per sample metadata
 */

/*

	This file is part of VIPS.

	VIPS is free software; you can redistribute it and/or modify
	it under the terms of the GNU Lesser General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
	02110-1301  USA

 */

/*

	These files are distributed with VIPS - http://www.vips.ecs.soton.ac.uk

 */

/*
#define DEBUG_VERBOSE
#define DEBUG
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /*HAVE_CONFIG_H*/
#include <glib/gi18n-lib.h>

#ifdef HAVE_TIFF

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <vips/vips.h>
#include <vips/internal.h>

#include "pforeign.h"
#include "tiff.h"

/* We do jpeg decompress ourselves, if we can.
 */
#ifdef HAVE_JPEG
#include "jpeg.h"
#endif /*HAVE_JPEG*/

/* Compression types we handle ourselves.
 */
static int rtiff_we_decompress[] = {
#ifdef HAVE_JPEG
	COMPRESSION_JPEG,
#endif /*HAVE_JPEG*/
	JP2K_YCC,
	JP2K_RGB,
	JP2K_LOSSY
};

/* What we read from the tiff dir to set our read strategy. For multipage
 * read, we need to read and compare lots of these, so it needs to be broken
 * out as a separate thing.
 */
typedef struct _RtiffHeader {
	guint32 width;
	guint32 height;
	int samples_per_pixel;
	int bits_per_sample;
	int photometric_interpretation;
	int inkset;
	int sample_format;
	gboolean separate;
	int orientation;

	/* If there's a premultiplied alpha, the band we need to
	 * unpremultiply with. -1 for no unpremultiplication.
	 */
	int alpha_band;
	guint16 compression;

	/* Is this directory tiled.
	 */
	gboolean tiled;

	/* Fields for tiled images, as returned by libtiff.
	 */
	guint32 tile_width;
	guint32 tile_height;
	tsize_t tile_size;
	tsize_t tile_row_size;

	/* Fields for strip images, as returned by libtiff.
	 */
	guint32 rows_per_strip;
	tsize_t strip_size;
	tsize_t scanline_size;
	int number_of_strips;

	/* If read_scanlinewise is TRUE, the strips are too large to read in a
	 * single lump and we will use the scanline API.
	 */
	gboolean read_scanlinewise;

	/* Strip read geometry. Number of lines we read at once (whole strip
	 * or 1) and size of the buffer we read to (a scanline, or a strip in
	 * size).
	 */
	guint32 read_height;
	tsize_t read_size;

	/* Scale factor to get absolute cd/m2 from XYZ.
	 */
	double stonits;

	/* Number of subifds, 0 for none.
	 */
	int subifd_count;

	/* Optional IMAGEDESCRIPTION.
	 */
	char *image_description;

	/* TRUE if we decompress ourselves rather than relying on libtiff.
	 */
	gboolean we_decompress;

	/* TRUE if we use TIFFRGBAImage or TIFFReadRGBATile.
	 * Used for COMPRESSION_OJPEG
	 */
	gboolean read_as_rgba;

} RtiffHeader;

/* Scanline-type process function.
 */
struct _Rtiff;
typedef void (*scanline_process_fn)(struct _Rtiff *,
	VipsPel *q, VipsPel *p, int n, void *client);

/* Stuff we track during a read.
 */
typedef struct _Rtiff {
	/* Parameters.
	 */
	VipsSource *source;
	VipsImage *out;
	int page;
	int n;
	gboolean autorotate;
	int subifd;
	VipsFailOn fail_on;

	/* We decompress some compression types in parallel, so we need to
	 * lock tile get.
	 */
	GRecMutex lock;

	/* The TIFF we read.
	 */
	TIFF *tiff;

	/* Number of pages (directories) in image.
	 */
	int n_pages;

	/* The current page we have set.
	 */
	int current_page;

	/* Process for this image type.
	 */
	scanline_process_fn sfn;
	void *client;

	/* Set this is the processfn is just doing a memcpy.
	 */
	gboolean memcpy;

	/* Geometry as read from the TIFF header. This is read for the first
	 * page, and equal for all other pages.
	 */
	RtiffHeader header;

	/* Hold a single strip or tile, possibly just an image plane.
	 */
	tdata_t plane_buf;

	/* Hold a plane-assembled strip or tile ... a set of samples_per_pixel
	 * strips or tiles interleaved.
	 */
	tdata_t contig_buf;

	/* The Y we are reading at. Used to verify strip read is sequential.
	 */
	int y_pos;

	/* Stop processing due to an error or warning.
	 */
	gboolean failed;
} Rtiff;

/* Convert IEEE 754-2008 16-bit float to 32-bit float
 */
static inline float
half_2_float(gushort h)
{
	const float sign = (h >> 15) * -2 + 1;
	const int exp = ((h & 0x7C00) >> 10) - 15;
	const float prec = (h & 0x03FF);

	switch (exp) {
	case 16:
		return INFINITY * sign;
	case -15:
		return sign / (float) (1 << 14) * (prec / 1024.0F);
	default:
		return exp > 0
			? sign * (float) (1 << exp) * (1.0F + prec / 1024.0F)
			: sign / (float) (1 << -exp) * (1.0F + prec / 1024.0F);
	}
}

/* Test for field exists.
 */
static int
tfexists(TIFF *tif, ttag_t tag)
{
	guint32 a, b;

	if (TIFFGetField(tif, tag, &a, &b))
		return 1;
	else
		return 0;
}

/* Get a guint32 field.
 */
static int
tfget32(TIFF *tif, ttag_t tag, guint32 *out)
{
	guint32 fld;

	if (!TIFFGetFieldDefaulted(tif, tag, &fld)) {
		vips_error("tiff2vips",
			_("required field %d missing"), tag);
		return 0;
	}

	*out = fld;

	return 1;
}

/* Get a guint16 field.
 */
static int
tfget16(TIFF *tif, ttag_t tag, int *out)
{
	guint16 fld;

	if (!TIFFGetFieldDefaulted(tif, tag, &fld)) {
		vips_error("tiff2vips",
			_("required field %d missing"), tag);
		return 0;
	}

	*out = fld;

	return 1;
}

static int
get_resolution(TIFF *tiff, VipsImage *out)
{
	float x, y;
	int ru;

	if (TIFFGetFieldDefaulted(tiff, TIFFTAG_XRESOLUTION, &x) &&
		TIFFGetFieldDefaulted(tiff, TIFFTAG_YRESOLUTION, &y) &&
		tfget16(tiff, TIFFTAG_RESOLUTIONUNIT, &ru)) {
		switch (ru) {
		case RESUNIT_NONE:
			break;

		case RESUNIT_INCH:
			/* In pixels-per-inch ... convert to mm.
			 */
			x /= 10.0F * 2.54F;
			y /= 10.0F * 2.54F;
			vips_image_set_string(out,
				VIPS_META_RESOLUTION_UNIT, "in");
			break;

		case RESUNIT_CENTIMETER:
			/* In pixels-per-centimetre ... convert to mm.
			 */
			x /= 10.0F;
			y /= 10.0F;
			vips_image_set_string(out,
				VIPS_META_RESOLUTION_UNIT, "cm");
			break;

		default:
			vips_error("tiff2vips",
				"%s", _("unknown resolution unit"));
			return -1;
		}
	}
	else {
		/* We used to warn about missing res data, but it happens so
		 * often and is so harmless, why bother.
		 */
		x = 1.0F;
		y = 1.0F;
	}

	out->Xres = x;
	out->Yres = y;

	return 0;
}

static int
get_sample_format(TIFF *tiff)
{
	int sample_format;
	guint16 v;

	sample_format = SAMPLEFORMAT_INT;

	if (TIFFGetFieldDefaulted(tiff, TIFFTAG_SAMPLEFORMAT, &v)) {
		/* Some images have this set to void, bizarre.
		 */
		if (v == SAMPLEFORMAT_VOID)
			v = SAMPLEFORMAT_UINT;

		sample_format = v;
	}

	return sample_format;
}

static int
get_orientation(TIFF *tiff)
{
	int orientation;
	guint16 v;

	orientation = ORIENTATION_TOPLEFT;

	if (TIFFGetFieldDefaulted(tiff, TIFFTAG_ORIENTATION, &v))
		/* Can have mad values.
		 */
		orientation = VIPS_CLIP(1, v, 8);

	return orientation;
}

/* Can be called many times.
 */
static void
rtiff_free(Rtiff *rtiff)
{
	VIPS_FREEF(TIFFClose, rtiff->tiff);
	g_rec_mutex_clear(&rtiff->lock);
	VIPS_UNREF(rtiff->source);
}

static void
rtiff_close_cb(VipsImage *image, Rtiff *rtiff)
{
	rtiff_free(rtiff);
}

static void
rtiff_minimise_cb(VipsImage *image, Rtiff *rtiff)
{
	/* We must not minimised tiled images. These can be read from many
	 * threads, and this minimise handler is not inside the lock.
	 */
	if (!rtiff->header.tiled &&
		rtiff->source)
		vips_source_minimise(rtiff->source);
}

static int
rtiff_handler_error(TIFF *tiff, void* user_data,
	const char *module, const char *fmt, va_list ap)
{
	vips_verror("tiff2vips", fmt, ap);
	return 1;
}

static int
rtiff_handler_warning(TIFF *tiff, void* user_data,
	const char *module, const char *fmt, va_list ap)
{
	if (user_data) {
		Rtiff *rtiff = (Rtiff*) user_data;
		if (rtiff->fail_on >= VIPS_FAIL_ON_WARNING) {
			rtiff->failed = TRUE;
		}
	}

	g_logv(G_LOG_DOMAIN, G_LOG_LEVEL_WARNING, fmt, ap);
	return 1;
}

static Rtiff *
rtiff_new(VipsSource *source, VipsImage *out,
	int page, int n, gboolean autorotate, int subifd, VipsFailOn fail_on,
	gboolean unlimited)
{
	Rtiff *rtiff;

	if (!(rtiff = VIPS_NEW(out, Rtiff)))
		return NULL;

	g_object_ref(source);
	rtiff->source = source;
	rtiff->out = out;
	rtiff->page = page;
	rtiff->n = n;
	rtiff->autorotate = autorotate;
	rtiff->subifd = subifd;
	rtiff->fail_on = fail_on;
	g_rec_mutex_init(&rtiff->lock);
	rtiff->tiff = NULL;
	rtiff->n_pages = 0;
	rtiff->current_page = -1;
	rtiff->sfn = NULL;
	rtiff->client = NULL;
	rtiff->memcpy = FALSE;
	rtiff->plane_buf = NULL;
	rtiff->contig_buf = NULL;
	rtiff->y_pos = 0;
	rtiff->failed = FALSE;

	g_signal_connect(out, "close",
		G_CALLBACK(rtiff_close_cb), rtiff);
	g_signal_connect(out, "minimise",
		G_CALLBACK(rtiff_minimise_cb), rtiff);

	if (rtiff->page < 0 ||
		rtiff->page > 1000000) {
		vips_error("tiff2vips", _("bad page number %d"),
			rtiff->page);
		return NULL;
	}

	/* We allow n == -1, meaning all pages. It gets swapped for a real n
	 * value when we open the TIFF.
	 */
	if (rtiff->n != -1 &&
		(rtiff->n < 1 || rtiff->n > 1000000)) {
		vips_error("tiff2vips", _("bad number of pages %d"),
			rtiff->n);
		return NULL;
	}

	if (!(rtiff->tiff = vips__tiff_openin_source(source,
		rtiff_handler_error, rtiff_handler_warning, rtiff, unlimited)))
		return NULL;

	return rtiff;
}

static int
rtiff_strip_read(Rtiff *rtiff, int strip, tdata_t buf)
{
	tsize_t length;

#ifdef DEBUG_VERBOSE
	printf("rtiff_strip_read: reading strip %d\n", strip);
#endif /*DEBUG_VERBOSE*/

	if (rtiff->header.read_scanlinewise)
		length = TIFFReadScanline(rtiff->tiff, buf, strip, (tsample_t) 0);
	else
		length = TIFFReadEncodedStrip(rtiff->tiff, strip, buf, (tsize_t) -1);

	if (length == -1 && rtiff->fail_on >= VIPS_FAIL_ON_WARNING) {
		vips_foreign_load_invalidate(rtiff->out);
		vips_error("tiff2vips", "%s", _("read error"));
		return -1;
	}

	if (rtiff->failed) {
		vips_foreign_load_invalidate(rtiff->out);
		return -1;
	}

	return 0;
}

static int
rtiff_rgba_strip_read(Rtiff *rtiff, int strip, tdata_t buf)
{
	RtiffHeader *header = &rtiff->header;

	TIFFRGBAImage img;
	guint32 rows_to_read;
	char err[1024] = "";

#ifdef DEBUG_VERBOSE
	printf("rtiff_rgba_strip_read: reading strip %d\n", strip);
#endif /*DEBUG_VERBOSE*/

	if (!TIFFRGBAImageOK(rtiff->tiff, err) ||
		!TIFFRGBAImageBegin(&img, rtiff->tiff, 0, err)) {
		vips_foreign_load_invalidate(rtiff->out);
		vips_error("tiff2vips", "%s", err);
		return -1;
	}

	img.req_orientation = header->orientation;
	img.row_offset = strip * header->rows_per_strip;
	img.col_offset = 0;

	rows_to_read =
		VIPS_MIN(header->rows_per_strip, header->height - img.row_offset);

	if (!TIFFRGBAImageGet(&img, buf, header->width, rows_to_read)) {
		TIFFRGBAImageEnd(&img);
		vips_foreign_load_invalidate(rtiff->out);
		vips_error("tiff2vips", "%s", _("read error"));
		return -1;
	}

	TIFFRGBAImageEnd(&img);

	if (rtiff->failed) {
		vips_foreign_load_invalidate(rtiff->out);
		return -1;
	}

	return 0;
}

/* We need to hint to libtiff what format we'd like pixels in.
 */
static void
rtiff_set_decode_format(Rtiff *rtiff)
{
	/* Ask for YCbCr->RGB for jpg data.
	 */
	if (rtiff->header.compression == COMPRESSION_JPEG ||
		rtiff->header.compression == COMPRESSION_OJPEG)
		TIFFSetField(rtiff->tiff, TIFFTAG_JPEGCOLORMODE, JPEGCOLORMODE_RGB);

	/* Ask for SGI LOGLUV as 3xfloat.
	 */
	if (rtiff->header.photometric_interpretation ==
		PHOTOMETRIC_LOGLUV)
		TIFFSetField(rtiff->tiff, TIFFTAG_SGILOGDATAFMT, SGILOGDATAFMT_FLOAT);
}

static int
rtiff_set_page(Rtiff *rtiff, int page)
{
	if (rtiff->current_page != page) {
#ifdef DEBUG
		printf("rtiff_set_page: selecting page %d, subifd %d\n",
			page, rtiff->subifd);
#endif /*DEBUG*/

		if (!TIFFSetDirectory(rtiff->tiff, page)) {
			vips_error("tiff2vips", _("TIFF does not contain page %d"), page);
			return -1;
		}

		if (rtiff->subifd >= 0) {
			guint16 subifd_count;
			toff_t *subifd_offsets;

			if (!TIFFGetField(rtiff->tiff, TIFFTAG_SUBIFD,
					&subifd_count, &subifd_offsets)) {
				vips_error("tiff2vips", "%s", _("no SUBIFD tag"));
				return -1;
			}

			if (rtiff->subifd >= subifd_count) {
				vips_error("tiff2vips",
					_("subifd %d out of range, only 0-%d available"),
					rtiff->subifd,
					subifd_count - 1);
				return -1;
			}

			if (!TIFFSetSubDirectory(rtiff->tiff,
					subifd_offsets[rtiff->subifd])) {
				vips_error("tiff2vips", "%s", _("subdirectory unreadable"));
				return -1;
			}
		}

		rtiff->current_page = page;

		/* These can get unset when we change directories. Make sure
		 * they are set again.
		 */
		rtiff_set_decode_format(rtiff);
	}

	return 0;
}

static int
rtiff_n_pages(Rtiff *rtiff)
{
	int n;

	(void) TIFFSetDirectory(rtiff->tiff, 0);

	for (n = 1; TIFFReadDirectory(rtiff->tiff); n++)
		;

	/* Make sure the nest set_page() will set the directory.
	 */
	rtiff->current_page = -1;

#ifdef DEBUG
	printf("rtiff_n_pages: found %d pages\n", n);
#endif /*DEBUG*/

	return n;
}

static int
rtiff_check_samples(Rtiff *rtiff, int samples_per_pixel)
{
	if (rtiff->header.samples_per_pixel != samples_per_pixel) {
		vips_error("tiff2vips", _("not %d bands"), samples_per_pixel);
		return -1;
	}

	return 0;
}

/* Check n and n+1 so we can have an alpha.
 */
static int
rtiff_check_min_samples(Rtiff *rtiff, int samples_per_pixel)
{
	if (rtiff->header.samples_per_pixel < samples_per_pixel) {
		vips_error("tiff2vips", _("not at least %d samples per pixel"),
			samples_per_pixel);
		return -1;
	}

	return 0;
}

/* Only allow samples which are whole bytes in size.
 */
static int
rtiff_non_fractional(Rtiff *rtiff)
{
	if (rtiff->header.bits_per_sample % 8 != 0 ||
		rtiff->header.bits_per_sample == 0) {
		vips_error("tiff2vips", "%s", _("samples_per_pixel "
										"not a whole number of bytes"));
		return -1;
	}

	return 0;
}

static int
rtiff_check_interpretation(Rtiff *rtiff, int photometric_interpretation)
{
	if (rtiff->header.photometric_interpretation !=
		photometric_interpretation) {
		vips_error("tiff2vips", _("not photometric interpretation %d"),
			photometric_interpretation);
		return -1;
	}

	return 0;
}

static int
rtiff_check_bits(Rtiff *rtiff, int bits_per_sample)
{
	if (rtiff->header.bits_per_sample != bits_per_sample) {
		vips_error("tiff2vips", _("not %d bits per sample"), bits_per_sample);
		return -1;
	}

	return 0;
}

static int
rtiff_check_bits_palette(Rtiff *rtiff)
{
	if (rtiff->header.bits_per_sample != 16 &&
		rtiff->header.bits_per_sample != 8 &&
		rtiff->header.bits_per_sample != 4 &&
		rtiff->header.bits_per_sample != 2 &&
		rtiff->header.bits_per_sample != 1) {
		vips_error("tiff2vips",
			_("%d bits per sample palette image not supported"),
			rtiff->header.bits_per_sample);
		return -1;
	}

	return 0;
}

static VipsBandFormat
rtiff_guess_format(Rtiff *rtiff)
{
	int bits_per_sample = rtiff->header.bits_per_sample;
	int sample_format = rtiff->header.sample_format;

	switch (bits_per_sample) {
	case 1:
	case 2:
	case 4:
	case 8:
		if (sample_format == SAMPLEFORMAT_INT)
			return VIPS_FORMAT_CHAR;
		if (sample_format == SAMPLEFORMAT_UINT)
			return VIPS_FORMAT_UCHAR;
		break;

	case 16:
		if (sample_format == SAMPLEFORMAT_INT)
			return VIPS_FORMAT_SHORT;
		if (sample_format == SAMPLEFORMAT_UINT)
			return VIPS_FORMAT_USHORT;
		if (sample_format == SAMPLEFORMAT_IEEEFP)
			return VIPS_FORMAT_FLOAT;
		break;

	case 32:
		if (sample_format == SAMPLEFORMAT_INT)
			return VIPS_FORMAT_INT;
		if (sample_format == SAMPLEFORMAT_UINT)
			return VIPS_FORMAT_UINT;
		if (sample_format == SAMPLEFORMAT_IEEEFP)
			return VIPS_FORMAT_FLOAT;
		break;

	case 64:
		if (sample_format == SAMPLEFORMAT_IEEEFP)
			return VIPS_FORMAT_DOUBLE;
		if (sample_format == SAMPLEFORMAT_COMPLEXIEEEFP)
			return VIPS_FORMAT_COMPLEX;
		break;

	case 128:
		if (sample_format == SAMPLEFORMAT_COMPLEXIEEEFP)
			return VIPS_FORMAT_DPCOMPLEX;
		break;

	default:
		break;
	}

	vips_error("tiff2vips", "%s", _("unsupported tiff image type\n"));

	return VIPS_FORMAT_NOTSET;
}

/* Per-scanline process function for VIPS_CODING_LABQ.
 */
static void
rtiff_labpack_line(Rtiff *rtiff, VipsPel *q, VipsPel *p, int n, void *dummy)
{
	int samples_per_pixel = rtiff->header.samples_per_pixel;

	int x;

	for (x = 0; x < n; x++) {
		q[0] = p[0];
		q[1] = p[1];
		q[2] = p[2];
		q[3] = 0;

		q += 4;
		p += samples_per_pixel;
	}
}

/* Read an 8-bit LAB image.
 */
static int
rtiff_parse_labpack(Rtiff *rtiff, VipsImage *out)
{
	if (rtiff_check_min_samples(rtiff, 3) ||
		rtiff_check_bits(rtiff, 8) ||
		rtiff_check_interpretation(rtiff, PHOTOMETRIC_CIELAB))
		return -1;

	out->Bands = 4;
	out->BandFmt = VIPS_FORMAT_UCHAR;
	out->Coding = VIPS_CODING_LABQ;
	out->Type = VIPS_INTERPRETATION_LAB;

	rtiff->sfn = rtiff_labpack_line;

	return 0;
}

/* Per-scanline process function for 8-bit VIPS_CODING_LAB to 16-bit LabS with
 * alpha.
 */
static void
rtiff_lab_with_alpha_line(Rtiff *rtiff,
	VipsPel *q, VipsPel *p, int n, void *dummy)
{
	int samples_per_pixel = rtiff->header.samples_per_pixel;

	unsigned char *p1;
	short *q1;
	int x;

	p1 = (unsigned char *) p;
	q1 = (short *) q;
	for (x = 0; x < n; x++) {
		int i;

		q1[0] = ((unsigned int) p1[0]) * 32767 / 255;
		q1[1] = ((short) p1[1]) << 8;
		q1[2] = ((short) p1[2]) << 8;

		for (i = 3; i < samples_per_pixel; i++)
			q1[i] = (p1[i] << 8) + p1[i];

		q1 += samples_per_pixel;
		p1 += samples_per_pixel;
	}
}

/* Read an 8-bit LAB image with alpha bands into 16-bit LabS.
 */
static int
rtiff_parse_lab_with_alpha(Rtiff *rtiff, VipsImage *out)
{
	if (rtiff_check_min_samples(rtiff, 4) ||
		rtiff_check_bits(rtiff, 8) ||
		rtiff_check_interpretation(rtiff, PHOTOMETRIC_CIELAB))
		return -1;

	out->Bands = rtiff->header.samples_per_pixel;
	out->BandFmt = VIPS_FORMAT_SHORT;
	out->Coding = VIPS_CODING_NONE;
	out->Type = VIPS_INTERPRETATION_LABS;

	rtiff->sfn = rtiff_lab_with_alpha_line;

	return 0;
}

/* Per-scanline process function for LABS.
 */
static void
rtiff_labs_line(Rtiff *rtiff, VipsPel *q, VipsPel *p, int n, void *dummy)
{
	int samples_per_pixel = rtiff->header.samples_per_pixel;

	unsigned short *p1;
	short *q1;
	int x;
	int i;

	p1 = (unsigned short *) p;
	q1 = (short *) q;
	for (x = 0; x < n; x++) {
		/* We use signed int16 for L.
		 */
		q1[0] = p1[0] >> 1;

		for (i = 1; i < samples_per_pixel; i++)
			q1[i] = p1[i];

		q1 += samples_per_pixel;
		p1 += samples_per_pixel;
	}
}

/* Read a 16-bit LAB image.
 */
static int
rtiff_parse_labs(Rtiff *rtiff, VipsImage *out)
{
	if (rtiff_check_min_samples(rtiff, 3) ||
		rtiff_check_bits(rtiff, 16) ||
		rtiff_check_interpretation(rtiff, PHOTOMETRIC_CIELAB))
		return -1;

	out->Bands = rtiff->header.samples_per_pixel;
	out->BandFmt = VIPS_FORMAT_SHORT;
	out->Coding = VIPS_CODING_NONE;
	out->Type = VIPS_INTERPRETATION_LABS;

	rtiff->sfn = rtiff_labs_line;

	return 0;
}

/* libtiff delivers logluv as illuminant-free 0-1 XYZ in 3 x float.
 */
static void
rtiff_logluv_line(Rtiff *rtiff, VipsPel *q, VipsPel *p, int n, void *dummy)
{
	int samples_per_pixel = rtiff->header.samples_per_pixel;

	float *p1;
	float *q1;
	int x;
	int i;

	p1 = (float *) p;
	q1 = (float *) q;
	for (x = 0; x < n; x++) {
		q1[0] = VIPS_D65_X0 * p1[0];
		q1[1] = VIPS_D65_Y0 * p1[1];
		q1[2] = VIPS_D65_Z0 * p1[2];

		for (i = 3; i < samples_per_pixel; i++)
			q1[i] = p1[i];

		q1 += samples_per_pixel;
		p1 += samples_per_pixel;
	}
}

/* LOGLUV images arrive from libtiff as float xyz.
 */
static int
rtiff_parse_logluv(Rtiff *rtiff, VipsImage *out)
{
	if (rtiff_check_min_samples(rtiff, 3) ||
		rtiff_check_interpretation(rtiff, PHOTOMETRIC_LOGLUV))
		return -1;

	out->Bands = rtiff->header.samples_per_pixel;
	out->BandFmt = VIPS_FORMAT_FLOAT;
	out->Coding = VIPS_CODING_NONE;
	out->Type = VIPS_INTERPRETATION_XYZ;

	rtiff->sfn = rtiff_logluv_line;

	return 0;
}

/* Make a N-bit scanline process function. We pass in the code to expand the
 * bits down the byte since this does not generalize well.
 */
#define NBIT_LINE(N, EXPAND) \
	static void \
		rtiff_##N##bit_line(Rtiff *rtiff, \
			VipsPel *q, VipsPel *p, int n, void *flg) \
	{ \
		int photometric = rtiff->header.photometric_interpretation; \
		int mask = photometric == PHOTOMETRIC_MINISBLACK ? 0 : 0xff; \
		int bps = rtiff->header.bits_per_sample; \
		int load = 8 / bps - 1; \
\
		int x; \
		VipsPel bits; \
\
		/* Stop a compiler warning. \
		 */ \
		bits = 0; \
\
		for (x = 0; x < n; x++) { \
			if ((x & load) == 0) \
				/* Flip the bits for miniswhite. \
				 */ \
				bits = *p++ ^ mask; \
\
			EXPAND(q[x], bits); \
\
			bits <<= bps; \
		} \
	}

/* Expand the top bit down a byte. Use a sign-extending shift.
 */
#define EXPAND1(Q, BITS) \
	G_STMT_START \
	{ \
		(Q) = (((signed char) (BITS & 128)) >> 7); \
	} \
	G_STMT_END

/* Expand the top two bits down a byte. Shift down, then expand up.
 */
#define EXPAND2(Q, BITS) \
	G_STMT_START \
	{ \
		VipsPel twobits = BITS >> 6; \
		VipsPel fourbits = twobits | (twobits << 2); \
		Q = fourbits | (fourbits << 4); \
	} \
	G_STMT_END

/* Expand the top four bits down a byte.
 */
#define EXPAND4(Q, BITS) \
	G_STMT_START \
	{ \
		Q = (BITS & 0xf0) | (BITS >> 4); \
	} \
	G_STMT_END

NBIT_LINE(1, EXPAND1)
NBIT_LINE(2, EXPAND2)
NBIT_LINE(4, EXPAND4)

/* Read a 1-bit TIFF image.
 */
static int
rtiff_parse_onebit(Rtiff *rtiff, VipsImage *out)
{
	if (rtiff_check_samples(rtiff, 1) ||
		rtiff_check_bits(rtiff, 1))
		return -1;

	out->Bands = 1;
	out->BandFmt = VIPS_FORMAT_UCHAR;
	out->Coding = VIPS_CODING_NONE;
	out->Type = VIPS_INTERPRETATION_B_W;

	rtiff->sfn = rtiff_1bit_line;

	return 0;
}

/* Read a 2-bit TIFF image.
 */
static int
rtiff_parse_twobit(Rtiff *rtiff, VipsImage *out)
{
	if (rtiff_check_samples(rtiff, 1) ||
		rtiff_check_bits(rtiff, 2))
		return -1;

	out->Bands = 1;
	out->BandFmt = VIPS_FORMAT_UCHAR;
	out->Coding = VIPS_CODING_NONE;
	out->Type = VIPS_INTERPRETATION_B_W;

	rtiff->sfn = rtiff_2bit_line;

	return 0;
}

/* Read a 4-bit TIFF image.
 */
static int
rtiff_parse_fourbit(Rtiff *rtiff, VipsImage *out)
{
	if (rtiff_check_samples(rtiff, 1) ||
		rtiff_check_bits(rtiff, 4))
		return -1;

	out->Bands = 1;
	out->BandFmt = VIPS_FORMAT_UCHAR;
	out->Coding = VIPS_CODING_NONE;
	out->Type = VIPS_INTERPRETATION_B_W;

	rtiff->sfn = rtiff_4bit_line;

	return 0;
}

/* Swap the sense of the first channel, if necessary.
 */
#define GREY_LOOP(TYPE, MAX) \
	{ \
		TYPE *p1; \
		TYPE *q1; \
\
		p1 = (TYPE *) p; \
		q1 = (TYPE *) q; \
		for (x = 0; x < n; x++) { \
			if (invert) \
				q1[0] = MAX - p1[0]; \
			else \
				q1[0] = p1[0]; \
\
			for (i = 1; i < samples_per_pixel; i++) \
				q1[i] = p1[i]; \
\
			q1 += samples_per_pixel; \
			p1 += samples_per_pixel; \
		} \
	}

/* GREY_LOOP implementation for 16-bit float
 */
#define GREY_LOOP_F16 \
	{ \
		gushort *p1; \
		float *q1; \
\
		p1 = (gushort *) p; \
		q1 = (float *) q; \
		for (x = 0; x < n; x++) { \
			if (invert) \
				q1[0] = 1.0 - half_2_float(p1[0]); \
			else \
				q1[0] = half_2_float(p1[0]); \
\
			for (i = 1; i < samples_per_pixel; i++) \
				q1[i] = half_2_float(p1[i]); \
\
			q1 += samples_per_pixel; \
			p1 += samples_per_pixel; \
		} \
	}

/* Per-scanline process function for greyscale images.
 */
static void
rtiff_greyscale_line(Rtiff *rtiff,
	VipsPel *q, VipsPel *p, int n, void *client)
{
	int samples_per_pixel = rtiff->header.samples_per_pixel;
	int bits_per_sample = rtiff->header.bits_per_sample;
	int photometric_interpretation =
		rtiff->header.photometric_interpretation;
	VipsBandFormat format = rtiff_guess_format(rtiff);

	/* Swapping black and white doesn't make sense for the signed formats.
	 */
	gboolean invert =
		photometric_interpretation == PHOTOMETRIC_MINISWHITE &&
		vips_band_format_isuint(format);

	int x, i;

	switch (format) {
	case VIPS_FORMAT_CHAR:
		GREY_LOOP(gchar, 0);
		break;

	case VIPS_FORMAT_UCHAR:
		GREY_LOOP(guchar, UCHAR_MAX);
		break;

	case VIPS_FORMAT_SHORT:
		GREY_LOOP(gshort, 0);
		break;

	case VIPS_FORMAT_USHORT:
		GREY_LOOP(gushort, USHRT_MAX);
		break;

	case VIPS_FORMAT_INT:
		GREY_LOOP(gint, 0);
		break;

	case VIPS_FORMAT_UINT:
		GREY_LOOP(guint, UINT_MAX);
		break;

	case VIPS_FORMAT_FLOAT:
		if (bits_per_sample == 16) {
			GREY_LOOP_F16;
		}
		else {
			GREY_LOOP(float, 1.0);
		}
		break;

	case VIPS_FORMAT_DOUBLE:
		GREY_LOOP(double, 1.0);
		break;

	default:
		g_assert_not_reached();
	}
}

/* Read a grey-scale TIFF image. We have to invert the first band if
 * PHOTOMETRIC_MINISBLACK is set.
 */
static int
rtiff_parse_greyscale(Rtiff *rtiff, VipsImage *out)
{
	if (rtiff_check_min_samples(rtiff, 1) ||
		rtiff_non_fractional(rtiff))
		return -1;

	out->Bands = rtiff->header.samples_per_pixel;
	out->BandFmt = rtiff_guess_format(rtiff);
	if (out->BandFmt == VIPS_FORMAT_NOTSET)
		return -1;
	out->Coding = VIPS_CODING_NONE;

	if (rtiff->header.bits_per_sample == 16)
		out->Type = VIPS_INTERPRETATION_GREY16;
	else
		out->Type = VIPS_INTERPRETATION_B_W;

	/* rtiff_greyscale_line() doesn't do complex.
	 */
	if (vips_check_noncomplex("tiff2vips", out))
		return -1;

	rtiff->sfn = rtiff_greyscale_line;

	return 0;
}

typedef struct {
	/* LUTs mapping image indexes to RGB.
	 */
	VipsPel *red8;
	VipsPel *green8;
	VipsPel *blue8;

	guint16 *red16;
	guint16 *green16;
	guint16 *blue16;

	/* All maps equal, so we write mono.
	 */
	gboolean mono;
} PaletteRead;

/* 1/2/4 bit samples with an 8-bit palette.
 */
static void
rtiff_palette_line_bit(Rtiff *rtiff,
	VipsPel *q, VipsPel *p, int n, void *client)
{
	PaletteRead *read = (PaletteRead *) client;
	int samples_per_pixel = rtiff->header.samples_per_pixel;
	int bits_per_sample = rtiff->header.bits_per_sample;

	int bit;
	VipsPel data;
	int x;

	bit = 0;
	data = 0;
	for (x = 0; x < n * samples_per_pixel; x++) {
		int i;

		if (bit <= 0) {
			data = *p++;
			bit = 8;
		}

		i = data >> (8 - bits_per_sample);
		data <<= bits_per_sample;
		bit -= bits_per_sample;

		/* The first band goes through the LUT, subsequent bands are
		 * left-justified and copied.
		 */
		if (x % samples_per_pixel == 0) {
			if (read->mono)
				*q++ = read->red8[i];
			else {
				q[0] = read->red8[i];
				q[1] = read->green8[i];
				q[2] = read->blue8[i];
				q += 3;
			}
		}
		else
			*q++ = VIPS_LSHIFT_INT(i, 8 - bits_per_sample);
	}
}

/* 8-bit samples with an 8-bit palette.
 */
static void
rtiff_palette_line8(Rtiff *rtiff, VipsPel *q, VipsPel *p, int n,
	void *client)
{
	PaletteRead *read = (PaletteRead *) client;
	int samples_per_pixel = rtiff->header.samples_per_pixel;

	int x;
	int s;

	for (x = 0; x < n; x++) {
		int i = p[0];

		if (read->mono)
			q[0] = read->red8[i];
		else {
			q[0] = read->red8[i];
			q[1] = read->green8[i];
			q[2] = read->blue8[i];
			q += 2;
		}

		for (s = 1; s < samples_per_pixel; s++)
			q[s] = p[s];

		q += samples_per_pixel;
		p += samples_per_pixel;
	}
}

/* 16-bit samples with 16-bit data in the palette.
 */
static void
rtiff_palette_line16(Rtiff *rtiff, VipsPel *q, VipsPel *p, int n,
	void *client)
{
	PaletteRead *read = (PaletteRead *) client;
	int samples_per_pixel = rtiff->header.samples_per_pixel;

	guint16 *p16, *q16;
	int x;
	int s;

	q16 = (guint16 *) q;
	p16 = (guint16 *) p;

	for (x = 0; x < n; x++) {
		int i = p16[0];

		if (read->mono)
			q16[0] = read->red16[i];
		else {
			q16[0] = read->red16[i];
			q16[1] = read->green16[i];
			q16[2] = read->blue16[i];
			q16 += 2;
		}

		for (s = 1; s < samples_per_pixel; s++)
			q16[s] = p16[s];

		q16 += samples_per_pixel;
		p16 += samples_per_pixel;
	}
}

/* Read a palette-ised TIFF image.
 */
static int
rtiff_parse_palette(Rtiff *rtiff, VipsImage *out)
{
	int samples_per_pixel = rtiff->header.samples_per_pixel;
	int bits_per_sample = rtiff->header.bits_per_sample;

	int len;
	PaletteRead *read;
	int i;

	if (rtiff_check_bits_palette(rtiff) ||
		rtiff_check_min_samples(rtiff, 1))
		return -1;
	len = 1 << bits_per_sample;

	if (!(read = VIPS_NEW(out, PaletteRead)) ||
		!(read->red8 = VIPS_ARRAY(out, len, VipsPel)) ||
		!(read->green8 = VIPS_ARRAY(out, len, VipsPel)) ||
		!(read->blue8 = VIPS_ARRAY(out, len, VipsPel)))
		return -1;

	/* Get maps, convert to 8-bit data.
	 */
	if (!TIFFGetField(rtiff->tiff,
			TIFFTAG_COLORMAP,
			&read->red16, &read->green16, &read->blue16)) {
		vips_error("tiff2vips", "%s", _("bad colormap"));
		return -1;
	}

	/* Old-style colourmaps were 8-bit. If all the top bytes are zero,
	 * assume we have one of these.
	 *
	 * See: https://github.com/libvips/libvips/issues/220
	 */
	for (i = 0; i < len; i++)
		if ((read->red16[i] >> 8) |
			(read->green16[i] >> 8) |
			(read->blue16[i] >> 8))
			break;
	if (i < len)
		for (i = 0; i < len; i++) {
			read->red8[i] = read->red16[i] >> 8;
			read->green8[i] = read->green16[i] >> 8;
			read->blue8[i] = read->blue16[i] >> 8;
		}
	else {
		g_warning("assuming 8-bit palette");

		for (i = 0; i < len; i++) {
			read->red8[i] = read->red16[i] & 0xff;
			read->green8[i] = read->green16[i] & 0xff;
			read->blue8[i] = read->blue16[i] & 0xff;
		}
	}

	/* Are all the maps equal? We have a mono image.
	 */
	read->mono = TRUE;
	for (i = 0; i < len; i++)
		if (read->red16[i] != read->green16[i] ||
			read->green16[i] != read->blue16[i]) {
			read->mono = FALSE;
			break;
		}

	/* There's a TIFF extension, INDEXED, that is the preferred way to
	 * encode mono palette images, but few applications support it. So we
	 * just search the colormap.
	 */

	if (bits_per_sample <= 8)
		out->BandFmt = VIPS_FORMAT_UCHAR;
	else
		out->BandFmt = VIPS_FORMAT_USHORT;
	out->Coding = VIPS_CODING_NONE;

	if (read->mono) {
		out->Bands = samples_per_pixel;
		if (bits_per_sample <= 8)
			out->Type = VIPS_INTERPRETATION_B_W;
		else
			out->Type = VIPS_INTERPRETATION_GREY16;
	}
	else {
		out->Bands = samples_per_pixel + 2;
		if (bits_per_sample <= 8)
			out->Type = VIPS_INTERPRETATION_sRGB;
		else
			out->Type = VIPS_INTERPRETATION_RGB16;
	}

	rtiff->client = read;
	if (bits_per_sample < 8)
		rtiff->sfn = rtiff_palette_line_bit;
	else if (bits_per_sample == 8)
		rtiff->sfn = rtiff_palette_line8;
	else if (bits_per_sample == 16)
		rtiff->sfn = rtiff_palette_line16;
	else
		g_assert_not_reached();

	return 0;
}

/* Per-scanline process function when we just need to copy.
 */
static void
rtiff_memcpy_line(Rtiff *rtiff, VipsPel *q, VipsPel *p, int n, void *client)
{
	VipsImage *im = (VipsImage *) client;
	size_t len = n * VIPS_IMAGE_SIZEOF_PEL(im);

	memcpy(q, p, len);
}

/* Per-scanline process function when we just need to copy.
 */
static void
rtiff_memcpy_f16_line(Rtiff *rtiff, VipsPel *q, VipsPel *p, int n, void *client)
{
	VipsImage *im = (VipsImage *) client;
	size_t len = (size_t) n * im->Bands;

	if (im->BandFmt == VIPS_FORMAT_COMPLEX ||
		im->BandFmt == VIPS_FORMAT_DPCOMPLEX)
		len *= 2;

	int i;

	gushort *restrict hp = (gushort *) p;
	float *restrict fq = (float *) q;

	for (i = 0; i < len; i++)
		fq[i] = half_2_float(hp[i]);
}

/* Read a regular multiband image where we can just copy pixels from the tiff
 * buffer.
 */
static int
rtiff_parse_copy(Rtiff *rtiff, VipsImage *out)
{
	int samples_per_pixel = rtiff->header.samples_per_pixel;
	int photometric_interpretation =
		rtiff->header.photometric_interpretation;
	int bits_per_sample = rtiff->header.bits_per_sample;
	int sample_format = rtiff->header.sample_format;
	int inkset = rtiff->header.inkset;

	if (rtiff_non_fractional(rtiff))
		return -1;

	out->Bands = samples_per_pixel;
	out->BandFmt = rtiff_guess_format(rtiff);
	if (out->BandFmt == VIPS_FORMAT_NOTSET)
		return -1;
	out->Coding = VIPS_CODING_NONE;

	if (samples_per_pixel >= 3 &&
		(photometric_interpretation == PHOTOMETRIC_RGB ||
		photometric_interpretation == PHOTOMETRIC_YCBCR)) {
		if (out->BandFmt == VIPS_FORMAT_USHORT)
			out->Type = VIPS_INTERPRETATION_RGB16;
		else if (!vips_band_format_isint(out->BandFmt))
			/* Most float images use 0 - 1 for black - white.
			 * Photoshop uses 0 - 1 and no gamma.
			 */
			out->Type = VIPS_INTERPRETATION_scRGB;
		else
			out->Type = VIPS_INTERPRETATION_sRGB;
	}
	else if (samples_per_pixel >= 3 &&
		photometric_interpretation == PHOTOMETRIC_CIELAB)
		out->Type = VIPS_INTERPRETATION_LAB;
	else if (photometric_interpretation == PHOTOMETRIC_SEPARATED &&
		samples_per_pixel >= 4 &&
		inkset == INKSET_CMYK)
		out->Type = VIPS_INTERPRETATION_CMYK;
	else
		out->Type = VIPS_INTERPRETATION_MULTIBAND;

	rtiff->client = out;

	if (bits_per_sample == 16 && sample_format == SAMPLEFORMAT_IEEEFP) {
		rtiff->sfn = rtiff_memcpy_f16_line;
	}
	else {
		rtiff->sfn = rtiff_memcpy_line;

		/* We expand YCBCR images to RGB using JPEGCOLORMODE_RGB, and this
		 * means we need a slightly larger read buffer for the edge pixels. In
		 * turn, this means we can't just memcpy to libvips regions.
		 */
		rtiff->memcpy = photometric_interpretation != PHOTOMETRIC_YCBCR;
	}

	return 0;
}

/* Read an image as RGBA using TIFFRGBAImage
 */
static int
rtiff_parse_rgba(Rtiff *rtiff, VipsImage *out)
{
	out->Bands = 4;
	out->Type = VIPS_INTERPRETATION_sRGB;
	out->BandFmt = VIPS_FORMAT_UCHAR;
	out->Coding = VIPS_CODING_NONE;

	rtiff->client = out;

	/* We'll have RGBA areas of exact size as we need, so we can just copy it
	 */
	rtiff->sfn = rtiff_memcpy_line;
	rtiff->memcpy = TRUE;

	return 0;
}

typedef int (*reader_fn)(Rtiff *rtiff, VipsImage *out);

/* We have a range of output paths. Look at the tiff header and try to
 * route the input image to the best output path.
 */
static reader_fn
rtiff_pick_reader(Rtiff *rtiff)
{
	int bits_per_sample = rtiff->header.bits_per_sample;
	int photometric_interpretation =
		rtiff->header.photometric_interpretation;
	int samples_per_pixel = rtiff->header.samples_per_pixel;
	int read_as_rgba = rtiff->header.read_as_rgba;

	if (read_as_rgba)
		return rtiff_parse_rgba;

	if (photometric_interpretation == PHOTOMETRIC_CIELAB) {
		if (bits_per_sample == 8) {
			if (samples_per_pixel > 3)
				return rtiff_parse_lab_with_alpha;
			else
				return rtiff_parse_labpack;
		}
		if (bits_per_sample == 16)
			return rtiff_parse_labs;
	}

	if (photometric_interpretation == PHOTOMETRIC_LOGLUV)
		return rtiff_parse_logluv;

	if (photometric_interpretation == PHOTOMETRIC_MINISWHITE ||
		photometric_interpretation == PHOTOMETRIC_MINISBLACK) {

		if (bits_per_sample == 1)
			return rtiff_parse_onebit;
		else if (bits_per_sample == 2)
			return rtiff_parse_twobit;
		else if (bits_per_sample == 4)
			return rtiff_parse_fourbit;
		else
			return rtiff_parse_greyscale;
	}

	if (photometric_interpretation == PHOTOMETRIC_PALETTE)
		return rtiff_parse_palette;

	return rtiff_parse_copy;
}

/* Set the header on @out from our rtiff. rtiff_header_read() has already been
 * called.
 */
static int
rtiff_set_header(Rtiff *rtiff, VipsImage *out)
{
	guint32 data_len;
	void *data;

	rtiff_set_decode_format(rtiff);

	if (rtiff->header.photometric_interpretation == PHOTOMETRIC_LOGLUV)
		vips_image_set_double(out, "stonits", rtiff->header.stonits);

	out->Xsize = rtiff->header.width;
	out->Ysize = rtiff->header.height * rtiff->n;

	VIPS_SETSTR(out->filename,
		vips_connection_filename(VIPS_CONNECTION(rtiff->source)));

	if (rtiff->n > 1)
		vips_image_set_int(out, VIPS_META_PAGE_HEIGHT, rtiff->header.height);

	if (rtiff->header.subifd_count > 0)
		vips_image_set_int(out,
			VIPS_META_N_SUBIFDS, rtiff->header.subifd_count);

	vips_image_set_int(out, VIPS_META_N_PAGES, rtiff->n_pages);

	/* We have a range of output paths. Look at the tiff header and try to
	 * route the input image to the best output path.
	 */
	if (rtiff_pick_reader(rtiff)(rtiff, out))
		return -1;

	/* Read any ICC profile.
	 */
	if (TIFFGetField(rtiff->tiff, TIFFTAG_ICCPROFILE, &data_len, &data))
		vips_image_set_blob_copy(out, VIPS_META_ICC_NAME, data, data_len);

	/* Read any XMP metadata.
	 */
	if (TIFFGetField(rtiff->tiff, TIFFTAG_XMLPACKET, &data_len, &data))
		vips_image_set_blob_copy(out, VIPS_META_XMP_NAME, data, data_len);

	/* Read any IPTC metadata.
	 */
	if (TIFFGetField(rtiff->tiff, TIFFTAG_RICHTIFFIPTC, &data_len, &data)) {
		vips_image_set_blob_copy(out, VIPS_META_IPTC_NAME, data, data_len);

		/* Older versions of libvips used this misspelt name :-( attach
		 * under this name too for compatibility.
		 */
		vips_image_set_blob_copy(out, "ipct-data", data, data_len);
	}

	/* Read any photoshop metadata.
	 */
	if (TIFFGetField(rtiff->tiff, TIFFTAG_PHOTOSHOP, &data_len, &data))
		vips_image_set_blob_copy(out, VIPS_META_PHOTOSHOP_NAME, data, data_len);

	if (rtiff->header.image_description)
		vips_image_set_string(out, VIPS_META_IMAGEDESCRIPTION,
			rtiff->header.image_description);

	if (get_resolution(rtiff->tiff, out))
		return -1;

	vips_image_set_int(out, VIPS_META_BITS_PER_SAMPLE,
		rtiff->header.bits_per_sample);

	/* Set the "orientation" tag. This is picked up later by autorot, if
	 * requested.
	 */
	vips_image_set_int(out, VIPS_META_ORIENTATION, rtiff->header.orientation);

	/* Hint smalltile for tiled images, since we may be decompressing
	 * outside the lock and THINSTRIP would prevent parallel tile decode.
	 */
	vips_image_pipelinev(out,
		rtiff->header.tiled
			? VIPS_DEMAND_STYLE_SMALLTILE
			: VIPS_DEMAND_STYLE_THINSTRIP,
		NULL);

	return 0;
}

/* Tilewise read sequence value.
 */
typedef struct _RtiffSeq {
	Rtiff *rtiff;

	/* Decompressed tile here.
	 */
	tdata_t *buf;

	/* If we are decompressing, we need a buffer to read the raw tile to
	 * before running the decompressor. This needs to be per-thread, since
	 * we decompress in parallel.
	 */
	tdata_t compressed_buf;
	tsize_t compressed_buf_length;
} RtiffSeq;

/* Allocate a tile buffer. Have one of these for each thread so we can unpack
 * to vips in parallel.
 */
static void *
rtiff_seq_start(VipsImage *out, void *a, void *b)
{
	Rtiff *rtiff = (Rtiff *) a;
	RtiffSeq *seq;

	if (!(seq = VIPS_NEW(out, RtiffSeq)))
		return NULL;
	seq->rtiff = rtiff;
	if (!(seq->buf = vips_malloc(NULL, rtiff->header.tile_size)))
		return NULL;

	/* If we will be decompressing, we need a buffer large enough to hold
	 * the largest compressed tile in any page.
	 *
	 * Allocate a buffer 2x the uncompressed tile size ... much simpler
	 * than searching every page for the largest tile with
	 * TIFFTAG_TILEBYTECOUNTS.
	 */
	if (rtiff->header.we_decompress) {
		seq->compressed_buf_length = 2 * rtiff->header.tile_size;
		if (!(seq->compressed_buf = VIPS_MALLOC(NULL,
				  seq->compressed_buf_length)))
			return NULL;
	}

	return (void *) seq;
}

#ifdef HAVE_JPEG
static void
rtiff_decompress_jpeg_init_source(j_decompress_ptr cinfo)
{
	/* Nothing.
	 */
}

static boolean
rtiff_decompress_jpeg_fill_input_buffer(j_decompress_ptr cinfo)
{
	static const JOCTET mybuffer[4] = {
		(JOCTET) 0xFF, (JOCTET) JPEG_EOI, 0, 0
	};

	/* The whole JPEG data is expected to reside in the supplied memory
	 * buffer, so any request for more data beyond the given buffer size
	 * is treated as an error.
	 */
	WARNMS(cinfo, JWRN_VIPS_IMAGE_EOF);

	/* Insert a fake EOI marker
	 */
	cinfo->src->next_input_byte = mybuffer;
	cinfo->src->bytes_in_buffer = 2;

	return TRUE;
}

/* Skip data -- used to skip over a potentially large amount of
 * uninteresting data (such as an APPn marker).
 *
 * Writers of suspendable-input applications must note that skip_input_data
 * is not granted the right to give a suspension return.  If the skip extends
 * beyond the data currently in the buffer, the buffer can be marked empty so
 * that the next read will cause a fill_input_buffer call that can suspend.
 * Arranging for additional bytes to be discarded before reloading the input
 * buffer is the application writer's problem.
 */

static void
rtiff_decompress_jpeg_skip_input_data(j_decompress_ptr cinfo, long num_bytes)
{
	struct jpeg_source_mgr *src = cinfo->src;

	/* Just a dumb implementation for now.  Could use fseek() except
	 * it doesn't work on pipes.  Not clear that being smart is worth
	 * any trouble anyway -- large skips are infrequent.
	 */
	if (num_bytes > 0) {
		while (num_bytes > (long) src->bytes_in_buffer) {
			num_bytes -= (long) src->bytes_in_buffer;
			(void) (*src->fill_input_buffer)(cinfo);
			/* note we assume that fill_input_buffer will never
			 * return FALSE, so suspension need not be handled.
			 */
		}

		src->next_input_byte += (size_t) num_bytes;
		src->bytes_in_buffer -= (size_t) num_bytes;
	}
}

static void
rtiff_decompress_jpeg_set_memory(j_decompress_ptr cinfo,
	void *data, size_t data_len)
{
	if (!cinfo->src)
		cinfo->src =
			(struct jpeg_source_mgr *) (*cinfo->mem->alloc_small)(
				(j_common_ptr) cinfo,
				JPOOL_PERMANENT,
				sizeof(struct jpeg_source_mgr));

	/* Present the whole of data as one chunk.
	 */
	cinfo->src->bytes_in_buffer = data_len;
	cinfo->src->next_input_byte = (JOCTET *) data;
	cinfo->src->init_source = rtiff_decompress_jpeg_init_source;
	cinfo->src->fill_input_buffer = rtiff_decompress_jpeg_fill_input_buffer;
	cinfo->src->skip_input_data = rtiff_decompress_jpeg_skip_input_data;
	cinfo->src->resync_to_restart = jpeg_resync_to_restart;
}

static int
rtiff_decompress_jpeg_run(Rtiff *rtiff, j_decompress_ptr cinfo,
	void *data, size_t data_len, void *out)
{
	void *tables;
	uint32_t tables_len;
	int bytes_per_pixel;
	size_t bytes_per_scanline;
	VipsPel *q;
	int y;

#ifdef DEBUG_VERBOSE
	printf("rtiff_decompress_jpeg_run: decompressing %zd bytes of jpg\n",
		data_len);
#endif /*DEBUG_VERBOSE*/

	/* Tables are optional.
	 */
	tables = NULL;
	tables_len = 0;
	(void) TIFFGetField(rtiff->tiff,
		TIFFTAG_JPEGTABLES, &tables_len, &tables);

	if (tables) {
		rtiff_decompress_jpeg_set_memory(cinfo, tables, tables_len);
		if (jpeg_read_header(cinfo, FALSE) !=
			JPEG_HEADER_TABLES_ONLY)
			return -1;
	}

	rtiff_decompress_jpeg_set_memory(cinfo, data, data_len);

	if (jpeg_read_header(cinfo, TRUE) != JPEG_HEADER_OK)
		return -1;

	/* This isn't stored in the tile -- we have to set it from the
	 * enclosing TIFF.
	 */
	switch (rtiff->header.photometric_interpretation) {
	case PHOTOMETRIC_SEPARATED:
		cinfo->jpeg_color_space = JCS_CMYK;
		bytes_per_pixel = 4;
		break;

	case PHOTOMETRIC_YCBCR:
		cinfo->jpeg_color_space = JCS_YCbCr;
		bytes_per_pixel = 3;
		break;

	case PHOTOMETRIC_RGB:
	case PHOTOMETRIC_CIELAB:
		// RGB-compressed CIELAB is a possibility, amazingly
		cinfo->jpeg_color_space = JCS_RGB;
		bytes_per_pixel = 3;
		break;

	case PHOTOMETRIC_MINISWHITE:
	case PHOTOMETRIC_MINISBLACK:
		cinfo->jpeg_color_space = JCS_GRAYSCALE;
		bytes_per_pixel = 1;
		break;

	default:
		cinfo->jpeg_color_space = JCS_UNKNOWN;
		bytes_per_pixel = 1;
		break;
	}

	jpeg_calc_output_dimensions(cinfo);
	bytes_per_scanline = (size_t) cinfo->output_width * bytes_per_pixel;

	/* Double-check tile dimensions.
	 */
	if (cinfo->output_width > rtiff->header.tile_width ||
		cinfo->output_height > rtiff->header.tile_height ||
		bytes_per_scanline > rtiff->header.tile_row_size)
		return -1;

	jpeg_start_decompress(cinfo);

	q = (VipsPel *) out;
	for (y = 0; y < cinfo->output_height; y++) {
		JSAMPROW row_pointer[1];

		row_pointer[0] = (JSAMPLE *) q;
		jpeg_read_scanlines(cinfo, &row_pointer[0], 1);
		q += bytes_per_scanline;
	}

	return 0;
}

static void
rtiff_decompress_jpeg_emit_message(j_common_ptr cinfo, int msg_level)
{
	if (msg_level < 0) {
		long num_warnings;

		/* Always count warnings in num_warnings.
		 */
		num_warnings = ++cinfo->err->num_warnings;

		/* Corrupt files may give many warnings, the policy here is to
		 * show only the first warning and treat many warnings as fatal,
		 * unless unlimited is set.
		 */
		if (num_warnings == 1)
			(*cinfo->err->output_message)(cinfo);
	}
	else if (cinfo->err->trace_level >= msg_level)
		/* It's a trace message. Show it if trace_level >= msg_level.
		 */
		(*cinfo->err->output_message)(cinfo);
}

/* Decompress a tile of size coefficients into out.
 */
static int
rtiff_decompress_jpeg(Rtiff *rtiff, void *data, size_t data_len, void *out)
{
	struct jpeg_decompress_struct cinfo = { 0 };
	ErrorManager eman;

	if (setjmp(eman.jmp) == 0) {
		cinfo.err = jpeg_std_error(&eman.pub);
		cinfo.err->addon_message_table = vips__jpeg_message_table;
		cinfo.err->first_addon_message = 1000;
		cinfo.err->last_addon_message = 1001;
		eman.pub.error_exit = vips__new_error_exit;
		eman.pub.emit_message = rtiff_decompress_jpeg_emit_message;
		eman.pub.output_message = vips__new_output_message;
		eman.fp = NULL;

		jpeg_create_decompress(&cinfo);

		if (rtiff_decompress_jpeg_run(rtiff, &cinfo,
				data, data_len, out)) {
			jpeg_destroy_decompress(&cinfo);
			return -1;
		}
	}
	else {
#ifdef DEBUG_VERBOSE
		printf("rtiff_decompress_jpeg: error return\n");
#endif /*DEBUG_VERBOSE*/

		jpeg_destroy_decompress(&cinfo);
		return -1;
	}

	jpeg_destroy_decompress(&cinfo);

	return 0;
}
#endif /*HAVE_JPEG*/

static int
rtiff_decompress_tile(Rtiff *rtiff, tdata_t *in, tsize_t size, tdata_t *out)
{
	g_assert(rtiff->header.we_decompress);

	switch (rtiff->header.compression) {
	case JP2K_YCC:
	case JP2K_RGB:
	case JP2K_LOSSY:
		if (vips__foreign_load_jp2k_decompress(
				rtiff->out,
				rtiff->header.tile_width,
				rtiff->header.tile_height,
				TRUE,
				in, size,
				out, rtiff->header.tile_size))
			return -1;
		break;

#ifdef HAVE_JPEG
	case COMPRESSION_JPEG:
		if (rtiff_decompress_jpeg(rtiff, in, size, out))
			return -1;
		break;
#endif /*HAVE_JPEG*/

	default:
		g_assert_not_reached();
		break;
	}

	return 0;
}

/* Decompress a tile to RGBA
 */
static int
rtiff_read_rgba_tile(Rtiff *rtiff, int x, int y, tdata_t *buf)
{
	guint32 *u32_buf = (guint32 *) buf;

	if (!TIFFReadRGBATile(rtiff->tiff, x, y, u32_buf))
		return -1;

	/* For some reason TIFFReadRGBATile decodes tiles upside down,
	 * so we need to flip them.
	 */
	guint32 tile_width = rtiff->header.tile_width;
	guint32 tile_height = rtiff->header.tile_height;

	guint32 *up = u32_buf;
	guint32 *down = u32_buf + (tile_height - 1) * tile_width;
	for (int yy = 0; yy < tile_height / 2; yy++) {
		for (int xx = 0; xx < tile_width; xx++)
			VIPS_SWAP(guint32, up[xx], down[xx]);

		up += tile_width;
		down -= tile_width;
	}

	return 0;
}

/* Select a page and decompress a tile. This has to be a single operation,
 * since it changes the current page number in TIFF.
 */
static int
rtiff_read_tile(RtiffSeq *seq, tdata_t *buf, int page, int x, int y)
{
	Rtiff *rtiff = seq->rtiff;

	tsize_t size;

#ifdef DEBUG_VERBOSE
	printf("rtiff_read_tile: page = %d, x = %d, y = %d, "
		   "we_decompress = %d\n",
		page, x, y, rtiff->header.we_decompress);
#endif /*DEBUG_VERBOSE*/

	/* Compressed tiles load to compressed_buf.
	 */
	if (rtiff->header.we_decompress) {
		ttile_t tile_no;

		g_rec_mutex_lock(&rtiff->lock);

		if (rtiff_set_page(rtiff, page)) {
			g_rec_mutex_unlock(&rtiff->lock);
			return -1;
		}

		tile_no = TIFFComputeTile(rtiff->tiff, x, y, 0, 0);

		size = TIFFReadRawTile(rtiff->tiff, tile_no,
			seq->compressed_buf, seq->compressed_buf_length);
		if (size <= 0) {
			vips_foreign_load_invalidate(rtiff->out);
			g_rec_mutex_unlock(&rtiff->lock);
			return -1;
		}

		g_rec_mutex_unlock(&rtiff->lock);

		/* Decompress outside the lock, so we get parallelism.
		 */
		if (rtiff_decompress_tile(rtiff, seq->compressed_buf, size, buf)) {
			vips_error("tiff2vips", _("decompress error tile %d x %d"), x, y);
			return -1;
		}
	}
	else {
		g_rec_mutex_lock(&rtiff->lock);

		if (rtiff_set_page(rtiff, page)) {
			g_rec_mutex_unlock(&rtiff->lock);
			return -1;
		}

		int result;
		if (rtiff->header.read_as_rgba)
			result = rtiff_read_rgba_tile(rtiff, x, y, buf);
		else
			result = TIFFReadTile(rtiff->tiff, buf, x, y, 0, 0) < 0;
		if (result && rtiff->fail_on >= VIPS_FAIL_ON_WARNING) {
			vips_foreign_load_invalidate(rtiff->out);
			g_rec_mutex_unlock(&rtiff->lock);
			return -1;
		}

		g_rec_mutex_unlock(&rtiff->lock);
	}

	return 0;
}

/* Paint a tile from the file. This is a
 * special-case for when a region is exactly a tiff tile, and pixels need no
 * conversion. In this case, libtiff can read tiles directly to our output
 * region.
 */
static int
rtiff_fill_region_aligned(VipsRegion *out,
	void *vseq, void *a, void *b, gboolean *stop)
{
	RtiffSeq *seq = (RtiffSeq *) vseq;
	Rtiff *rtiff = (Rtiff *) a;
	VipsRect *r = &out->valid;
	int page_height = rtiff->header.height;
	int page_no = r->top / page_height;
	int page_y = r->top % page_height;

	g_assert((r->left % rtiff->header.tile_width) == 0);
	g_assert((r->top % rtiff->header.tile_height) == 0);
	g_assert(r->width == rtiff->header.tile_width);
	g_assert(r->height == rtiff->header.tile_height);
	g_assert(VIPS_REGION_LSKIP(out) == VIPS_REGION_SIZEOF_LINE(out));

#ifdef DEBUG_VERBOSE
	printf("rtiff_fill_region_aligned:\n");
#endif /*DEBUG_VERBOSE*/

	/* Read that tile directly into the vips tile.
	 */
	if (rtiff_read_tile(seq,
			(tdata_t *) VIPS_REGION_ADDR(out, r->left, r->top),
			rtiff->page + page_no, r->left, page_y))
		return -1;

	return 0;
}

/* Loop over the output region, painting in tiles from the file.
 */
static int
rtiff_fill_region_unaligned(VipsRegion *out,
	void *vseq, void *a, void *b, gboolean *stop)
{
	RtiffSeq *seq = (RtiffSeq *) vseq;
	Rtiff *rtiff = (Rtiff *) a;
	int tile_width = rtiff->header.tile_width;
	int tile_height = rtiff->header.tile_height;
	int page_height = rtiff->header.height;
	int tile_row_size = rtiff->header.tile_row_size;
	VipsRect *r = &out->valid;

	int x, y, z;

#ifdef DEBUG_VERBOSE
	printf("rtiff_fill_region_unaligned:\n");
#endif /*DEBUG_VERBOSE*/

	y = 0;
	while (y < r->height) {
		VipsRect tile, page, hit;

		/* Not necessary, but it stops static analyzers complaining
		 * about a used-before-set.
		 */
		hit.height = 0;

		x = 0;
		while (x < r->width) {
			/* page_no is within this toilet roll image, not tiff
			 * file page number ... add the number of the start
			 * page to get that.
			 */
			int page_no = (r->top + y) / page_height;
			int page_y = (r->top + y) % page_height;

			/* Coordinate of the tile on this page that xy falls in.
			 */
			int xs = ((r->left + x) / tile_width) * tile_width;
			int ys = (page_y / tile_height) * tile_height;

			if (rtiff_read_tile(seq,
					seq->buf, rtiff->page + page_no, xs, ys))
				return -1;

			/* Position of tile on the page.
			 */
			tile.left = xs;
			tile.top = ys;
			tile.width = tile_width;
			tile.height = tile_height;

			/* It'll be clipped by this page.
			 */
			page.left = 0;
			page.top = 0;
			page.width = rtiff->header.width;
			page.height = rtiff->header.height;
			vips_rect_intersectrect(&tile, &page, &tile);

			/* To image coordinates.
			 */
			tile.top += page_no * page_height;

			/* And clip again by this region.
			 */
			vips_rect_intersectrect(&tile, r, &hit);

			/* We are inside a tilecache, so requests will always
			 * be aligned left-right to tile boundaries.
			 *
			 * this is not true vertically for toilet-roll images.
			 */
			g_assert(hit.left == tile.left);

			/* Unpack to VIPS format.
			 * Just unpack the section of the tile we need.
			 */
			for (z = 0; z < hit.height; z++) {
				VipsPel *p = (VipsPel *) seq->buf +
					(hit.top - tile.top + z) *
						tile_row_size;
				VipsPel *q = VIPS_REGION_ADDR(out,
					hit.left, hit.top + z);

				rtiff->sfn(rtiff,
					q, p, hit.width, rtiff->client);
			}

			x += hit.width;
		}

		/* This will be the same for all tiles in the row we've just
		 * done.
		 */
		y += hit.height;
	}

	return 0;
}

/* Loop over the output region, painting in tiles from the file.
 */
static int
rtiff_fill_region(VipsRegion *out,
	void *vseq, void *a, void *b, gboolean *stop)
{
	Rtiff *rtiff = (Rtiff *) a;
	int tile_width = rtiff->header.tile_width;
	int tile_height = rtiff->header.tile_height;
	int page_width = rtiff->header.width;
	int page_height = rtiff->header.height;
	VipsRect *r = &out->valid;
	int page_no = r->top / page_height;
	int page_y = r->top % page_height;

	VipsGenerateFn generate;

#ifdef DEBUG_VERBOSE
	printf("rtiff_fill_region: left = %d, top = %d, "
		   "width = %d, height = %d\n",
		r->left, r->top, r->width, r->height);
#endif /*DEBUG_VERBOSE*/

	/* Special case: we are filling a single cache tile exactly sized to
	 * match the tiff tile, and we have no repacking to do for this format.
	 *
	 * If we are not on the first page, pages must be a multiple of the
	 * tile size of we'll miss alignment.
	 */
	if ((page_no == 0 || page_height % tile_height == 0) &&
		r->left % tile_width == 0 &&
		r->top % tile_height == 0 &&
		r->width == tile_width &&
		r->height == tile_height &&
		r->left + tile_width <= page_width &&
		page_y + tile_height <= page_height &&
		VIPS_REGION_LSKIP(out) == VIPS_REGION_SIZEOF_LINE(out) &&
		rtiff->memcpy)
		generate = rtiff_fill_region_aligned;
	else
		generate = rtiff_fill_region_unaligned;

	VIPS_GATE_START("rtiff_fill_region: work");

	if (generate(out, vseq, a, b, stop)) {
		VIPS_GATE_STOP("rtiff_fill_region: work");
		return -1;
	}

	VIPS_GATE_STOP("rtiff_fill_region: work");

	return 0;
}

static int
rtiff_seq_stop(void *vseq, void *a, void *b)
{
	RtiffSeq *seq = (RtiffSeq *) vseq;

	VIPS_FREE(seq->buf);
	VIPS_FREE(seq->compressed_buf);

	return 0;
}

/* Unpremultiply associative alpha, if any.
 */
static int
rtiff_unpremultiply(Rtiff *rtiff, VipsImage *in, VipsImage **out)
{
	if (rtiff->header.alpha_band != -1) {
		VipsImage *x;

		if (
			vips_unpremultiply(in, &x,
				"alpha_band", rtiff->header.alpha_band,
				NULL) ||
			vips_cast(x, out, in->BandFmt, NULL)) {
			g_object_unref(x);
			return -1;
		}
		g_object_unref(x);
	}
	else {
		*out = in;
		g_object_ref(in);
	}

	return 0;
}

/* Tile-type TIFF reader core - pass in a per-tile transform. Generate into
 * the im and do it all partially.
 */
static int
rtiff_read_tilewise(Rtiff *rtiff, VipsImage *out)
{
	int tile_width = rtiff->header.tile_width;
	int tile_height = rtiff->header.tile_height;
	VipsImage **t = (VipsImage **) vips_object_local_array(VIPS_OBJECT(out), 4);

	VipsImage *in;

#ifdef DEBUG
	printf("tiff2vips: rtiff_read_tilewise\n");
#endif /*DEBUG*/

	/* I don't have a sample images for tiled + separate, ban it for now.
	 */
	if (rtiff->header.separate) {
		vips_error("tiff2vips", "%s", _("tiled separate planes not supported"));
		return -1;
	}

	/* Read to this image, then cache to out, see below.
	 */
	t[0] = vips_image_new();

	if (rtiff_set_header(rtiff, t[0]))
		return -1;

	/* Double check: in memcpy mode, the vips tilesize should exactly
	 * match the tifftile size.
	 */
	if (rtiff->memcpy) {
		size_t vips_tile_size = VIPS_IMAGE_SIZEOF_PEL(t[0]) *
			tile_width * tile_height;

		if (rtiff->header.tile_size != vips_tile_size) {
			vips_error("tiff2vips", "%s", _("unsupported tiff image type"));
			return -1;
		}
	}

	/* Generate to out, adding a cache. Enough tiles for two complete rows.
	 * Set "threaded", so we allow many tiles to be read at once. We lock
	 * around each tile read.
	 */
	if (
		vips_image_generate(t[0],
			rtiff_seq_start, rtiff_fill_region, rtiff_seq_stop,
			rtiff, NULL) ||
		vips_tilecache(t[0], &t[1],
			"tile_width", tile_width,
			"tile_height", tile_height,
			"max_tiles", 2 * (1 + t[0]->Xsize / tile_width),
			"threaded", TRUE,
			NULL) ||
		rtiff_unpremultiply(rtiff, t[1], &t[2]))
		return -1;
	in = t[2];

	/* Only do this if we have to.
	 */
	if (rtiff->autorotate &&
		vips_image_get_orientation(in) != 1) {
		if (vips_autorot(in, &t[3], NULL))
			return -1;
		in = t[3];
	}

	if (vips_image_write(in, out))
		return -1;

	return 0;
}

/* Read a strip from a page. If the image is in separate planes, read each
 * plane and interleave to the output.
 *
 * No need to lock -- this is inside a sequential.
 */
static int
rtiff_strip_read_interleaved(Rtiff *rtiff,
	int page, tstrip_t strip, tdata_t buf)
{
	int samples_per_pixel = rtiff->header.samples_per_pixel;
	int read_height = rtiff->header.read_height;
	int bits_per_sample = rtiff->header.bits_per_sample;
	int read_as_rgba = rtiff->header.read_as_rgba;
	int strip_y = strip * read_height;

	if (rtiff_set_page(rtiff, page))
		return -1;

	if (read_as_rgba) {
		if (rtiff_rgba_strip_read(rtiff, strip, buf))
			return -1;
	}
	else if (rtiff->header.separate) {
		int page_width = rtiff->header.width;
		int page_height = rtiff->header.height;
		int strips_per_plane = 1 + (page_height - 1) / read_height;
		int strip_height = VIPS_MIN(read_height,
			page_height - strip_y);
		int pels_per_strip = page_width * strip_height;
		int bytes_per_sample = bits_per_sample >> 3;

		int i, j, k;

		for (i = 0; i < samples_per_pixel; i++) {
			VipsPel *p;
			VipsPel *q;

			if (rtiff_strip_read(rtiff,
					strips_per_plane * i + strip,
					rtiff->plane_buf))
				return -1;

			p = (VipsPel *) rtiff->plane_buf;
			q = i * bytes_per_sample + (VipsPel *) buf;
			for (j = 0; j < pels_per_strip; j++) {
				for (k = 0; k < bytes_per_sample; k++)
					q[k] = p[k];

				p += bytes_per_sample;
				q += bytes_per_sample * samples_per_pixel;
			}
		}
	}
	else {
		if (rtiff_strip_read(rtiff, strip, buf))
			return -1;
	}

	return 0;
}

static int
rtiff_stripwise_generate(VipsRegion *out_region,
	void *seq, void *a, void *b, gboolean *stop)
{
	VipsImage *out = out_region->im;
	Rtiff *rtiff = (Rtiff *) a;
	int read_height = rtiff->header.read_height;
	int page_height = rtiff->header.height;
	tsize_t scanline_size = rtiff->header.scanline_size;
	VipsRect *r = &out_region->valid;

	int y;

#ifdef DEBUG_VERBOSE
	printf("rtiff_stripwise_generate: top = %d, height = %d\n",
		r->top, r->height);
	printf("rtiff_stripwise_generate: y_top = %d\n", rtiff->y_pos);
#endif /*DEBUG_VERBOSE*/

	/* We're inside a tilecache where tiles are the full image width, so
	 * this should always be true.
	 */
	g_assert(r->left == 0);
	g_assert(r->width == out_region->im->Xsize);
	g_assert(VIPS_RECT_BOTTOM(r) <= out_region->im->Ysize);

	/* If we're reading more than one page, tiles won't fall on strip
	 * boundaries. Tiles may be contain several strips.
	 */

	/* Check that y_pos is correct. It should be, since we are inside
	 * a vips_sequential().
	 */
	if (r->top != rtiff->y_pos) {
		vips_error("tiff2vips",
			_("out of order read -- at line %d, but line %d requested"),
			rtiff->y_pos, r->top);
		return -1;
	}

	VIPS_GATE_START("rtiff_stripwise_generate: work");

	y = 0;
	while (y < r->height) {
		/* page_no is within this toilet roll image, not tiff
		 * file page number ... add the number of the start
		 * page to get that.
		 */
		int page_no = (r->top + y) / page_height;
		int y_page = (r->top + y) % page_height;

		/* Strip number.
		 */
		tstrip_t strip_no = y_page / read_height;

		VipsRect image, page, strip, hit;

		/* Our four (including the output region) rects, all in
		 * output image coordinates.
		 */
		image.left = 0;
		image.top = 0;
		image.width = out->Xsize;
		image.height = out->Ysize;

		page.left = 0;
		page.top = page_height * ((r->top + y) / page_height);
		page.width = out->Xsize;
		page.height = page_height;

		strip.left = 0;
		strip.top = page.top + strip_no * read_height;
		strip.width = out->Xsize;
		strip.height = read_height;

		/* Clip strip against page and image ... the final strip will
		 * be smaller.
		 */
		vips_rect_intersectrect(&strip, &image, &strip);
		vips_rect_intersectrect(&strip, &page, &strip);

		/* Now the bit that overlaps with the region we are filling.
		 */
		vips_rect_intersectrect(&strip, r, &hit);

		g_assert(hit.height > 0);

		/* Read directly into the image if we can. Otherwise, we must
		 * read to a temp buffer then unpack into the image.
		 *
		 * We need to read via a buffer if we need to reformat pixels,
		 * or if this strip is not aligned on a tile boundary.
		 */
		if (rtiff->memcpy &&
			hit.top == strip.top &&
			hit.height == strip.height) {
			if (rtiff_strip_read_interleaved(rtiff,
					rtiff->page + page_no, strip_no,
					VIPS_REGION_ADDR(out_region, 0, r->top + y))) {
				VIPS_GATE_STOP(
					"rtiff_stripwise_generate: work");
				return -1;
			}
		}
		else {
			VipsPel *p;
			VipsPel *q;
			int z;

			/* Read and interleave the entire strip.
			 */
			if (rtiff_strip_read_interleaved(rtiff,
					rtiff->page + page_no, strip_no,
					rtiff->contig_buf)) {
				VIPS_GATE_STOP(
					"rtiff_stripwise_generate: work");
				return -1;
			}

			/* Do any repacking to generate pixels in vips layout.
			 */
			p = (VipsPel *) rtiff->contig_buf +
				(hit.top - strip.top) * scanline_size;
			q = VIPS_REGION_ADDR(out_region, 0, r->top + y);
			for (z = 0; z < hit.height; z++) {
				rtiff->sfn(rtiff,
					q, p, out_region->im->Xsize, rtiff->client);

				p += scanline_size;
				q += VIPS_REGION_LSKIP(out_region);
			}
		}

		y += hit.height;
		rtiff->y_pos += hit.height;
	}

	VIPS_GATE_STOP("rtiff_stripwise_generate: work");

	return 0;
}

/* Stripwise reading.
 *
 * We could potentially read strips in any order, but this would give
 * catastrophic performance for operations like 90 degrees rotate on a
 * large image. Only offer sequential read.
 */
static int
rtiff_read_stripwise(Rtiff *rtiff, VipsImage *out)
{
	VipsImage **t = (VipsImage **)
		vips_object_local_array(VIPS_OBJECT(out), 4);

	VipsImage *in;
	int tile_height;

#ifdef DEBUG
	printf("tiff2vips: rtiff_read_stripwise\n");
#endif /*DEBUG*/

	t[0] = vips_image_new();
	if (rtiff_set_header(rtiff, t[0]))
		return -1;

	/* Double check: in memcpy mode, the vips linesize should exactly
	 * match the tiff line size.
	 */
	if (rtiff->memcpy) {
		size_t vips_line_size;

		/* Lines are smaller in plane-separated mode.
		 */
		if (rtiff->header.separate)
			vips_line_size = VIPS_IMAGE_SIZEOF_ELEMENT(t[0]) *
				t[0]->Xsize;
		else
			vips_line_size = VIPS_IMAGE_SIZEOF_LINE(t[0]);

		if (rtiff->header.bits_per_sample == 16 &&
			rtiff->header.sample_format == SAMPLEFORMAT_IEEEFP)
			vips_line_size /= 2;

		if (vips_line_size != rtiff->header.scanline_size) {
			vips_error("tiff2vips", "%s", _("unsupported tiff image type"));
			return -1;
		}
	}

	/* If we have separate image planes, we must read to a plane buffer,
	 * then interleave to the output.
	 *
	 * We don't need a separate buffer per thread since the _generate()
	 * function runs inside the cache lock.
	 */
	if (rtiff->header.separate) {
		if (!(rtiff->plane_buf = VIPS_MALLOC(out,
				  rtiff->header.read_size)))
			return -1;
	}

	/* If we need to manipulate pixels, we must read to an interleaved
	 * plane buffer before repacking to the output.
	 *
	 * If we are doing a multi-page read, we need a strip buffer, since
	 * strips may not be aligned on tile boundaries.
	 *
	 * We don't need a separate buffer per thread since the _generate()
	 * function runs inside the cache lock.
	 */
	if (!rtiff->memcpy ||
		rtiff->n > 1) {
		tsize_t size;

		size = rtiff->header.read_size;
		if (rtiff->header.separate)
			size *= rtiff->header.samples_per_pixel;

		if (!(rtiff->contig_buf = VIPS_MALLOC(out, size)))
			return -1;
	}

	/* rows_per_strip can be very large if this is a separate plane image,
	 * beware.
	 *
	 * Some images have very small rowsperstrip which will cause a lot of
	 * work for the tilecache -- set a min size for tiles which is a
	 * multiple of rowsperstrip.
	 */
	tile_height = VIPS_MAX(
		VIPS_ROUND_DOWN(16, rtiff->header.read_height),
		rtiff->header.read_height);

	if (
		vips_image_generate(t[0],
			NULL, rtiff_stripwise_generate, NULL,
			rtiff, NULL) ||
		vips_sequential(t[0], &t[1],
			"tile_height", tile_height,
			NULL) ||
		rtiff_unpremultiply(rtiff, t[1], &t[2]))
		return -1;
	in = t[2];

	/* Only do this if we have to.
	 */
	if (rtiff->autorotate &&
		vips_image_get_orientation(in) != 1) {
		if (vips_autorot(in, &t[3], NULL))
			return -1;
		in = t[3];
	}

	if (vips_image_write(in, out))
		return -1;

	return 0;
}

/* Load from a tiff dir into one of our tiff header structs.
 */
static int
rtiff_header_read(Rtiff *rtiff, RtiffHeader *header)
{
	int i;
	guint16 extra_samples_count;
	guint16 *extra_samples_types;
	guint16 subifd_count;
	toff_t *subifd_offsets;
	char *image_description;
	guint32 max_tile_dimension;
	gboolean can_read_as_rgba;

	if (!tfget32(rtiff->tiff, TIFFTAG_IMAGEWIDTH,
			&header->width) ||
		!tfget32(rtiff->tiff, TIFFTAG_IMAGELENGTH,
			&header->height) ||
		!tfget16(rtiff->tiff, TIFFTAG_SAMPLESPERPIXEL,
			&header->samples_per_pixel) ||
		!tfget16(rtiff->tiff, TIFFTAG_BITSPERSAMPLE,
			&header->bits_per_sample) ||
		!tfget16(rtiff->tiff, TIFFTAG_PHOTOMETRIC,
			&header->photometric_interpretation) ||
		!tfget16(rtiff->tiff, TIFFTAG_INKSET,
			&header->inkset))
		return -1;

	header->read_as_rgba = FALSE;

	/* TIFF images which can be read by TIFFRGBAImage or TIFFReadRGBATile.
	 */
	can_read_as_rgba =
		(header->samples_per_pixel == 1 ||
			header->samples_per_pixel == 3 ||
			header->samples_per_pixel == 4) &&
		(header->bits_per_sample == 1 ||
			header->bits_per_sample == 2 ||
			header->bits_per_sample == 4 ||
			header->bits_per_sample == 8 ||
			header->bits_per_sample == 16);

	TIFFGetFieldDefaulted(rtiff->tiff,
		TIFFTAG_COMPRESSION, &header->compression);

	/* We'll decode old-style JPEG using the libtiff RGBA path.
	 */
	if (header->compression == COMPRESSION_OJPEG) {
		if (!can_read_as_rgba) {
			vips_error("tiff2vips", "%s", _("unsupported tiff image type"));
			return -1;
		}

		header->read_as_rgba = TRUE;
	}

	/* One of the types we decompress?
	 */
	for (i = 0; i < VIPS_NUMBER(rtiff_we_decompress); i++)
		if (header->compression == rtiff_we_decompress[i]) {
#ifdef DEBUG
			printf("rtiff_header_read: compression %d handled by us\n",
				header->compression);
#endif /*DEBUG*/
			header->we_decompress = TRUE;
			break;
		}

	/* We must set this here since it'll change the value of scanline_size.
	 */
	rtiff_set_decode_format(rtiff);

	/* If there's YCbCr chroma subsampling and we're not already using one of
	 * the JPEG decompressors, use the libtiff RGBA path.
	 */
	if (!header->read_as_rgba &&
		header->compression != COMPRESSION_JPEG &&
		header->photometric_interpretation == PHOTOMETRIC_YCBCR) {
		guint16 hsub, vsub;

		TIFFGetFieldDefaulted(rtiff->tiff,
			TIFFTAG_YCBCRSUBSAMPLING, &hsub, &vsub);
		if (hsub != 1 || vsub != 1) {
			if (!can_read_as_rgba) {
				vips_error("tiff2vips",
					"%s", _("subsampled images not supported"));
				return -1;
			}

			header->read_as_rgba = TRUE;
		}
	}

	if (header->photometric_interpretation == PHOTOMETRIC_LOGLUV) {
		if (header->compression != COMPRESSION_SGILOG &&
			header->compression != COMPRESSION_SGILOG24) {
			vips_error("tiff2vips",
				"%s", _("not SGI-compressed LOGLUV"));
			return -1;
		}
	}

	/* For logluv, the calibration factor to get to absolute luminance.
	 */
	if (!TIFFGetField(rtiff->tiff, TIFFTAG_STONITS, &header->stonits))
		header->stonits = 1.0;

	/* Arbitrary sanity-checking limits.
	 */
	if (header->width <= 0 ||
		header->width >= VIPS_MAX_COORD ||
		header->height <= 0 ||
		header->height >= VIPS_MAX_COORD) {
		vips_error("tiff2vips",
			"%s", _("width/height out of range"));
		return -1;
	}

	if (header->samples_per_pixel <= 0 ||
		header->samples_per_pixel > 10000 ||
		header->bits_per_sample <= 0 ||
		header->bits_per_sample > 32) {
		vips_error("tiff2vips",
			"%s", _("samples out of range"));
		return -1;
	}

	header->sample_format = get_sample_format(rtiff->tiff);
	header->orientation = get_orientation(rtiff->tiff);

	header->separate = FALSE;
	if (tfexists(rtiff->tiff, TIFFTAG_PLANARCONFIG)) {
		int v;

		if (!tfget16(rtiff->tiff, TIFFTAG_PLANARCONFIG, &v))
			return -1;
		if (v == PLANARCONFIG_SEPARATE)
			header->separate = TRUE;
	}

	/* TIFFGetField needs a guint16 to write count to.
	 */
	if (TIFFGetField(rtiff->tiff, TIFFTAG_SUBIFD,
			&subifd_count, &subifd_offsets))
		header->subifd_count = subifd_count;

	/* IMAGEDESCRIPTION often has useful metadata. libtiff makes sure
	 * that data is null-terminated and contains no embedded null
	 * characters.
	 */
	if (TIFFGetField(rtiff->tiff, TIFFTAG_IMAGEDESCRIPTION, &image_description))
		header->image_description =
			vips_strdup(VIPS_OBJECT(rtiff->out), image_description);

	/* Tiles and strip images have slightly different fields.
	 */
	header->tiled = TIFFIsTiled(rtiff->tiff);

	if (header->read_as_rgba) {
		header->we_decompress = FALSE;
		header->photometric_interpretation = PHOTOMETRIC_RGB;
		header->samples_per_pixel = 4;
		header->bits_per_sample = 8;
		header->sample_format = SAMPLEFORMAT_UINT;
		header->separate = FALSE;
	}

#ifdef DEBUG
	printf("rtiff_header_read: header.read_as_rgba = %d\n",
		header->read_as_rgba);
	printf("rtiff_header_read: header.width = %d\n",
		header->width);
	printf("rtiff_header_read: header.height = %d\n",
		header->height);
	printf("rtiff_header_read: header.samples_per_pixel = %d\n",
		header->samples_per_pixel);
	printf("rtiff_header_read: header.bits_per_sample = %d\n",
		header->bits_per_sample);
	printf("rtiff_header_read: header.sample_format = %d\n",
		header->sample_format);
	printf("rtiff_header_read: header.orientation = %d\n",
		header->orientation);
	printf("rtiff_header_read: header.tiled = %d\n",
		header->tiled);
#endif /*DEBUG*/

	if (header->tiled) {
		if (!tfget32(rtiff->tiff, TIFFTAG_TILEWIDTH, &header->tile_width) ||
			!tfget32(rtiff->tiff, TIFFTAG_TILELENGTH, &header->tile_height))
			return -1;

#ifdef DEBUG
		printf("rtiff_header_read: header.tile_width = %d\n",
			header->tile_width);
		printf("rtiff_header_read: header.tile_height = %d\n",
			header->tile_height);
#endif /*DEBUG*/

		/* Arbitrary sanity-checking limits.
		 */
		max_tile_dimension = VIPS_MIN(8192,
			VIPS_ROUND_UP(2 * VIPS_MAX(header->width, header->height), 256));
		if (header->tile_width <= 0 ||
			header->tile_width > max_tile_dimension ||
			header->tile_width % 16 != 0 ||
			header->tile_height <= 0 ||
			header->tile_height > max_tile_dimension ||
			header->tile_height % 16 != 0) {
			vips_error("tiff2vips",
				"%s", _("tile size out of range"));
			return -1;
		}

		if (header->read_as_rgba) {
			header->tile_row_size = header->tile_width * 4;
			header->tile_size = header->tile_row_size * header->tile_height;
		}
		else {
			header->tile_size = TIFFTileSize(rtiff->tiff);
			header->tile_row_size = TIFFTileRowSize(rtiff->tiff);
		}

#ifdef DEBUG
		printf("rtiff_header_read: header.tile_size = %zd\n",
			header->tile_size);
		printf("rtiff_header_read: header.tile_row_size = %zd\n",
			header->tile_row_size);
#endif /*DEBUG*/

		/* Fuzzed TIFFs can give crazy values for tile_size. Sanity
		 * check at 100mb per tile.
		 */
		if (header->tile_size <= 0 ||
			header->tile_size > 100 * 1000 * 1000 ||
			header->tile_row_size <= 0 ||
			header->tile_row_size > 100 * 1000 * 1000) {
			vips_error("tiff2vips",
				"%s", _("tile size out of range"));
			return -1;
		}

		/* Stop some compiler warnings.
		 */
		header->rows_per_strip = 0;
		header->strip_size = 0;
		header->number_of_strips = 0;
		header->read_height = 0;
		header->read_size = 0;
	}
	else {
		if (!tfget32(rtiff->tiff,
				TIFFTAG_ROWSPERSTRIP, &header->rows_per_strip))
			return -1;

		/* rows_per_strip can be 2 ** 32 - 1, meaning the
		 * whole image. Clip this down to height to avoid
		 * confusing vips.
		 *
		 * And it mustn't be zero.
		 */
		header->rows_per_strip = VIPS_CLIP(1,
			header->rows_per_strip, header->height);

		header->number_of_strips = TIFFNumberOfStrips(rtiff->tiff);

		if (header->read_as_rgba) {
			header->scanline_size = header->width * 4;
			header->strip_size = header->scanline_size * header->rows_per_strip;
		}
		else {
			header->scanline_size = TIFFScanlineSize(rtiff->tiff);
			header->strip_size = TIFFStripSize(rtiff->tiff);
		}

#ifdef DEBUG
		printf("rtiff_header_read: header.rows_per_strip = %d\n",
			header->rows_per_strip);
		printf("rtiff_header_read: header.strip_size = %zd\n",
			header->strip_size);
		printf("rtiff_header_read: header.scanline_size = %zd\n",
			header->scanline_size);
		printf("rtiff_header_read: header.number_of_strips = %d\n",
			header->number_of_strips);
#endif /*DEBUG*/

		/* libtiff has two strip-wise readers. TIFFReadEncodedStrip()
		 * decompresses an entire strip to memory. It's fast, but it
		 * will need a lot of ram if the strip is large.
		 * TIFFReadScanline() reads a single scanline. It's slower, but
		 * will save a lot of memory if strips are large.
		 *
		 * If this image has a strip size of over 128 lines, fall back
		 * to TIFFReadScanline(), otherwise use TIFFReadEncodedStrip().
		 *
		 * Don't do this in plane-separate mode. TIFFReadScanline() is
		 * too fiddly to use in this case.
		 *
		 * Don't try scanline reading for YCbCr images.
		 * TIFFScanlineSize() will not work in this case due to
		 * chroma subsampling.
		 *
		 * Don't use scanline reading if we're going to use TIFFRGBAImage
		 */
		if (header->rows_per_strip > 128 &&
			!header->separate &&
			header->photometric_interpretation != PHOTOMETRIC_YCBCR &&
			!header->read_as_rgba) {
			header->read_scanlinewise = TRUE;
			header->read_height = 1;
			header->read_size = rtiff->header.scanline_size;
		}
		else {
			header->read_scanlinewise = FALSE;
			header->read_height = header->rows_per_strip;
			header->read_size = header->strip_size;
		}

#ifdef DEBUG
		printf("rtiff_header_read: header.read_scanlinewise = %d\n",
			header->read_scanlinewise);
		printf("rtiff_header_read: header.read_height = %d\n",
			header->read_height);
		printf("rtiff_header_read: header.read_size = %zd\n",
			header->read_size);
#endif /*DEBUG*/

		/* Stop some compiler warnings.
		 */
		header->tile_width = 0;
		header->tile_height = 0;
		header->tile_size = 0;
		header->tile_row_size = 0;
	}

	TIFFGetFieldDefaulted(rtiff->tiff, TIFFTAG_EXTRASAMPLES,
		&extra_samples_count, &extra_samples_types);

	header->alpha_band = -1;
	if (extra_samples_count > 0) {
		/* There must be exactly one band which is
		 * EXTRASAMPLE_ASSOCALPHA. Note which one it is so we can
		 * unpremultiply with the right channel.
		 */
		int i;

		for (i = 0; i < extra_samples_count; i++)
			if (extra_samples_types[i] == EXTRASAMPLE_ASSOCALPHA) {
				if (header->alpha_band != -1)
					g_warning("more than one alpha -- ignoring");

				header->alpha_band = header->samples_per_pixel -
					extra_samples_count + i;
			}
	}

	return 0;
}

static int
rtiff_header_equal(RtiffHeader *h1, RtiffHeader *h2)
{
	if (h1->width != h2->width ||
		h1->height != h2->height ||
		h1->samples_per_pixel != h2->samples_per_pixel ||
		h1->bits_per_sample != h2->bits_per_sample ||
		h1->photometric_interpretation != h2->photometric_interpretation ||
		h1->sample_format != h2->sample_format ||
		h1->compression != h2->compression ||
		h1->separate != h2->separate ||
		h1->tiled != h2->tiled ||
		h1->orientation != h2->orientation)
		return 0;

	if (h1->tiled) {
		if (h1->tile_width != h2->tile_width ||
			h1->tile_height != h2->tile_height)
			return 0;
	}
	else {
		if (h1->read_height != h2->read_height ||
			h1->read_size != h2->read_size ||
			h1->number_of_strips != h2->number_of_strips)
			return 0;
	}

	return 1;
}

static int
rtiff_header_read_all(Rtiff *rtiff)
{
#ifdef DEBUG
	printf("rtiff_header_read_all: reading header for page %d ...\n",
		rtiff->page);
#endif /*DEBUG*/

	/* -1 means "to the end".
	 *
	 * We must count pages before selecting and reading the header of the
	 * first page, since scanning a TIFF can change the value of libtiff's
	 * internal header fields in strange ways, especially if the TIFF is
	 * corrupt.
	 */
	rtiff->n_pages = rtiff_n_pages(rtiff);

	if (rtiff_set_page(rtiff, rtiff->page) ||
		rtiff_header_read(rtiff, &rtiff->header))
		return -1;

	/* If we're to read many pages, verify that they are all identical.
	 */
	if (rtiff->n == -1)
		rtiff->n = rtiff->n_pages - rtiff->page;
	if (rtiff->n > 1) {
		int i;

		for (i = 1; i < rtiff->n; i++) {
			RtiffHeader header;

#ifdef DEBUG
			printf("rtiff_header_read_all: verifying header for page %d ...\n",
				rtiff->page + i);
#endif /*DEBUG*/

			if (rtiff_set_page(rtiff, rtiff->page + i) ||
				rtiff_header_read(rtiff, &header))
				return -1;

			if (!rtiff_header_equal(&rtiff->header, &header)) {
				vips_error("tiff2vips", _("page %d differs from page %d"),
					rtiff->page + i, rtiff->page);
				return -1;
			}
		}

		/* Make sure the next set_page() will reread the directory.
		 */
		rtiff->current_page = -1;
	}

	return 0;
}

typedef gboolean (*TiffPropertyFn)(TIFF *tif);

static gboolean
vips__testtiff_source(VipsSource *source, TiffPropertyFn fn)
{
	TIFF *tif;
	gboolean property;

	vips__tiff_init();

	if (!(tif = vips__tiff_openin_source(source, rtiff_handler_error,
		rtiff_handler_warning, NULL, FALSE))) {
		vips_error_clear();
		return FALSE;
	}

	property = fn ? fn(tif) : TRUE;

	TIFFClose(tif);

	return property;
}

gboolean
vips__istiff_source(VipsSource *source)
{
	return vips__testtiff_source(source, NULL);
}

gboolean
vips__istifftiled_source(VipsSource *source)
{
	return vips__testtiff_source(source, TIFFIsTiled);
}

int
vips__tiff_read_header_source(VipsSource *source, VipsImage *out,
	int page, int n, gboolean autorotate, int subifd, VipsFailOn fail_on,
	gboolean unlimited)
{
	Rtiff *rtiff;

	vips__tiff_init();

	if (!(rtiff = rtiff_new(source, out,
			  page, n, autorotate, subifd, fail_on, unlimited)) ||
		rtiff_header_read_all(rtiff))
		return -1;

	if (rtiff_set_header(rtiff, out))
		return -1;

	if (rtiff->autorotate &&
		vips_image_get_orientation_swap(out)) {
		VIPS_SWAP(int, out->Xsize, out->Ysize);
		vips_autorot_remove_angle(out);
	}

	/* We never call vips_source_decode() since we need to be able to
	 * seek() the whole way through the file. Just minimise instead,
	 */
	vips_source_minimise(source);

	return 0;
}

int
vips__tiff_read_source(VipsSource *source, VipsImage *out,
	int page, int n, gboolean autorotate, int subifd, VipsFailOn fail_on,
	gboolean unlimited)
{
	Rtiff *rtiff;

#ifdef DEBUG
	printf("tiff2vips: libtiff version is \"%s\"\n", TIFFGetVersion());
#endif /*DEBUG*/

	vips__tiff_init();

	if (!(rtiff = rtiff_new(source, out,
			  page, n, autorotate, subifd, fail_on, unlimited)) ||
		rtiff_header_read_all(rtiff))
		return -1;

	if (rtiff->header.tiled) {
		if (rtiff_read_tilewise(rtiff, out))
			return -1;
	}
	else {
		if (rtiff_read_stripwise(rtiff, out))
			return -1;
	}

	/* We never call vips_source_decode() since we need to be able to
	 * seek() the whole way through the file. Just minimise instead,
	 */
	vips_source_minimise(source);

	return 0;
}

#endif /*HAVE_TIFF*/
