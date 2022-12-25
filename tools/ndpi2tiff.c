/* ndpi2tiff
 v. 1.5-3
 Copyright (c) 2011-2021 Christophe Deroulers
 Distributed under the GNU General Public License v3 -- contact the 
 author for commercial use
 Based on tiffcp.c -- copyright notice of tiffcp.c below */

/*
 * Copyright (c) 1988-1997 Sam Leffler
 * Copyright (c) 1991-1997 Silicon Graphics, Inc.
 *
 *  Revised:  2/18/01 BAR -- added syntax for extracting single images from
 *                          multi-image TIFF files.
 *
 *    New syntax is:  sourceFileName,image#
 *
 * image# ranges from 0..<n-1> where n is the # of images in the file.
 * There may be no white space between the comma and the filename or
 * image number.
 *
 *    Example:   tiffcp source.tif,1 destination.tif
 *
 * Copies the 2nd image in source.tif to the destination.
 *
 *****
 * Permission to use, copy, modify, distribute, and sell this software and 
 * its documentation for any purpose is hereby granted without fee, provided
 * that (i) the above copyright notices and this permission notice appear in
 * all copies of the software and related documentation, and (ii) the names of
 * Sam Leffler and Silicon Graphics may not be used in any advertising or
 * publicity relating to the software without the specific, prior written
 * permission of Sam Leffler and Silicon Graphics.
 * 
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY 
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  
 * 
 * IN NO EVENT SHALL SAM LEFFLER OR SILICON GRAPHICS BE LIABLE FOR
 * ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF 
 * LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE 
 * OF THIS SOFTWARE.
 */

#include "tif_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <ctype.h>
#include <assert.h>

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#include "tiffio.h"

#ifndef HAVE_GETOPT
extern int getopt(int, char**, char*);
#endif

#if defined(VMS)
# define unlink delete
#endif

#define	streq(a,b)	(strcmp(a,b) == 0)
#define	strneq(a,b,n)	(strncmp(a,b,n) == 0)

#define	TRUE	1
#define	FALSE	0

#define TIFF_INT32_FORMAT "%"PRId32
#define TIFF_UINT32_FORMAT "%"PRIu32
#define TIFF_UINT64_FORMAT "%"PRIu64

static int outtiled = -1;
static uint32_t tilewidth;
static uint32_t tilelength;

static uint16_t config;
static uint16_t compression;
static uint16_t predictor;
static int preset;
static uint16_t fillorder;
static uint16_t orientation;
static uint32_t rowsperstrip;
static uint32_t g3opts;
static int ignore = FALSE;		/* if true, ignore read errors */
static uint32_t defg3opts = (uint32_t) -1;
static int quality = 75;		/* JPEG quality */
static int jpegcolormode = JPEGCOLORMODE_RGB;
static uint16_t defcompression = (uint16_t) -1;
static uint16_t defpredictor = (uint16_t) -1;
static int defpreset =  -1;

static int tiffcp(TIFF*, TIFF*);
static int processCompressOptions(char*);
static void usage(void);

static char comma = ',';  /* (default) comma separator character */
static TIFF* bias = NULL;
static int pageNum = 0;
static int pageInSeq = 0;

static const char TIFF_SUFFIX[] = ".tif";

static void my_asprintf(char **ret, const char *format, ...)
{
	int n;
	char * p;
	va_list ap;

	va_start(ap, format);
	n= vsnprintf(NULL, 0, format, ap);
	va_end(ap);
	p= _NDPImalloc(n+1);
	if (p == NULL)
	{
		perror("Insufficient memory for a character string ");
		exit(EXIT_FAILURE);
	}
	va_start(ap, format);
	vsnprintf(p, n+1, format, ap);
	va_end(ap);
	*ret= p;
}

static char* build_outfilename(const char* infilename)
{
	char* outfilename;
	int l;

	l = strlen(infilename);
	if ((infilename[l-1] == 'i' || infilename[l-1] == 'I') &&
	    (infilename[l-2] == 'p' || infilename[l-2] == 'P') &&
	    (infilename[l-3] == 'd' || infilename[l-3] == 'D') &&
	    (infilename[l-4] == 'n' || infilename[l-4] == 'N') &&
	    (infilename[l-5] == '.'))
		l-= 5;
	outfilename = _NDPImalloc(l+sizeof(TIFF_SUFFIX));
	if (outfilename == NULL)
	{
		perror("Insufficient memory for a character string ");
		exit(EXIT_FAILURE);
	}
	strncpy(outfilename, infilename, l);
	strcat(outfilename, TIFF_SUFFIX);
	return outfilename;
}

static int nextSrcImage (TIFF *tif, char **imageSpec)
/*
  seek to the next image specified in *imageSpec
  returns 1 if success, 0 if no more images to process
  *imageSpec=NULL if subsequent images should be processed in sequence
*/
{
	if (**imageSpec == comma) {  /* if not @comma, we've done all images */
		char *start = *imageSpec + 1;
		tdir_t nextImage = (tdir_t)strtol(start, imageSpec, 0);
		if (start == *imageSpec) nextImage = NDPICurrentDirectory (tif);
		if (**imageSpec)
		{
			if (**imageSpec == comma) {
				/* a trailing comma denotes remaining images in sequence */
				if ((*imageSpec)[1] == '\0') *imageSpec = NULL;
			}else{
				fprintf (stderr,
				    "Expected a %c separated image # list after %s\n",
				    comma, NDPIFileName (tif));
				exit (-4);   /* syntax error */
			}
		}
		if (NDPISetDirectory (tif, nextImage)) return 1;
		fprintf (stderr, "%s%c%d not found!\n",
		    NDPIFileName(tif), comma, (int) nextImage);
	}
	return 0;
}

  
static TIFF* openSrcImage (char **imageSpec)
/*
  imageSpec points to a pointer to a filename followed by optional ,image#'s
  Open the TIFF file and assign *imageSpec to either NULL if there are
  no images specified, or a pointer to the next image number text
*/
{
	TIFF *tif;
	char *fn = *imageSpec;
	*imageSpec = strchr (fn, comma);
	if (*imageSpec) {  /* there is at least one image number specifier */
		**imageSpec = '\0';
		tif = NDPIOpen (fn, "r");
		/* but, ignore any single trailing comma */
		if (!(*imageSpec)[1]) {*imageSpec = NULL; return tif;}
		if (tif) {
			**imageSpec = comma;  /* replace the comma */
			if (!nextSrcImage(tif, imageSpec)) {
				NDPIClose (tif);
				tif = NULL;
			}
		}
	}else
		tif = NDPIOpen (fn, "r");
	return tif;
}

int
main(int argc, char* argv[])
{
	uint16_t defconfig = (uint16_t) -1;
	uint16_t deffillorder = 0;
	uint32_t deftilewidth = (uint32_t) -1;
	uint32_t deftilelength = (uint32_t) -1;
	uint32_t defrowsperstrip = (uint32_t) 0;
	uint64_t diroff = 0;
	TIFF* in;
	TIFF* out;
	char mode[10];
	char* mp = mode;
	char* outfilename;
	int c;
	extern int optind;
	extern char* optarg;
	TIFFErrorHandler oerror;
	TIFFErrorHandler owarning;

	oerror = NDPISetErrorHandler(NULL);
	owarning = NDPISetWarningHandler(NULL);

	*mp++ = 'w';
	*mp = '\0';
	while ((c = getopt(argc, argv, ",:b:c:f:l:o:z:p:r:w:T:aistBLMC8x")) != -1)
		switch (c) {
		case ',':
			if (optarg[0] != '=') usage();
			comma = optarg[1];
			break;
		case 'b':   /* this file is bias image subtracted from others */
			if (bias) {
				fputs ("Only 1 bias image may be specified\n", stderr);
				exit (-2);
			}
			{
				uint16_t samples = (uint16_t) -1;
				char **biasFn = &optarg;
				bias = openSrcImage (biasFn);
				if (!bias) exit (-5);
				if (NDPIIsTiled (bias)) {
					fputs ("Bias image must be organized in strips\n", stderr);
					exit (-7);
				}
				NDPIGetField(bias, TIFFTAG_SAMPLESPERPIXEL, &samples);
				if (samples != 1) {
					fputs ("Bias image must be monochrome\n", stderr);
					exit (-7);
				}
			}
			break;
		case 'a':   /* append to output */
			mode[0] = 'a';
			break;
		case 'c':   /* compression scheme */
			if (!processCompressOptions(optarg))
				usage();
			break;
		case 'f':   /* fill order */
			if (streq(optarg, "lsb2msb"))
				deffillorder = FILLORDER_LSB2MSB;
			else if (streq(optarg, "msb2lsb"))
				deffillorder = FILLORDER_MSB2LSB;
			else
				usage();
			break;
		case 'i':   /* ignore errors */
			ignore = TRUE;
			break;
		case 'l':   /* tile length */
			outtiled = TRUE;
			deftilelength = atoi(optarg);
			break;
		case 'o':   /* initial directory offset */
			diroff = strtoul(optarg, NULL, 0);
			break;
		case 'p':   /* planar configuration */
			if (streq(optarg, "separate"))
				defconfig = PLANARCONFIG_SEPARATE;
			else if (streq(optarg, "contig"))
				defconfig = PLANARCONFIG_CONTIG;
			else
				usage();
			break;
		case 'r':   /* rows/strip */
			defrowsperstrip = atol(optarg);
			break;
		case 's':   /* generate stripped output */
			outtiled = FALSE;
			break;
		case 't':   /* generate tiled output */
			outtiled = TRUE;
			break;
		case 'w':   /* tile width */
			outtiled = TRUE;
			deftilewidth = atoi(optarg);
			break;
		case 'B':
			*mp++ = 'b'; *mp = '\0';
			break;
		case 'L':
			*mp++ = 'l'; *mp = '\0';
			break;
		case 'M':
			*mp++ = 'm'; *mp = '\0';
			break;
		case 'C':
			*mp++ = 'c'; *mp = '\0';
			break;
		case '8':
			*mp++ = '8'; *mp = '\0';
			break;
		case 'x':
			pageInSeq = 1;
			break;
		case 'T':
			switch (optarg[0]) {
			case 'W':
				NDPISetWarningHandler(owarning);
				break;
			case 'E':
				NDPISetErrorHandler(oerror);
				break;
			default:
				usage();
			}
			break;
		case '?':
			usage();
			/*NOTREACHED*/
		}
	if (argc - optind != 1)
		usage();
	outfilename= build_outfilename(argv[optind]);
	out = NDPIOpen(outfilename, mode);
	if (out == NULL)
		return (-2);
	_NDPIfree(outfilename);
	pageNum = -1;
	for (; optind <= argc-1 ; optind++) {
		char *imageCursor = argv[optind];
		in = openSrcImage (&imageCursor);
		if (in == NULL) {
			(void) NDPIClose(out);
			return (-3);
		}
		if (diroff != 0 && !NDPISetSubDirectory(in, diroff)) {
			NDPIError(NDPIFileName(in),
			    "Error, setting subdirectory at " TIFF_UINT64_FORMAT, diroff);
			(void) NDPIClose(in);
			(void) NDPIClose(out);
			return (1);
		}
		for (;;) {
			config = defconfig;
			compression = defcompression;
			predictor = defpredictor;
                        preset = defpreset;
			fillorder = deffillorder;
			rowsperstrip = defrowsperstrip;
			tilewidth = deftilewidth;
			tilelength = deftilelength;
			g3opts = defg3opts;
			if (!tiffcp(in, out) || !NDPIWriteDirectory(out)) {
				(void) NDPIClose(in);
				(void) NDPIClose(out);
				return (1);
			}
			if (imageCursor) { /* seek next image directory */
				if (!nextSrcImage(in, &imageCursor)) break;
			}else
				if (!NDPIReadDirectory(in)) break;
		}
		(void) NDPIClose(in);
	}

	(void) NDPIClose(out);
	return (0);
}

static void
processZIPOptions(char* cp)
{
	if ( (cp = strchr(cp, ':')) ) {
		do {
			cp++;
			if (isdigit((int)*cp))
				defpredictor = atoi(cp);
			else if (*cp == 'p')
				defpreset = atoi(++cp);
			else
				usage();
		} while( (cp = strchr(cp, ':')) );
	}
}

static void
processG3Options(char* cp)
{
	if( (cp = strchr(cp, ':')) ) {
		if (defg3opts == (uint32_t) -1)
			defg3opts = 0;
		do {
			cp++;
			if (strneq(cp, "1d", 2))
				defg3opts &= ~GROUP3OPT_2DENCODING;
			else if (strneq(cp, "2d", 2))
				defg3opts |= GROUP3OPT_2DENCODING;
			else if (strneq(cp, "fill", 4))
				defg3opts |= GROUP3OPT_FILLBITS;
			else
				usage();
		} while( (cp = strchr(cp, ':')) );
	}
}

static int
processCompressOptions(char* opt)
{
	if (streq(opt, "none")) {
		defcompression = COMPRESSION_NONE;
	} else if (streq(opt, "packbits")) {
		defcompression = COMPRESSION_PACKBITS;
	} else if (strneq(opt, "jpeg", 4)) {
		char* cp = strchr(opt, ':');

		defcompression = COMPRESSION_JPEG;
		while( cp )
		{
			if (isdigit((int)cp[1]))
				quality = atoi(cp+1);
			else if (cp[1] == 'r' )
				jpegcolormode = JPEGCOLORMODE_RAW;
			else
				usage();

			cp = strchr(cp+1,':');
		}
	} else if (strneq(opt, "g3", 2)) {
		processG3Options(opt);
		defcompression = COMPRESSION_CCITTFAX3;
	} else if (streq(opt, "g4")) {
		defcompression = COMPRESSION_CCITTFAX4;
	} else if (strneq(opt, "lzw", 3)) {
		char* cp = strchr(opt, ':');
		if (cp)
			defpredictor = atoi(cp+1);
		defcompression = COMPRESSION_LZW;
	} else if (strneq(opt, "zip", 3)) {
		processZIPOptions(opt);
		defcompression = COMPRESSION_ADOBE_DEFLATE;
	} else if (strneq(opt, "lzma", 4)) {
		processZIPOptions(opt);
		defcompression = COMPRESSION_LZMA;
	} else if (strneq(opt, "jbig", 4)) {
		defcompression = COMPRESSION_JBIG;
	} else if (strneq(opt, "sgilog", 6)) {
		defcompression = COMPRESSION_SGILOG;
	} else
		return (0);
	return (1);
}

char* stuff[] = {
"usage: ndpi2tiff [options] ndpi_input_file",
"where options are:",
" -a              append to output instead of overwriting",
" -o offset       set initial directory offset",
" -p contig       pack samples contiguously (e.g. RGBRGB...)",
" -p separate     store samples separately (e.g. RRR...GGG...BBB...)",
" -s              write output in strips",
" -t              write output in tiles",
" -8              write BigTIFF instead of default ClassicTIFF",
" -i              ignore read errors",
" -TE             report TIFF errors",
" -TW             report TIFF warnings",
" -b file[,#]     bias (dark) monochrome image to be subtracted from all others",
" -,=%            use % rather than , to separate image #'s (per Note below)",
"",
" -r #            make each strip have no more than # rows",
" -w #            set output tile width (pixels)",
" -l #            set output tile length (pixels)",
"",
" -f lsb2msb      force lsb-to-msb FillOrder for output",
" -f msb2lsb      force msb-to-lsb FillOrder for output",
"",
" -c lzw[:opts]   compress output with Lempel-Ziv & Welch encoding",
" -c zip[:opts]   compress output with deflate encoding",
" -c lzma[:opts]  compress output with LZMA2 encoding",
" -c jpeg[:opts]  compress output with JPEG encoding",
" -c jbig         compress output with ISO JBIG encoding",
" -c packbits     compress output with packbits encoding",
" -c g3[:opts]    compress output with CCITT Group 3 encoding",
" -c g4           compress output with CCITT Group 4 encoding",
" -c sgilog       compress output with SGILOG encoding",
" -c none         use no compression algorithm on output",
" -x              force the merged tiff pages in sequence",
"",
"Group 3 options:",
" 1d              use default CCITT Group 3 1D-encoding",
" 2d              use optional CCITT Group 3 2D-encoding",
" fill            byte-align EOL codes",
"For example, -c g3:2d:fill to get G3-2D-encoded data with byte-aligned EOLs",
"",
"JPEG options:",
" #               set compression quality level (0-100, default 75)",
" r               output color image as RGB rather than YCbCr",
"For example, -c jpeg:r:50 to get JPEG-encoded RGB data with 50% comp. quality",
"",
"LZW, Deflate (ZIP) and LZMA2 options:",
" #               set predictor value",
" p#              set compression level (preset)",
"For example, -c lzw:2 to get LZW-encoded data with horizontal differencing,",
"-c zip:3:p9 for Deflate encoding with maximum compression level and floating",
"point predictor.",
"",
"Note that input filenames may be of the form filename,x,y,z",
"where x, y, and z specify image numbers in the filename to copy.",
"example:  ndpi2tiff -c none esp.ndpi,0",
"  extracts only the 1st image from esp.ndpi yielding uncompressed result esp.tif",
NULL
};

static void
usage(void)
{
	char buf[BUFSIZ];
	int i;

	setbuf(stderr, buf);
	fprintf(stderr, "ndpi2tiff version 1.5-3 license GNU GPL v3 (c) 2011-2021 Christophe Deroulers\n"
			"Please quote \"Diagnostic Pathology 2013, 8:92\" if you use for research\n");
	for (i = 0; stuff[i] != NULL; i++)
		fprintf(stderr, "%s\n", stuff[i]);
	exit(-1);
}

#define	CopyField(tag, v) \
    if (NDPIGetField(in, tag, &v)) NDPISetField(out, tag, v)
#define	CopyField2(tag, v1, v2) \
    if (NDPIGetField(in, tag, &v1, &v2)) NDPISetField(out, tag, v1, v2)
#define	CopyField3(tag, v1, v2, v3) \
    if (NDPIGetField(in, tag, &v1, &v2, &v3)) NDPISetField(out, tag, v1, v2, v3)
#define	CopyField4(tag, v1, v2, v3, v4) \
    if (NDPIGetField(in, tag, &v1, &v2, &v3, &v4)) NDPISetField(out, tag, v1, v2, v3, v4)

static void
cpTag(TIFF* in, TIFF* out, uint16_t tag, uint16_t count, TIFFDataType type)
{
	switch (type) {
	case TIFF_SHORT:
		if (count == 1) {
			uint16_t shortv;
			CopyField(tag, shortv);
		} else if (count == 2) {
			uint16_t shortv1, shortv2;
			CopyField2(tag, shortv1, shortv2);
		} else if (count == 4) {
			uint16_t *tr, *tg, *tb, *ta;
			CopyField4(tag, tr, tg, tb, ta);
		} else if (count == (uint16_t) -1) {
			uint16_t shortv1;
			uint16_t* shortav;
			CopyField2(tag, shortv1, shortav);
		}
		break;
	case TIFF_LONG:
		{ uint32_t longv;
		  CopyField(tag, longv);
		}
		break;
	case TIFF_RATIONAL:
		if (count == 1) {
			float floatv;
			CopyField(tag, floatv);
		} else if (count == (uint16_t) -1) {
			float* floatav;
			CopyField(tag, floatav);
		}
		break;
	case TIFF_ASCII:
		{ char* stringv;
		  CopyField(tag, stringv);
		}
		break;
	case TIFF_DOUBLE:
		if (count == 1) {
			double doublev;
			CopyField(tag, doublev);
		} else if (count == (uint16_t) -1) {
			double* doubleav;
			CopyField(tag, doubleav);
		}
		break;
	default:
		NDPIError(NDPIFileName(in),
		    "Data type %d is not supported, tag %d skipped.",
		    tag, type);
	}
}

static struct cpTag {
	uint16_t tag;
	uint16_t count;
	TIFFDataType type;
} tags[] = {
	{ TIFFTAG_SUBFILETYPE,		1, TIFF_LONG },
	{ TIFFTAG_THRESHHOLDING,	1, TIFF_SHORT },
	{ TIFFTAG_DOCUMENTNAME,		1, TIFF_ASCII },
	{ TIFFTAG_IMAGEDESCRIPTION,	1, TIFF_ASCII },
	{ TIFFTAG_MAKE,			1, TIFF_ASCII },
	{ TIFFTAG_MODEL,		1, TIFF_ASCII },
	{ TIFFTAG_MINSAMPLEVALUE,	1, TIFF_SHORT },
	{ TIFFTAG_MAXSAMPLEVALUE,	1, TIFF_SHORT },
	{ TIFFTAG_XRESOLUTION,		1, TIFF_RATIONAL },
	{ TIFFTAG_YRESOLUTION,		1, TIFF_RATIONAL },
	{ TIFFTAG_PAGENAME,		1, TIFF_ASCII },
	{ TIFFTAG_XPOSITION,		1, TIFF_RATIONAL },
	{ TIFFTAG_YPOSITION,		1, TIFF_RATIONAL },
	{ TIFFTAG_RESOLUTIONUNIT,	1, TIFF_SHORT },
	{ TIFFTAG_SOFTWARE,		1, TIFF_ASCII },
	{ TIFFTAG_DATETIME,		1, TIFF_ASCII },
	{ TIFFTAG_ARTIST,		1, TIFF_ASCII },
	{ TIFFTAG_HOSTCOMPUTER,		1, TIFF_ASCII },
	{ TIFFTAG_WHITEPOINT,		(uint16_t) -1, TIFF_RATIONAL },
	{ TIFFTAG_PRIMARYCHROMATICITIES,(uint16_t) -1,TIFF_RATIONAL },
	{ TIFFTAG_HALFTONEHINTS,	2, TIFF_SHORT },
	{ TIFFTAG_INKSET,		1, TIFF_SHORT },
	{ TIFFTAG_DOTRANGE,		2, TIFF_SHORT },
	{ TIFFTAG_TARGETPRINTER,	1, TIFF_ASCII },
	{ TIFFTAG_SAMPLEFORMAT,		1, TIFF_SHORT },
	{ TIFFTAG_YCBCRCOEFFICIENTS,	(uint16_t) -1,TIFF_RATIONAL },
	{ TIFFTAG_YCBCRSUBSAMPLING,	2, TIFF_SHORT },
	{ TIFFTAG_YCBCRPOSITIONING,	1, TIFF_SHORT },
	{ TIFFTAG_REFERENCEBLACKWHITE,	(uint16_t) -1,TIFF_RATIONAL },
	{ TIFFTAG_EXTRASAMPLES,		(uint16_t) -1, TIFF_SHORT },
	{ TIFFTAG_SMINSAMPLEVALUE,	1, TIFF_DOUBLE },
	{ TIFFTAG_SMAXSAMPLEVALUE,	1, TIFF_DOUBLE },
	{ TIFFTAG_STONITS,		1, TIFF_DOUBLE },
};
#define	NTAGS	(sizeof (tags) / sizeof (tags[0]))

#define	CopyTag(tag, count, type)	cpTag(in, out, tag, count, type)

typedef int (*copyFunc)
    (TIFF* in, TIFF* out, uint32_t l, uint32_t w, uint16_t samplesperpixel);
static	copyFunc pickCopyFunc(TIFF*, TIFF*, uint16_t, uint16_t);

/* PODD */

static int
tiffcp(TIFF* in, TIFF* out)
{
	uint16_t bitspersample, samplesperpixel;
	uint16_t input_compression, input_photometric;
	copyFunc cf;
	uint32_t width, length;
	struct cpTag* p;
	int thisouttiled = outtiled;
	int32_t ndpi_zoffset;
	float ndpi_magnification;
	char* imagedescription;

	CopyField(TIFFTAG_IMAGEWIDTH, width);
	CopyField(TIFFTAG_IMAGELENGTH, length);
	CopyField(TIFFTAG_BITSPERSAMPLE, bitspersample);
	CopyField(TIFFTAG_SAMPLESPERPIXEL, samplesperpixel);
	if (compression != (uint16_t)-1)
		NDPISetField(out, TIFFTAG_COMPRESSION, compression);
	else
		CopyField(TIFFTAG_COMPRESSION, compression);
	NDPIGetFieldDefaulted(in, TIFFTAG_COMPRESSION, &input_compression);
	NDPIGetFieldDefaulted(in, TIFFTAG_PHOTOMETRIC, &input_photometric);
	if (input_compression == COMPRESSION_JPEG) {
		/* Force conversion to RGB */
		NDPISetField(in, TIFFTAG_JPEGCOLORMODE, JPEGCOLORMODE_RGB);
	} else if (input_photometric == PHOTOMETRIC_YCBCR) {
		/* Otherwise, can't handle subsampled input */
		uint16_t subsamplinghor,subsamplingver;

		NDPIGetFieldDefaulted(in, TIFFTAG_YCBCRSUBSAMPLING,
				      &subsamplinghor, &subsamplingver);
		if (subsamplinghor!=1 || subsamplingver!=1) {
			fprintf(stderr, "ndpi2tiff: %s: Can't copy/convert subsampled image.\n",
				NDPIFileName(in));
			return FALSE;
		}
	}
	if (compression == COMPRESSION_JPEG) {
		if (input_photometric == PHOTOMETRIC_RGB &&
		    jpegcolormode == JPEGCOLORMODE_RGB)
		  NDPISetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_YCBCR);
		else
		  NDPISetField(out, TIFFTAG_PHOTOMETRIC, input_photometric);
	}
	else if (compression == COMPRESSION_SGILOG
	    || compression == COMPRESSION_SGILOG24)
		NDPISetField(out, TIFFTAG_PHOTOMETRIC,
		    samplesperpixel == 1 ?
		    PHOTOMETRIC_LOGL : PHOTOMETRIC_LOGLUV);
	else
		CopyTag(TIFFTAG_PHOTOMETRIC, 1, TIFF_SHORT);
	if (fillorder != 0)
		NDPISetField(out, TIFFTAG_FILLORDER, fillorder);
	else
		CopyTag(TIFFTAG_FILLORDER, 1, TIFF_SHORT);
	/*
	 * Will copy `Orientation' tag from input image
	 */
	NDPIGetFieldDefaulted(in, TIFFTAG_ORIENTATION, &orientation);
	switch (orientation) {
		case ORIENTATION_BOTRIGHT:
		case ORIENTATION_RIGHTBOT:	/* XXX */
			NDPIWarning(NDPIFileName(in), "using bottom-left orientation");
			orientation = ORIENTATION_BOTLEFT;
		/* fall thru... */
		case ORIENTATION_LEFTBOT:	/* XXX */
		case ORIENTATION_BOTLEFT:
			break;
		case ORIENTATION_TOPRIGHT:
		case ORIENTATION_RIGHTTOP:	/* XXX */
		default:
			NDPIWarning(NDPIFileName(in), "using top-left orientation");
			orientation = ORIENTATION_TOPLEFT;
		/* fall thru... */
		case ORIENTATION_LEFTTOP:	/* XXX */
		case ORIENTATION_TOPLEFT:
			break;
	}
	/*
	 * Look for magnification and z-offset of NDPI slice
	 */
	NDPISetField(out, TIFFTAG_ORIENTATION, orientation);
	if (! NDPIGetField(in, NDPITAG_MAGNIFICATION, &ndpi_magnification) ) {
		NDPIWarning(NDPIFileName(in),
			"Warning, Magnification not found in NDPI file subdirectory");
		ndpi_magnification= 0;
	}
	if (ndpi_magnification == -1) {
		my_asprintf(&imagedescription, "macroscopic",
			ndpi_magnification);
	} else if (ndpi_magnification == -2) {
		my_asprintf(&imagedescription, "map");
	} else {
		if (! NDPIGetField(in, NDPITAG_ZOFFSET, &ndpi_zoffset)) {
			NDPIError(NDPIFileName(in),
				"Error, z-Offset not found in NDPI file subdirectory");
			return (0);
		}
		my_asprintf(&imagedescription, "x%g z" TIFF_INT32_FORMAT,
			ndpi_magnification, ndpi_zoffset);
		fprintf(stderr, "Dealing with slice at magnification x%g at z-offset=" TIFF_INT32_FORMAT "\n",
			ndpi_magnification, ndpi_zoffset);
	}
	NDPISetField(out, TIFFTAG_IMAGEDESCRIPTION, imagedescription);
	_NDPIfree(imagedescription);
	/*
	 * Choose tiles/strip for the output image according to
	 * the command line arguments (-tiles, -strips) and the
	 * structure of the input image.
	 */
		/* 65500 is the maximum image size in the standard
		 * JPEG library
		 */
	if (width >= 65500 || length >= 65500)
		thisouttiled = TRUE;
	if (thisouttiled == -1)
		thisouttiled = NDPIIsTiled(in);
	if (thisouttiled) {
		/*
		 * Setup output file's tile width&height.  If either
		 * is not specified, use either the value from the
		 * input image or, if nothing is defined, use the
		 * library default.
		 */
		if (tilewidth == (uint32_t) -1)
			NDPIGetField(in, TIFFTAG_TILEWIDTH, &tilewidth);
		if (tilelength == (uint32_t) -1)
			NDPIGetField(in, TIFFTAG_TILELENGTH, &tilelength);
		NDPIDefaultTileSize(out, &tilewidth, &tilelength);
		NDPISetField(out, TIFFTAG_TILEWIDTH, tilewidth);
		NDPISetField(out, TIFFTAG_TILELENGTH, tilelength);
	} else {
		/*
		 * RowsPerStrip is left unspecified: use either the
		 * value from the input image or, if nothing is defined,
		 * use the library default.
		 */
		if (rowsperstrip == (uint32_t) 0) {
			if (!NDPIGetField(in, TIFFTAG_ROWSPERSTRIP,
			    &rowsperstrip)) {
				rowsperstrip =
				    NDPIDefaultStripSize(out, rowsperstrip);
			}
			if (rowsperstrip > length && rowsperstrip != (uint32_t)-1)
				rowsperstrip = length;
		}
		else if (rowsperstrip == (uint32_t) -1)
			rowsperstrip = length;
		NDPISetField(out, TIFFTAG_ROWSPERSTRIP, rowsperstrip);
	}
	if (config != (uint16_t) -1)
		NDPISetField(out, TIFFTAG_PLANARCONFIG, config);
	else
		CopyField(TIFFTAG_PLANARCONFIG, config);
	if (samplesperpixel <= 4)
		CopyTag(TIFFTAG_TRANSFERFUNCTION, 4, TIFF_SHORT);
	CopyTag(TIFFTAG_COLORMAP, 4, TIFF_SHORT);
/* SMinSampleValue & SMaxSampleValue */
	switch (compression) {
		case COMPRESSION_JPEG:
			NDPISetField(out, TIFFTAG_JPEGQUALITY, quality);
			NDPISetField(out, TIFFTAG_JPEGCOLORMODE, jpegcolormode);
			break;
		case COMPRESSION_JBIG:
			CopyTag(TIFFTAG_FAXRECVPARAMS, 1, TIFF_LONG);
			CopyTag(TIFFTAG_FAXRECVTIME, 1, TIFF_LONG);
			CopyTag(TIFFTAG_FAXSUBADDRESS, 1, TIFF_ASCII);
			CopyTag(TIFFTAG_FAXDCS, 1, TIFF_ASCII);
			break;
		case COMPRESSION_LZW:
		case COMPRESSION_ADOBE_DEFLATE:
		case COMPRESSION_DEFLATE:
                case COMPRESSION_LZMA:
			if (predictor != (uint16_t)-1)
				NDPISetField(out, TIFFTAG_PREDICTOR, predictor);
			else
				CopyField(TIFFTAG_PREDICTOR, predictor);
			if (preset != -1) {
                                if (compression == COMPRESSION_ADOBE_DEFLATE
                                         || compression == COMPRESSION_DEFLATE)
                                        NDPISetField(out, TIFFTAG_ZIPQUALITY, preset);
				else if (compression == COMPRESSION_LZMA)
					NDPISetField(out, TIFFTAG_LZMAPRESET, preset);
                        }
			break;
		case COMPRESSION_CCITTFAX3:
		case COMPRESSION_CCITTFAX4:
			if (compression == COMPRESSION_CCITTFAX3) {
				if (g3opts != (uint32_t) -1)
					NDPISetField(out, TIFFTAG_GROUP3OPTIONS,
					    g3opts);
				else
					CopyField(TIFFTAG_GROUP3OPTIONS, g3opts);
			} else
				CopyTag(TIFFTAG_GROUP4OPTIONS, 1, TIFF_LONG);
			CopyTag(TIFFTAG_BADFAXLINES, 1, TIFF_LONG);
			CopyTag(TIFFTAG_CLEANFAXDATA, 1, TIFF_LONG);
			CopyTag(TIFFTAG_CONSECUTIVEBADFAXLINES, 1, TIFF_LONG);
			CopyTag(TIFFTAG_FAXRECVPARAMS, 1, TIFF_LONG);
			CopyTag(TIFFTAG_FAXRECVTIME, 1, TIFF_LONG);
			CopyTag(TIFFTAG_FAXSUBADDRESS, 1, TIFF_ASCII);
			break;
	}
	{
		uint32_t len32;
		void** data;
		if (NDPIGetField(in, TIFFTAG_ICCPROFILE, &len32, &data))
			NDPISetField(out, TIFFTAG_ICCPROFILE, len32, data);
	}
	{
		uint16_t ninks;
		const char* inknames;
		if (NDPIGetField(in, TIFFTAG_NUMBEROFINKS, &ninks)) {
			NDPISetField(out, TIFFTAG_NUMBEROFINKS, ninks);
			if (NDPIGetField(in, TIFFTAG_INKNAMES, &inknames)) {
				int inknameslen = strlen(inknames) + 1;
				const char* cp = inknames;
				while (ninks > 1) {
					cp = strchr(cp, '\0');
                                        cp++;
                                        inknameslen += (strlen(cp) + 1);
					ninks--;
				}
				NDPISetField(out, TIFFTAG_INKNAMES, inknameslen, inknames);
			}
		}
	}
	{
		unsigned short pg0, pg1;

		if (pageInSeq == 1) {
			if (pageNum < 0) /* only one input file */ {
				if (NDPIGetField(in, TIFFTAG_PAGENUMBER, &pg0, &pg1))
					NDPISetField(out, TIFFTAG_PAGENUMBER, pg0, pg1);
			} else
				NDPISetField(out, TIFFTAG_PAGENUMBER, pageNum++, 0);

		} else {
			if (NDPIGetField(in, TIFFTAG_PAGENUMBER, &pg0, &pg1)) {
				if (pageNum < 0) /* only one input file */
					NDPISetField(out, TIFFTAG_PAGENUMBER, pg0, pg1);
				else
					NDPISetField(out, TIFFTAG_PAGENUMBER, pageNum++, 0);
			}
		}
	}

	for (p = tags; p < &tags[NTAGS]; p++)
		CopyTag(p->tag, p->count, p->type);

	cf = pickCopyFunc(in, out, bitspersample, samplesperpixel);
	return (cf ? (*cf)(in, out, length, width, samplesperpixel) : FALSE);
}

/*
 * Copy Functions.
 */
#define	DECLAREcpFunc(x) \
static int x(TIFF* in, TIFF* out, \
    uint32_t imagelength, uint32_t imagewidth, tsample_t spp)

#define	DECLAREreadFunc(x) \
static int x(TIFF* in, \
    uint8_t* buf, uint32_t firstrow, uint32_t lengthtoread, uint32_t imagewidth, tsample_t spp)
typedef int (*readFunc)(TIFF*, uint8_t*, uint32_t, uint32_t, uint32_t, tsample_t);

#define	DECLAREwriteFunc(x) \
static int x(TIFF* out, \
    uint8_t* buf, uint32_t firstrow, uint32_t lengthtowrite, uint32_t imagewidth, tsample_t spp)
typedef int (*writeFunc)(TIFF*, uint8_t*, uint32_t, uint32_t, uint32_t, tsample_t);

/*
 * Contig -> contig by scanline for rows/strip change.
 */
DECLAREcpFunc(cpContig2ContigByRow)
{
	tsize_t scanlinesize = NDPIScanlineSize(in);
	tdata_t buf;
	uint32_t row;

	buf = _NDPImalloc(scanlinesize);
	if (!buf)
		return 0;
	_NDPImemset(buf, 0, scanlinesize);
	(void) imagewidth; (void) spp;
	for (row = 0; row < imagelength; row++) {
		if (NDPIReadScanline(in, buf, row, 0) < 0 && !ignore) {
			NDPIError(NDPIFileName(in),
				  "Error, can't read scanline " 
				  TIFF_UINT32_FORMAT,
				  row);
			goto bad;
		}
		if (NDPIWriteScanline(out, buf, row, 0) < 0) {
			NDPIError(NDPIFileName(out),
				  "Error, can't write scanline " 
				  TIFF_UINT32_FORMAT,
				  row);
			goto bad;
		}
	}
	_NDPIfree(buf);
	return 1;
bad:
	_NDPIfree(buf);
	return 0;
}


typedef void biasFn (void *image, void *bias, uint32_t pixels);

#define subtract(bits) \
static void subtract##bits (void *i, void *b, uint32_t pixels)\
{\
   uint##bits##_t *image = i;\
   uint##bits##_t *bias = b;\
   while (pixels--) {\
     *image = *image > *bias ? *image-*bias : 0;\
     image++, bias++; \
   } \
}

subtract(8)
subtract(16)
subtract(32)

static biasFn *lineSubtractFn (unsigned bits)
{
	switch (bits) {
		case  8:  return subtract8;
		case 16:  return subtract16;
		case 32:  return subtract32;
	}
	return NULL;
}

/*
 * Contig -> contig by scanline while subtracting a bias image.
 */
DECLAREcpFunc(cpBiasedContig2Contig)
{
	if (spp == 1) {
		tsize_t biasSize = NDPIScanlineSize(bias);
		tsize_t bufSize = NDPIScanlineSize(in);
		tdata_t buf, biasBuf;
		uint32_t biasWidth = 0, biasLength = 0;
		NDPIGetField(bias, TIFFTAG_IMAGEWIDTH, &biasWidth);
		NDPIGetField(bias, TIFFTAG_IMAGELENGTH, &biasLength);
		if (biasSize == bufSize &&
		    imagelength == biasLength && imagewidth == biasWidth) {
			uint16_t sampleBits = 0;
			biasFn *subtractLine;
			NDPIGetField(in, TIFFTAG_BITSPERSAMPLE, &sampleBits);
			subtractLine = lineSubtractFn (sampleBits);
			if (subtractLine) {
				uint32_t row;
				buf = _NDPImalloc(bufSize);
				biasBuf = _NDPImalloc(bufSize);
				for (row = 0; row < imagelength; row++) {
					if (NDPIReadScanline(in, buf, row, 0) < 0
					    && !ignore) {
						NDPIError(NDPIFileName(in),
						    "Error, can't read scanline "
						    TIFF_UINT32_FORMAT,
						    row);
						goto bad;
					}
					if (NDPIReadScanline(bias, biasBuf, row, 0) < 0
					    && !ignore) {
						NDPIError(NDPIFileName(in),
						    "Error, can't read biased scanline "
						    TIFF_UINT32_FORMAT,
						    row);
						goto bad;
					}
					subtractLine (buf, biasBuf, imagewidth);
					if (NDPIWriteScanline(out, buf, row, 0) < 0) {
						NDPIError(NDPIFileName(out),
						    "Error, can't write scanline "
						    TIFF_UINT32_FORMAT,
						    row);
						goto bad;
					}
				}

				_NDPIfree(buf);
				_NDPIfree(biasBuf);
				NDPISetDirectory(bias,
				    NDPICurrentDirectory(bias)); /* rewind */
				return 1;
bad:
				_NDPIfree(buf);
				_NDPIfree(biasBuf);
				return 0;
			} else {
				NDPIError(NDPIFileName(in),
				    "No support for biasing %d bit pixels\n",
				    sampleBits);
				return 0;
			}
		}
		NDPIError(NDPIFileName(in),
		    "Bias image %s,%d\nis not the same size as %s,%d\n",
		    NDPIFileName(bias), NDPICurrentDirectory(bias),
		    NDPIFileName(in), NDPICurrentDirectory(in));
		return 0;
	} else {
		NDPIError(NDPIFileName(in),
		    "Can't bias %s,%d as it has >1 Sample/Pixel\n",
		    NDPIFileName(in), NDPICurrentDirectory(in));
		return 0;
	}

}


/*
 * Strip -> strip for change in encoding.
 */
DECLAREcpFunc(cpDecodedStrips)
{
	tsize_t stripsize  = NDPIStripSize(in);
	tdata_t buf = _NDPImalloc(stripsize);

	(void) imagewidth; (void) spp;
	if (buf) {
		tstrip_t s, ns = NDPINumberOfStrips(in);
		uint32_t row = 0;
		_NDPImemset(buf, 0, stripsize);
		for (s = 0; s < ns; s++) {
			tsize_t cc = (row + rowsperstrip > imagelength) ?
			    NDPIVStripSize(in, imagelength - row) : stripsize;
			if (NDPIReadEncodedStrip(in, s, buf, cc) < 0
			    && !ignore) {
				NDPIError(NDPIFileName(in),
				    "Error, can't read strip "
				    TIFF_UINT32_FORMAT,
				    s);
				goto bad;
			}
			if (NDPIWriteEncodedStrip(out, s, buf, cc) < 0) {
				NDPIError(NDPIFileName(out),
				    "Error, can't write strip "
				    TIFF_UINT32_FORMAT,
				    s);
				goto bad;
			}
			row += rowsperstrip;
		}
		_NDPIfree(buf);
		return 1;
	} else {
		NDPIError(NDPIFileName(in),
		    "Error, can't allocate memory buffer of size "
		    "%"TIFF_SSIZE_FORMAT " to read strips",
		    stripsize);
		return 0;
	}

bad:
	_NDPIfree(buf);
	return 0;
}

/*
 * Separate -> separate by row for rows/strip change.
 */
DECLAREcpFunc(cpSeparate2SeparateByRow)
{
	tsize_t scanlinesize = NDPIScanlineSize(in);
	tdata_t buf;
	uint32_t row;
	tsample_t s;

	(void) imagewidth;
	buf = _NDPImalloc(scanlinesize);
	if (!buf)
		return 0;
	_NDPImemset(buf, 0, scanlinesize);
	for (s = 0; s < spp; s++) {
		for (row = 0; row < imagelength; row++) {
			if (NDPIReadScanline(in, buf, row, s) < 0 && !ignore) {
				NDPIError(NDPIFileName(in),
				    "Error, can't read scanline "
				    TIFF_UINT32_FORMAT,
				    row);
				goto bad;
			}
			if (NDPIWriteScanline(out, buf, row, s) < 0) {
				NDPIError(NDPIFileName(out),
				    "Error, can't write scanline "
				    TIFF_UINT32_FORMAT,
				    row);
				goto bad;
			}
		}
	}
	_NDPIfree(buf);
	return 1;
bad:
	_NDPIfree(buf);
	return 0;
}

/*
 * Contig -> separate by row.
 */
DECLAREcpFunc(cpContig2SeparateByRow)
{
	tsize_t scanlinesizein = NDPIScanlineSize(in);
	tsize_t scanlinesizeout = NDPIScanlineSize(out);
	tdata_t inbuf;
	tdata_t outbuf;
	register uint8_t *inp, *outp;
	register uint32_t n;
	uint32_t row;
	tsample_t s;

	inbuf = _NDPImalloc(scanlinesizein);
	outbuf = _NDPImalloc(scanlinesizeout);
	if (!inbuf || !outbuf)
		return 0;
	_NDPImemset(inbuf, 0, scanlinesizein);
	_NDPImemset(outbuf, 0, scanlinesizeout);
	/* unpack channels */
	for (s = 0; s < spp; s++) {
		for (row = 0; row < imagelength; row++) {
			if (NDPIReadScanline(in, inbuf, row, 0) < 0
			    && !ignore) {
				NDPIError(NDPIFileName(in),
				    "Error, can't read scanline "
				    TIFF_UINT32_FORMAT,
				    row);
				goto bad;
			}
			inp = ((uint8_t*)inbuf) + s;
			outp = (uint8_t*)outbuf;
			for (n = imagewidth; n-- > 0;) {
				*outp++ = *inp;
				inp += spp;
			}
			if (NDPIWriteScanline(out, outbuf, row, s) < 0) {
				NDPIError(NDPIFileName(out),
				    "Error, can't write scanline "
				    TIFF_UINT32_FORMAT,
				    row);
				goto bad;
			}
		}
	}
	if (inbuf) _NDPIfree(inbuf);
	if (outbuf) _NDPIfree(outbuf);
	return 1;
bad:
	if (inbuf) _NDPIfree(inbuf);
	if (outbuf) _NDPIfree(outbuf);
	return 0;
}

/*
 * Separate -> contig by row.
 */
DECLAREcpFunc(cpSeparate2ContigByRow)
{
	tsize_t scanlinesizein = NDPIScanlineSize(in);
	tsize_t scanlinesizeout = NDPIScanlineSize(out);
	tdata_t inbuf;
	tdata_t outbuf;
	register uint8_t *inp, *outp;
	register uint32_t n;
	uint32_t row;
	tsample_t s;

	inbuf = _NDPImalloc(scanlinesizein);
	outbuf = _NDPImalloc(scanlinesizeout);
	if (!inbuf || !outbuf)
		return 0;
	_NDPImemset(inbuf, 0, scanlinesizein);
	_NDPImemset(outbuf, 0, scanlinesizeout);
	for (row = 0; row < imagelength; row++) {
		/* merge channels */
		for (s = 0; s < spp; s++) {
			if (NDPIReadScanline(in, inbuf, row, s) < 0
			    && !ignore) {
				NDPIError(NDPIFileName(in),
				    "Error, can't read scanline "
				    TIFF_UINT32_FORMAT,
				    row);
				goto bad;
			}
			inp = (uint8_t*)inbuf;
			outp = ((uint8_t*)outbuf) + s;
			for (n = imagewidth; n-- > 0;) {
				*outp = *inp++;
				outp += spp;
			}
		}
		if (NDPIWriteScanline(out, outbuf, row, 0) < 0) {
			NDPIError(NDPIFileName(out),
			    "Error, can't write scanline "
			    TIFF_UINT32_FORMAT,
			    row);
			goto bad;
		}
	}
	if (inbuf) _NDPIfree(inbuf);
	if (outbuf) _NDPIfree(outbuf);
	return 1;
bad:
	if (inbuf) _NDPIfree(inbuf);
	if (outbuf) _NDPIfree(outbuf);
	return 0;
}

static void
cpStripToTile(uint8_t* out, uint8_t* in,
    uint32_t rows, uint32_t cols, int outskew, int inskew)
{
	while (rows-- > 0) {
		uint32_t j = cols;
		while (j-- > 0)
			*out++ = *in++;
		out += outskew;
		in += inskew;
	}
}

static void
cpContigBufToSeparateBuf(uint8_t* out, uint8_t* in,
    uint32_t rows, uint32_t cols, int outskew, int inskew, tsample_t spp,
    int bytes_per_sample )
{
	while (rows-- > 0) {
		uint32_t j = cols;
		while (j-- > 0)
		{
			int n = bytes_per_sample;

			while( n-- ) {
				*out++ = *in++;
			}
			in += (spp-1) * bytes_per_sample;
		}
		out += outskew;
		in += inskew;
	}
}

static void
cpSeparateBufToContigBuf(uint8_t* out, uint8_t* in,
    uint32_t rows, uint32_t cols, int outskew, int inskew, tsample_t spp,
    int bytes_per_sample)
{
	while (rows-- > 0) {
		uint32_t j = cols;
		while (j-- > 0) {
			int n = bytes_per_sample;

			while( n-- ) {
				*out++ = *in++;
			}
			out += (spp-1)*bytes_per_sample;
		}
		out += outskew;
		in += inskew;
	}
}

static int
cpImage(TIFF* in, TIFF* out, readFunc fin, writeFunc fout,
	uint32_t imagelength, uint32_t imagewidth, tsample_t spp)
{
	int status = 0;
	tdata_t buf = NULL;
	tsize_t scanlinesize = NDPIRasterScanlineSize(in);
	/* tilelength is != -1 if and only if the output image is tiled */
	tsize_t bufferlength = tilelength == (uint32_t) -1 ? imagelength : tilelength;
	tsize_t bytes = scanlinesize * bufferlength;
	/*
	 * XXX: Check for integer overflow.
	 */
	if (scanlinesize
	    && bufferlength
	    && bytes / (tsize_t)bufferlength == scanlinesize) {
		buf = _NDPImalloc(bytes);
		if (buf) {
			tsize_t lengthtodo= imagelength;
			for (; lengthtodo >= bufferlength ; lengthtodo -= bufferlength)
				if ((*fin)(in, (uint8_t*)buf, 
					imagelength-lengthtodo,
					bufferlength, imagewidth, spp)) {
					status = (*fout)(out, (uint8_t*)buf,
						imagelength-lengthtodo,
						bufferlength, imagewidth, spp);
				}
			if (status && lengthtodo > 0)
				if ((*fin)(in, (uint8_t*)buf,
					imagelength-lengthtodo,
					lengthtodo, imagewidth, spp)) {
					status = (*fout)(out, (uint8_t*)buf,
						imagelength-lengthtodo,
						lengthtodo, imagewidth, spp);
				}
			_NDPIfree(buf);
		} else {
			NDPIError(NDPIFileName(in),
			    "Error, can't allocate space for image buffer");
		}
	} else {
		NDPIError(NDPIFileName(in), "Error, no space for image buffer");
	}

	return status;
}

DECLAREreadFunc(readContigStripsIntoBuffer)
{
	tsize_t scanlinesize = NDPIScanlineSize(in);
	uint8_t* bufp = buf;
	uint32_t row;

	(void) imagewidth; (void) spp;
	for (row = firstrow; row < firstrow + lengthtoread; row++) {
		if (NDPIReadScanline(in, (tdata_t) bufp, row, 0) < 0
		    && !ignore) {
			NDPIError(NDPIFileName(in),
			    "Error, can't read scanline "
			    TIFF_UINT32_FORMAT,
			    row);
			return 0;
		}
		bufp += scanlinesize;
	}

	return 1;
}

DECLAREreadFunc(readSeparateStripsIntoBuffer)
{
	int status = 1;
	tsize_t scanlinesize = NDPIScanlineSize(in);
	tdata_t scanline;
	if (!scanlinesize)
		return 0;

	scanline = _NDPImalloc(scanlinesize);
	if (!scanline)
		return 0;
	_NDPImemset(scanline, 0, scanlinesize);
	(void) imagewidth;
	if (scanline) {
		uint8_t* bufp = (uint8_t*) buf;
		uint32_t row;
		tsample_t s;
		for (row = firstrow; row < firstrow + lengthtoread; row++) {
			/* merge channels */
			for (s = 0; s < spp; s++) {
				uint8_t* bp = bufp + s;
				tsize_t n = scanlinesize;
				uint8_t* sbuf = scanline;

				if (NDPIReadScanline(in, scanline, row, s) < 0
				    && !ignore) {
					NDPIError(NDPIFileName(in),
					    "Error, can't read scanline "
					    TIFF_UINT32_FORMAT,
					    row);
					    status = 0;
					goto done;
				}
				while (n-- > 0)
					*bp = *sbuf++, bp += spp;
			}
			bufp += scanlinesize * spp;
		}
	}

done:
	_NDPIfree(scanline);
	return status;
}

DECLAREreadFunc(readContigTilesIntoBuffer)
{
	int status = 1;
	tsize_t tilesize = NDPITileSize(in);
	tdata_t tilebuf;
	uint32_t imagew = NDPIScanlineSize(in);
	uint32_t tilew  = NDPITileRowSize(in);
	int iskew = imagew - tilew;
	uint8_t* bufp = (uint8_t*) buf;
	uint32_t tw, tl;
	uint32_t row;

	(void) spp;
	tilebuf = _NDPImalloc(tilesize);
	if (tilebuf == 0)
		return 0;
	_NDPImemset(tilebuf, 0, tilesize);
	(void) NDPIGetField(in, TIFFTAG_TILEWIDTH, &tw);
	(void) NDPIGetField(in, TIFFTAG_TILELENGTH, &tl);
        
	for (row = firstrow; row < firstrow + lengthtoread; row += tl) {
		uint32_t nrow = (row+tl > firstrow+lengthtoread) ?
			firstrow+lengthtoread-row : tl;
		uint32_t colb = 0;
		uint32_t col;

		for (col = 0; col < imagewidth; col += tw) {
			if (NDPIReadTile(in, tilebuf, col, row, 0, 0) < 0
			    && !ignore) {
				NDPIError(NDPIFileName(in),
				    "Error, can't read tile at " 
				    TIFF_UINT32_FORMAT " "
				    TIFF_UINT32_FORMAT,
				    col, row);
				status = 0;
				goto done;
			}
			if (colb + tilew > imagew) {
				uint32_t width = imagew - colb;
				uint32_t oskew = tilew - width;
				/* CD: check this */
				cpStripToTile(bufp + colb,
				    tilebuf, nrow, width,
				    oskew + iskew, oskew );
			} else
				/* CD: check this */
				cpStripToTile(bufp + colb,
				    tilebuf, nrow, tilew,
				    iskew, 0);
			colb += tilew;
		}
		bufp += imagew * nrow;
	}
done:
	_NDPIfree(tilebuf);
	return status;
}

DECLAREreadFunc(readSeparateTilesIntoBuffer)
{
	int status = 1;
	uint32_t imagew = NDPIRasterScanlineSize(in);
	uint32_t tilew = NDPITileRowSize(in);
	int iskew  = imagew - tilew*spp;
	tsize_t tilesize = NDPITileSize(in);
	tdata_t tilebuf;
	uint8_t* bufp = (uint8_t*) buf;
	uint32_t tw, tl;
	uint32_t row;
	uint16_t bps, bytes_per_sample;

	tilebuf = _NDPImalloc(tilesize);
	if (tilebuf == 0)
		return 0;
	_NDPImemset(tilebuf, 0, tilesize);
	(void) NDPIGetField(in, TIFFTAG_TILEWIDTH, &tw);
	(void) NDPIGetField(in, TIFFTAG_TILELENGTH, &tl);
	(void) NDPIGetField(in, TIFFTAG_BITSPERSAMPLE, &bps);
	assert( bps % 8 == 0 );
	bytes_per_sample = bps/8;

	for (row = firstrow; row < firstrow + lengthtoread; row += tl) {
		uint32_t nrow = (row+tl > firstrow+lengthtoread) ?
			firstrow+lengthtoread-row : tl;
		uint32_t colb = 0;
		uint32_t col;

		for (col = 0; col < imagewidth; col += tw) {
			tsample_t s;

			for (s = 0; s < spp; s++) {
				if (NDPIReadTile(in, tilebuf, col, row, 0, s) < 0
				    && !ignore) {
					NDPIError(NDPIFileName(in),
					    "Error, can't read tile at "
					    TIFF_UINT32_FORMAT " "
					    TIFF_UINT32_FORMAT ", "
					    "sample %u",
					    col, row, s);
					status = 0;
					goto done;
				}
				/*
				 * Tile is clipped horizontally.  Calculate
				 * visible portion and skewing factors.
				 */
				if (colb + tilew*spp > imagew) {
					uint32_t width = imagew - colb;
					int oskew = tilew*spp - width;
					/* CD: check this */
					cpSeparateBufToContigBuf(
					    bufp+colb+s*bytes_per_sample,
					    tilebuf, nrow,
					    width/(spp*bytes_per_sample),
					    oskew + iskew,
					    oskew/spp, spp,
					    bytes_per_sample);
				} else
					/* CD: check this */
					cpSeparateBufToContigBuf(
					    bufp+colb+s*bytes_per_sample,
					    tilebuf, nrow, tw,
					    iskew, 0, spp,
					    bytes_per_sample);
			}
			colb += tilew*spp;
		}
		bufp += imagew * nrow;
	}
done:
	_NDPIfree(tilebuf);
	return status;
}

DECLAREwriteFunc(writeBufferToContigStrips)
{
	uint32_t row, rowsperstrip;
	tstrip_t strip = 0;

	(void) imagewidth; (void) spp;
	(void) NDPIGetFieldDefaulted(out, TIFFTAG_ROWSPERSTRIP, &rowsperstrip);
	for (row = firstrow; row < firstrow+lengthtowrite; row += rowsperstrip) {
		uint32_t nrows = (row+rowsperstrip > firstrow+lengthtowrite) ?
			firstrow+lengthtowrite-row : rowsperstrip;
		tsize_t stripsize = NDPIVStripSize(out, nrows);
		if (NDPIWriteEncodedStrip(out, strip++, buf, stripsize) < 0) {
			NDPIError(NDPIFileName(out),
			    "Error, can't write strip "
			    TIFF_UINT32_FORMAT,
			    strip - 1);
			return 0;
		}
		buf += stripsize;
	}
	return 1;
}

DECLAREwriteFunc(writeBufferToSeparateStrips)
{
	uint32_t rowsize = imagewidth * spp;
	uint32_t rowsperstrip;
	tsize_t stripsize = NDPIStripSize(out);
	tdata_t obuf;
	tstrip_t strip = 0;
	tsample_t s;

	obuf = _NDPImalloc(stripsize);
	if (obuf == NULL)
		return (0);
	_NDPImemset(obuf, 0, stripsize);
	(void) NDPIGetFieldDefaulted(out, TIFFTAG_ROWSPERSTRIP, &rowsperstrip);
	for (s = 0; s < spp; s++) {
		uint32_t row;
		for (row = firstrow; row < firstrow+lengthtowrite; row += rowsperstrip) {
			uint32_t nrows = (row+rowsperstrip > firstrow+lengthtowrite) ?
			    firstrow+lengthtowrite-row : rowsperstrip;
			tsize_t stripsize = NDPIVStripSize(out, nrows);

			cpContigBufToSeparateBuf(
			    obuf, (uint8_t*) buf + row*rowsize + s,
			    nrows, imagewidth, 0, 0, spp, 1);
			if (NDPIWriteEncodedStrip(out, strip++, obuf, stripsize) < 0) {
				NDPIError(NDPIFileName(out),
				    "Error, can't write strip "
				    TIFF_UINT32_FORMAT,
				    strip - 1);
				_NDPIfree(obuf);
				return 0;
			}
		}
	}
	_NDPIfree(obuf);
	return 1;

}

DECLAREwriteFunc(writeBufferToContigTiles)
{
	uint32_t imagew = NDPIScanlineSize(out);
	uint32_t tilew  = NDPITileRowSize(out);
	int iskew = imagew - tilew;
	tsize_t tilesize = NDPITileSize(out);
	tdata_t obuf;
	uint8_t* bufp = (uint8_t*) buf;
	uint32_t tl, tw;
	uint32_t row;

	(void) spp;

	obuf = _NDPImalloc(tilesize);
	if (obuf == NULL)
		return 0;
	_NDPImemset(obuf, 0, tilesize);
	(void) NDPIGetField(out, TIFFTAG_TILELENGTH, &tl);
	(void) NDPIGetField(out, TIFFTAG_TILEWIDTH, &tw);
	for (row = firstrow; row < firstrow+lengthtowrite; row += tilelength) {
		uint32_t nrow = (row+tl > firstrow+lengthtowrite) ?
			firstrow+lengthtowrite-row : tl;
		uint32_t colb = 0;
		uint32_t col;

		for (col = 0; col < imagewidth; col += tw) {
			/*
			 * Tile is clipped horizontally.  Calculate
			 * visible portion and skewing factors.
			 */
			if (colb + tilew > imagew) {
				uint32_t width = imagew - colb;
				int oskew = tilew - width;
				cpStripToTile(obuf, bufp + colb, nrow, width,
				    oskew, oskew + iskew);
			} else
				cpStripToTile(obuf, bufp + colb, nrow, tilew,
				    0, iskew);
			if (NDPIWriteTile(out, obuf, col, row, 0, 0) < 0) {
				NDPIError(NDPIFileName(out),
				    "Error, can't write tile at "
				    TIFF_UINT32_FORMAT " " 
				    TIFF_UINT32_FORMAT,
				    col, row);
				_NDPIfree(obuf);
				return 0;
			}
			colb += tilew;
		}
		bufp += nrow * imagew;
	}
	_NDPIfree(obuf);
	return 1;
}

DECLAREwriteFunc(writeBufferToSeparateTiles)
{
	uint32_t imagew = NDPIScanlineSize(out);
	tsize_t tilew  = NDPITileRowSize(out);
	uint32_t iimagew = NDPIRasterScanlineSize(out);
	int iskew = iimagew - tilew*spp;
	tsize_t tilesize = NDPITileSize(out);
	tdata_t obuf;
	uint8_t* bufp = (uint8_t*) buf;
	uint32_t tl, tw;
	uint32_t row;
	uint16_t bps, bytes_per_sample;

	obuf = _NDPImalloc(tilesize);
	if (obuf == NULL)
		return 0;
	_NDPImemset(obuf, 0, tilesize);
	(void) NDPIGetField(out, TIFFTAG_TILELENGTH, &tl);
	(void) NDPIGetField(out, TIFFTAG_TILEWIDTH, &tw);
	(void) NDPIGetField(out, TIFFTAG_BITSPERSAMPLE, &bps);
	assert( bps % 8 == 0 );
	bytes_per_sample = bps/8;

	for (row = firstrow; row < firstrow+lengthtowrite; row += tl) {
		uint32_t nrow = (row+tl > firstrow+lengthtowrite) ? firstrow+lengthtowrite-row : tl;
		uint32_t colb = 0;
		uint32_t col;

		for (col = 0; col < imagewidth; col += tw) {
			tsample_t s;
			for (s = 0; s < spp; s++) {
				/*
				 * Tile is clipped horizontally.  Calculate
				 * visible portion and skewing factors.
				 */
				if (colb + tilew > imagew) {
					uint32_t width = (imagew - colb);
					int oskew = tilew - width;

					cpContigBufToSeparateBuf(obuf,
					    bufp + (colb*spp) + s,
					    nrow, width/bytes_per_sample,
					    oskew, (oskew*spp)+iskew, spp,
					    bytes_per_sample);
				} else
					cpContigBufToSeparateBuf(obuf,
					    bufp + (colb*spp) + s,
					    nrow, tilewidth,
					    0, iskew, spp,
					    bytes_per_sample);
				if (NDPIWriteTile(out, obuf, col, row, 0, s) < 0) {
					NDPIError(NDPIFileName(out),
					    "Error, can't write tile at "
					    TIFF_UINT32_FORMAT " " 
					    TIFF_UINT32_FORMAT
					    " sample %u",
					    col, row, s);
					_NDPIfree(obuf);
					return 0;
				}
			}
			colb += tilew;
		}
		bufp += nrow * iimagew;
	}
	_NDPIfree(obuf);
	return 1;
}

/*
 * Contig strips -> contig tiles.
 */
DECLAREcpFunc(cpContigStrips2ContigTiles)
{
	return cpImage(in, out,
	    readContigStripsIntoBuffer,
	    writeBufferToContigTiles,
	    imagelength, imagewidth, spp);
}

/*
 * Contig strips -> separate tiles.
 */
DECLAREcpFunc(cpContigStrips2SeparateTiles)
{
	return cpImage(in, out,
	    readContigStripsIntoBuffer,
	    writeBufferToSeparateTiles,
	    imagelength, imagewidth, spp);
}

/*
 * Separate strips -> contig tiles.
 */
DECLAREcpFunc(cpSeparateStrips2ContigTiles)
{
	return cpImage(in, out,
	    readSeparateStripsIntoBuffer,
	    writeBufferToContigTiles,
	    imagelength, imagewidth, spp);
}

/*
 * Separate strips -> separate tiles.
 */
DECLAREcpFunc(cpSeparateStrips2SeparateTiles)
{
	return cpImage(in, out,
	    readSeparateStripsIntoBuffer,
	    writeBufferToSeparateTiles,
	    imagelength, imagewidth, spp);
}

/*
 * Contig strips -> contig tiles.
 */
DECLAREcpFunc(cpContigTiles2ContigTiles)
{
	return cpImage(in, out,
	    readContigTilesIntoBuffer,
	    writeBufferToContigTiles,
	    imagelength, imagewidth, spp);
}

/*
 * Contig tiles -> separate tiles.
 */
DECLAREcpFunc(cpContigTiles2SeparateTiles)
{
	return cpImage(in, out,
	    readContigTilesIntoBuffer,
	    writeBufferToSeparateTiles,
	    imagelength, imagewidth, spp);
}

/*
 * Separate tiles -> contig tiles.
 */
DECLAREcpFunc(cpSeparateTiles2ContigTiles)
{
	return cpImage(in, out,
	    readSeparateTilesIntoBuffer,
	    writeBufferToContigTiles,
	    imagelength, imagewidth, spp);
}

/*
 * Separate tiles -> separate tiles (tile dimension change).
 */
DECLAREcpFunc(cpSeparateTiles2SeparateTiles)
{
	return cpImage(in, out,
	    readSeparateTilesIntoBuffer,
	    writeBufferToSeparateTiles,
	    imagelength, imagewidth, spp);
}

/*
 * Contig tiles -> contig tiles (tile dimension change).
 */
DECLAREcpFunc(cpContigTiles2ContigStrips)
{
	return cpImage(in, out,
	    readContigTilesIntoBuffer,
	    writeBufferToContigStrips,
	    imagelength, imagewidth, spp);
}

/*
 * Contig tiles -> separate strips.
 */
DECLAREcpFunc(cpContigTiles2SeparateStrips)
{
	return cpImage(in, out,
	    readContigTilesIntoBuffer,
	    writeBufferToSeparateStrips,
	    imagelength, imagewidth, spp);
}

/*
 * Separate tiles -> contig strips.
 */
DECLAREcpFunc(cpSeparateTiles2ContigStrips)
{
	return cpImage(in, out,
	    readSeparateTilesIntoBuffer,
	    writeBufferToContigStrips,
	    imagelength, imagewidth, spp);
}

/*
 * Separate tiles -> separate strips.
 */
DECLAREcpFunc(cpSeparateTiles2SeparateStrips)
{
	return cpImage(in, out,
	    readSeparateTilesIntoBuffer,
	    writeBufferToSeparateStrips,
	    imagelength, imagewidth, spp);
}

/*
 * Select the appropriate copy function to use.
 */
static copyFunc
pickCopyFunc(TIFF* in, TIFF* out, uint16_t bitspersample, uint16_t samplesperpixel)
{
	uint16_t shortv;
	uint32_t w, l, tw, tl;
	int bychunk;

	(void) NDPIGetField(in, TIFFTAG_PLANARCONFIG, &shortv);
	if (shortv != config && bitspersample != 8 && samplesperpixel > 1) {
		fprintf(stderr,
		    "%s: Cannot handle different planar configuration w/ bits/sample != 8\n",
		    NDPIFileName(in));
		return (NULL);
	}
	NDPIGetField(in, TIFFTAG_IMAGEWIDTH, &w);
	NDPIGetField(in, TIFFTAG_IMAGELENGTH, &l);
	if (!(NDPIIsTiled(out) || NDPIIsTiled(in))) {
		uint32_t irps = (uint32_t) -1L;
		NDPIGetField(in, TIFFTAG_ROWSPERSTRIP, &irps);
		/* if biased, force decoded copying to allow image subtraction */
		bychunk = !bias && (rowsperstrip == irps);
	}else{  /* either in or out is tiled */
		if (bias) {
			fprintf(stderr,
			    "%s: Cannot handle tiled configuration w/bias image\n",
			NDPIFileName(in));
			return (NULL);
		}
		if (NDPIIsTiled(out)) {
			if (!NDPIGetField(in, TIFFTAG_TILEWIDTH, &tw))
				tw = w;
			if (!NDPIGetField(in, TIFFTAG_TILELENGTH, &tl))
				tl = l;
			bychunk = (tw == tilewidth && tl == tilelength);
		} else {  /* out's not, so in must be tiled */
			NDPIGetField(in, TIFFTAG_TILEWIDTH, &tw);
			NDPIGetField(in, TIFFTAG_TILELENGTH, &tl);
			bychunk = (tw == w && tl == rowsperstrip);
		}
	}
#define	T 1
#define	F 0
#define pack(a,b,c,d,e)	((long)(((a)<<11)|((b)<<3)|((c)<<2)|((d)<<1)|(e)))
	switch(pack(shortv,config,NDPIIsTiled(in),NDPIIsTiled(out),bychunk)) {
		/* Strips -> Tiles */
		case pack(PLANARCONFIG_CONTIG,   PLANARCONFIG_CONTIG,   F,T,F):
		case pack(PLANARCONFIG_CONTIG,   PLANARCONFIG_CONTIG,   F,T,T):
			return cpContigStrips2ContigTiles;
		case pack(PLANARCONFIG_CONTIG,   PLANARCONFIG_SEPARATE, F,T,F):
		case pack(PLANARCONFIG_CONTIG,   PLANARCONFIG_SEPARATE, F,T,T):
			return cpContigStrips2SeparateTiles;
		case pack(PLANARCONFIG_SEPARATE, PLANARCONFIG_CONTIG,   F,T,F):
		case pack(PLANARCONFIG_SEPARATE, PLANARCONFIG_CONTIG,   F,T,T):
			return cpSeparateStrips2ContigTiles;
		case pack(PLANARCONFIG_SEPARATE, PLANARCONFIG_SEPARATE, F,T,F):
		case pack(PLANARCONFIG_SEPARATE, PLANARCONFIG_SEPARATE, F,T,T):
			return cpSeparateStrips2SeparateTiles;
		/* Tiles -> Tiles */
		case pack(PLANARCONFIG_CONTIG,   PLANARCONFIG_CONTIG,   T,T,F):
		case pack(PLANARCONFIG_CONTIG,   PLANARCONFIG_CONTIG,   T,T,T):
			return cpContigTiles2ContigTiles;
		case pack(PLANARCONFIG_CONTIG,   PLANARCONFIG_SEPARATE, T,T,F):
		case pack(PLANARCONFIG_CONTIG,   PLANARCONFIG_SEPARATE, T,T,T):
			return cpContigTiles2SeparateTiles;
		case pack(PLANARCONFIG_SEPARATE, PLANARCONFIG_CONTIG,   T,T,F):
		case pack(PLANARCONFIG_SEPARATE, PLANARCONFIG_CONTIG,   T,T,T):
			return cpSeparateTiles2ContigTiles;
		case pack(PLANARCONFIG_SEPARATE, PLANARCONFIG_SEPARATE, T,T,F):
		case pack(PLANARCONFIG_SEPARATE, PLANARCONFIG_SEPARATE, T,T,T):
			return cpSeparateTiles2SeparateTiles;
		/* Tiles -> Strips */
		case pack(PLANARCONFIG_CONTIG,   PLANARCONFIG_CONTIG,   T,F,F):
		case pack(PLANARCONFIG_CONTIG,   PLANARCONFIG_CONTIG,   T,F,T):
			return cpContigTiles2ContigStrips;
		case pack(PLANARCONFIG_CONTIG,   PLANARCONFIG_SEPARATE, T,F,F):
		case pack(PLANARCONFIG_CONTIG,   PLANARCONFIG_SEPARATE, T,F,T):
			return cpContigTiles2SeparateStrips;
		case pack(PLANARCONFIG_SEPARATE, PLANARCONFIG_CONTIG,   T,F,F):
		case pack(PLANARCONFIG_SEPARATE, PLANARCONFIG_CONTIG,   T,F,T):
			return cpSeparateTiles2ContigStrips;
		case pack(PLANARCONFIG_SEPARATE, PLANARCONFIG_SEPARATE, T,F,F):
		case pack(PLANARCONFIG_SEPARATE, PLANARCONFIG_SEPARATE, T,F,T):
			return cpSeparateTiles2SeparateStrips;
		/* Strips -> Strips */
		case pack(PLANARCONFIG_CONTIG,   PLANARCONFIG_CONTIG,   F,F,F):
			return bias ? cpBiasedContig2Contig : cpContig2ContigByRow;
		case pack(PLANARCONFIG_CONTIG,   PLANARCONFIG_CONTIG,   F,F,T):
			return cpDecodedStrips;
		case pack(PLANARCONFIG_CONTIG, PLANARCONFIG_SEPARATE,   F,F,F):
		case pack(PLANARCONFIG_CONTIG, PLANARCONFIG_SEPARATE,   F,F,T):
			return cpContig2SeparateByRow;
		case pack(PLANARCONFIG_SEPARATE, PLANARCONFIG_CONTIG,   F,F,F):
		case pack(PLANARCONFIG_SEPARATE, PLANARCONFIG_CONTIG,   F,F,T):
			return cpSeparate2ContigByRow;
		case pack(PLANARCONFIG_SEPARATE, PLANARCONFIG_SEPARATE, F,F,F):
		case pack(PLANARCONFIG_SEPARATE, PLANARCONFIG_SEPARATE, F,F,T):
			return cpSeparate2SeparateByRow;
	}
#undef pack
#undef F
#undef T
	fprintf(stderr, "ndpi2tiff: %s: Don't know how to copy/convert image.\n",
	    NDPIFileName(in));
	return (NULL);
}

/* vim: set ts=8 sts=8 sw=8 noet: */
/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 8
 * fill-column: 78
 * End:
 */
