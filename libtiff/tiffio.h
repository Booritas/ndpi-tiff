/*
 * Copyright (c) 1988-1997 Sam Leffler
 * Copyright (c) 1991-1997 Silicon Graphics, Inc.
 *
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

#ifndef _TIFFIO_
#define	_TIFFIO_

/*
 * TIFF I/O Library Definitions.
 */
#include "tiff.h"
#include "tiffvers.h"

/*
 * TIFF is defined as an incomplete type to hide the
 * library's internal data structures from clients.
 */
typedef struct tiff TIFF;

/*
 * The following typedefs define the intrinsic size of
 * data types used in the *exported* interfaces.  These
 * definitions depend on the proper definition of types
 * in tiff.h.  Note also that the varargs interface used
 * to pass tag types and values uses the types defined in
 * tiff.h directly.
 *
 * NB: ttag_t is unsigned int and not unsigned short because
 *     ANSI C requires that the type before the ellipsis be a
 *     promoted type (i.e. one of int, unsigned int, pointer,
 *     or double) and because we defined pseudo-tags that are
 *     outside the range of legal Aldus-assigned tags.
 * NB: tsize_t is signed and not unsigned because some functions
 *     return -1.
 * NB: toff_t is not off_t for many reasons; TIFFs max out at
 *     32-bit file offsets, and BigTIFF maxes out at 64-bit
 *     offsets being the most important, and to ensure use of
 *     a consistently unsigned type across architectures.
 *     Prior to libtiff 4.0, this was an unsigned 32 bit type.
 */
/*
 * this is the machine addressing size type, only it's signed, so make it
 * int32_t on 32bit machines, int64_t on 64bit machines
 */
typedef TIFF_SSIZE_T tmsize_t;
#define TIFF_TMSIZE_T_MAX (tmsize_t)(SIZE_MAX >> 1)

typedef uint64_t toff_t;          /* file offset */
/* the following are deprecated and should be replaced by their defining
   counterparts */
typedef uint32_t ttag_t;          /* directory tag */
typedef uint16_t tdir_t;          /* directory index */
typedef uint16_t tsample_t;       /* sample number */
typedef uint32_t tstrile_t;       /* strip or tile number */
typedef tstrile_t tstrip_t;     /* strip number */
typedef tstrile_t ttile_t;      /* tile number */
typedef tmsize_t tsize_t;       /* i/o size in bytes */
typedef void* tdata_t;          /* image data ref */

#if !defined(__WIN32__) && (defined(_WIN32) || defined(WIN32))
#define __WIN32__
#endif

/*
 * On windows you should define USE_WIN32_FILEIO if you are using tif_win32.c
 * or AVOID_WIN32_FILEIO if you are using something else (like tif_unix.c).
 *
 * By default tif_unix.c is assumed.
 */

#if defined(_WINDOWS) || defined(__WIN32__) || defined(_Windows)
#  if !defined(__CYGWIN) && !defined(AVOID_WIN32_FILEIO) && !defined(USE_WIN32_FILEIO)
#    define AVOID_WIN32_FILEIO
#  endif
#endif

#if defined(USE_WIN32_FILEIO)
# define VC_EXTRALEAN
# include <windows.h>
# ifdef __WIN32__
DECLARE_HANDLE(thandle_t);     /* Win32 file handle */
# else
typedef HFILE thandle_t;       /* client data handle */
# endif /* __WIN32__ */
#else
typedef void* thandle_t;       /* client data handle */
#endif /* USE_WIN32_FILEIO */

/*
 * Flags to pass to NDPIPrintDirectory to control
 * printing of data structures that are potentially
 * very large.   Bit-or these flags to enable printing
 * multiple items.
 */
#define TIFFPRINT_NONE	       0x0    /* no extra info */
#define TIFFPRINT_STRIPS       0x1    /* strips/tiles info */
#define TIFFPRINT_CURVES       0x2    /* color/gray response curves */
#define TIFFPRINT_COLORMAP     0x4    /* colormap */
#define TIFFPRINT_JPEGQTABLES  0x100  /* JPEG Q matrices */
#define TIFFPRINT_JPEGACTABLES 0x200  /* JPEG AC tables */
#define TIFFPRINT_JPEGDCTABLES 0x200  /* JPEG DC tables */

/* 
 * Colour conversion stuff
 */

/* reference white */
#define D65_X0 (95.0470F)
#define D65_Y0 (100.0F)
#define D65_Z0 (108.8827F)

#define D50_X0 (96.4250F)
#define D50_Y0 (100.0F)
#define D50_Z0 (82.4680F)

/* Structure for holding information about a display device. */

typedef unsigned char TIFFRGBValue;               /* 8-bit samples */

typedef struct {
	float d_mat[3][3];                        /* XYZ -> luminance matrix */
	float d_YCR;                              /* Light o/p for reference white */
	float d_YCG;
	float d_YCB;
	uint32_t d_Vrwr;                            /* Pixel values for ref. white */
	uint32_t d_Vrwg;
	uint32_t d_Vrwb;
	float d_Y0R;                              /* Residual light for black pixel */
	float d_Y0G;
	float d_Y0B;
	float d_gammaR;                           /* Gamma values for the three guns */
	float d_gammaG;
	float d_gammaB;
} TIFFDisplay;

typedef struct {                                  /* YCbCr->RGB support */
	TIFFRGBValue* clamptab;                   /* range clamping table */
	int* Cr_r_tab;
	int* Cb_b_tab;
	int32_t* Cr_g_tab;
	int32_t* Cb_g_tab;
	int32_t* Y_tab;
} TIFFYCbCrToRGB;

typedef struct {                                  /* CIE Lab 1976->RGB support */
	int range;                                /* Size of conversion table */
#define CIELABTORGB_TABLE_RANGE 1500
	float rstep, gstep, bstep;
	float X0, Y0, Z0;                         /* Reference white point */
	TIFFDisplay display;
	float Yr2r[CIELABTORGB_TABLE_RANGE + 1];  /* Conversion of Yr to r */
	float Yg2g[CIELABTORGB_TABLE_RANGE + 1];  /* Conversion of Yg to g */
	float Yb2b[CIELABTORGB_TABLE_RANGE + 1];  /* Conversion of Yb to b */
} TIFFCIELabToRGB;

/*
 * RGBA-style image support.
 */
typedef struct _TIFFRGBAImage TIFFRGBAImage;
/*
 * The image reading and conversion routines invoke
 * ``put routines'' to copy/image/whatever tiles of
 * raw image data.  A default set of routines are 
 * provided to convert/copy raw image data to 8-bit
 * packed ABGR format rasters.  Applications can supply
 * alternate routines that unpack the data into a
 * different format or, for example, unpack the data
 * and draw the unpacked raster on the display.
 */
typedef void (*tileContigRoutine)
    (TIFFRGBAImage*, uint32_t*, uint32_t, uint32_t, uint32_t, uint32_t, int32_t, int32_t,
     unsigned char*);
typedef void (*tileSeparateRoutine)
    (TIFFRGBAImage*, uint32_t*, uint32_t, uint32_t, uint32_t, uint32_t, int32_t, int32_t,
     unsigned char*, unsigned char*, unsigned char*, unsigned char*);
/*
 * RGBA-reader state.
 */
struct _TIFFRGBAImage {
	TIFF* tif;                              /* image handle */
	int stoponerr;                          /* stop on read error */
	int isContig;                           /* data is packed/separate */
	int alpha;                              /* type of alpha data present */
	uint32_t width;                           /* image width */
	uint32_t height;                          /* image height */
	uint16_t bitspersample;                   /* image bits/sample */
	uint16_t samplesperpixel;                 /* image samples/pixel */
	uint16_t orientation;                     /* image orientation */
	uint16_t req_orientation;                 /* requested orientation */
	uint16_t photometric;                     /* image photometric interp */
	uint16_t* redcmap;                        /* colormap palette */
	uint16_t* greencmap;
	uint16_t* bluecmap;
	/* get image data routine */
	int (*get)(TIFFRGBAImage*, uint32_t*, uint32_t, uint32_t);
	/* put decoded strip/tile */
	union {
	    void (*any)(TIFFRGBAImage*);
	    tileContigRoutine contig;
	    tileSeparateRoutine separate;
	} put;
	TIFFRGBValue* Map;                      /* sample mapping array */
	uint32_t** BWmap;                         /* black&white map */
	uint32_t** PALmap;                        /* palette image map */
	TIFFYCbCrToRGB* ycbcr;                  /* YCbCr conversion state */
	TIFFCIELabToRGB* cielab;                /* CIE L*a*b conversion state */

	uint8_t* UaToAa;                          /* Unassociated alpha to associated alpha conversion LUT */
	uint8_t* Bitdepth16To8;                   /* LUT for conversion from 16bit to 8bit values */

	int row_offset;
	int col_offset;
};

/*
 * Macros for extracting components from the
 * packed ABGR form returned by NDPIReadRGBAImage.
 */
#define TIFFGetR(abgr) ((abgr) & 0xff)
#define TIFFGetG(abgr) (((abgr) >> 8) & 0xff)
#define TIFFGetB(abgr) (((abgr) >> 16) & 0xff)
#define TIFFGetA(abgr) (((abgr) >> 24) & 0xff)

/*
 * A CODEC is a software package that implements decoding,
 * encoding, or decoding+encoding of a compression algorithm.
 * The library provides a collection of builtin codecs.
 * More codecs may be registered through calls to the library
 * and/or the builtin implementations may be overridden.
 */
typedef int (*TIFFInitMethod)(TIFF*, int);
typedef struct {
	char* name;
	uint16_t scheme;
	TIFFInitMethod init;
} TIFFCodec;

#include <stdio.h>
#include <stdarg.h>

/* share internal LogLuv conversion routines? */
#ifndef LOGLUV_PUBLIC
#define LOGLUV_PUBLIC 1
#endif

#if defined(__GNUC__) || defined(__attribute__)
#  define TIFF_ATTRIBUTE(x)    __attribute__(x)
#else
#  define TIFF_ATTRIBUTE(x) /*nothing*/
#endif

#if defined(c_plusplus) || defined(__cplusplus)
extern "C" {
#endif
typedef void (*TIFFErrorHandler)(const char*, const char*, va_list);
typedef void (*TIFFErrorHandlerExt)(thandle_t, const char*, const char*, va_list);
typedef tmsize_t (*TIFFReadWriteProc)(thandle_t, void*, tmsize_t);
typedef toff_t (*TIFFSeekProc)(thandle_t, toff_t, int);
typedef int (*TIFFCloseProc)(thandle_t);
typedef toff_t (*TIFFSizeProc)(thandle_t);
typedef int (*TIFFMapFileProc)(thandle_t, void** base, toff_t* size);
typedef void (*TIFFUnmapFileProc)(thandle_t, void* base, toff_t size);
typedef void (*TIFFExtendProc)(TIFF*);

extern const char* TIFFGetVersion(void);

extern const TIFFCodec* NDPIFindCODEC(uint16_t);
extern TIFFCodec* NDPIRegisterCODEC(uint16_t, const char*, TIFFInitMethod);
extern void NDPIUnRegisterCODEC(TIFFCodec*);
extern int NDPIIsCODECConfigured(uint16_t);
extern TIFFCodec* NDPIGetConfiguredCODECs(void);

/*
 * Auxiliary functions.
 */

extern void* _NDPImalloc(tmsize_t s);
extern void* _TIFFcalloc(tmsize_t nmemb, tmsize_t siz);
extern void* _NDPIrealloc(void* p, tmsize_t s);
extern void _NDPImemset(void* p, int v, tmsize_t c);
extern void _NDPImemcpy(void* d, const void* s, tmsize_t c);
extern int _TIFFmemcmp(const void* p1, const void* p2, tmsize_t c);
extern void _NDPIfree(void* p);

/*
** Stuff, related to tag handling and creating custom tags.
*/
extern int NDPIGetTagListCount( TIFF * );
extern uint32_t NDPIGetTagListEntry(TIFF *, int tag_index );
    
#define TIFF_ANY       TIFF_NOTYPE     /* for field descriptor searching */
#define TIFF_VARIABLE  -1              /* marker for variable length tags */
#define TIFF_SPP       -2              /* marker for SamplesPerPixel tags */
#define TIFF_VARIABLE2 -3              /* marker for uint32_t var-length tags */

#define FIELD_CUSTOM    65

typedef struct _TIFFField TIFFField;
typedef struct _TIFFFieldArray TIFFFieldArray;

extern const TIFFField* NDPIFindField(TIFF *, uint32_t, TIFFDataType);
extern const TIFFField* NDPIFieldWithTag(TIFF*, uint32_t);
extern const TIFFField* NDPIFieldWithName(TIFF*, const char *);

extern uint32_t NDPIFieldTag(const TIFFField*);
extern const char* NDPIFieldName(const TIFFField*);
extern TIFFDataType NDPIFieldDataType(const TIFFField*);
extern int NDPIFieldPassCount(const TIFFField*);
extern int TIFFFieldReadCount(const TIFFField*);
extern int NDPIFieldWriteCount(const TIFFField*);

typedef int (*TIFFVSetMethod)(TIFF*, uint32_t, va_list);
typedef int (*TIFFVGetMethod)(TIFF*, uint32_t, va_list);
typedef void (*TIFFPrintMethod)(TIFF*, FILE*, long);

typedef struct {
    TIFFVSetMethod vsetfield; /* tag set routine */
    TIFFVGetMethod vgetfield; /* tag get routine */
    TIFFPrintMethod printdir; /* directory print routine */
} TIFFTagMethods;

extern  TIFFTagMethods *NDPIAccessTagMethods(TIFF *);
extern  void *NDPIGetClientInfo(TIFF *, const char *);
extern  void NDPISetClientInfo(TIFF *, void *, const char *);

extern void NDPICleanup(TIFF* tif);
extern void NDPIClose(TIFF* tif);
extern int NDPIFlush(TIFF* tif);
extern int NDPIFlushData(TIFF* tif);
extern int NDPIGetField(TIFF* tif, uint32_t tag, ...);
extern int NDPIVGetField(TIFF* tif, uint32_t tag, va_list ap);
extern int NDPIGetFieldDefaulted(TIFF* tif, uint32_t tag, ...);
extern int NDPIVGetFieldDefaulted(TIFF* tif, uint32_t tag, va_list ap);
extern int NDPIReadDirectory(TIFF* tif);
extern int TIFFReadCustomDirectory(TIFF* tif, toff_t diroff, const TIFFFieldArray* infoarray);
extern int TIFFReadEXIFDirectory(TIFF* tif, toff_t diroff);
extern int TIFFReadGPSDirectory(TIFF* tif, toff_t diroff);
extern uint64_t NDPIScanlineSize64(TIFF* tif);
extern tmsize_t NDPIScanlineSize(TIFF* tif);
extern uint64_t NDPIRasterScanlineSize64(TIFF* tif);
extern tmsize_t NDPIRasterScanlineSize(TIFF* tif);
extern uint64_t NDPIStripSize64(TIFF* tif);
extern tmsize_t NDPIStripSize(TIFF* tif);
extern uint64_t NDPIRawStripSize64(TIFF* tif, uint32_t strip);
extern tmsize_t NDPIRawStripSize(TIFF* tif, uint32_t strip);
extern uint64_t NDPIVStripSize64(TIFF* tif, uint32_t nrows);
extern tmsize_t NDPIVStripSize(TIFF* tif, uint32_t nrows);
extern uint64_t NDPITileRowSize64(TIFF* tif);
extern tmsize_t NDPITileRowSize(TIFF* tif);
extern uint64_t NDPITileSize64(TIFF* tif);
extern tmsize_t NDPITileSize(TIFF* tif);
extern uint64_t NDPIVTileSize64(TIFF* tif, uint32_t nrows);
extern tmsize_t NDPIVTileSize(TIFF* tif, uint32_t nrows);
extern uint32_t NDPIDefaultStripSize(TIFF* tif, uint32_t request);
extern void NDPIDefaultTileSize(TIFF*, uint32_t*, uint32_t*);
extern int NDPIFileno(TIFF*);
extern int NDPISetFileno(TIFF*, int);
extern thandle_t NDPIClientdata(TIFF*);
extern thandle_t NDPISetClientdata(TIFF*, thandle_t);
extern int NDPIGetMode(TIFF*);
extern int NDPISetMode(TIFF*, int);
extern int NDPIIsTiled(TIFF*);
extern int NDPIIsByteSwapped(TIFF*);
extern int NDPIIsUpSampled(TIFF*);
extern int NDPIIsMSB2LSB(TIFF*);
extern int NDPIIsBigEndian(TIFF*);
extern TIFFReadWriteProc NDPIGetReadProc(TIFF*);
extern TIFFReadWriteProc NDPIGetWriteProc(TIFF*);
extern TIFFSeekProc NDPIGetSeekProc(TIFF*);                                                          
extern TIFFCloseProc NDPIGetCloseProc(TIFF*);
extern TIFFSizeProc NDPIGetSizeProc(TIFF*);
extern TIFFMapFileProc NDPIGetMapFileProc(TIFF*);
extern TIFFUnmapFileProc NDPIGetUnmapFileProc(TIFF*);
extern uint32_t NDPICurrentRow(TIFF*);
extern uint16_t NDPICurrentDirectory(TIFF*);
extern uint16_t NDPINumberOfDirectories(TIFF*);
extern uint64_t NDPICurrentDirOffset(TIFF*);
extern uint32_t NDPICurrentStrip(TIFF*);
extern uint32_t NDPICurrentTile(TIFF* tif);
extern int NDPIReadBufferSetup(TIFF* tif, void* bp, tmsize_t size);
extern int TIFFWriteBufferSetup(TIFF* tif, void* bp, tmsize_t size);  
extern int NDPISetupStrips(TIFF *);
extern int NDPIWriteCheck(TIFF*, int, const char *);
extern void NDPIFreeDirectory(TIFF*);
extern int NDPICreateDirectory(TIFF*);
extern int NDPICreateCustomDirectory(TIFF*,const TIFFFieldArray*);
extern int NDPICreateEXIFDirectory(TIFF*);
extern int NDPICreateGPSDirectory(TIFF*);
extern int NDPILastDirectory(TIFF*);
extern int NDPISetDirectory(TIFF*, uint16_t);
extern int NDPISetSubDirectory(TIFF*, uint64_t);
extern int NDPIUnlinkDirectory(TIFF*, uint16_t);
extern int NDPISetField(TIFF*, uint32_t, ...);
extern int NDPIVSetField(TIFF*, uint32_t, va_list);
extern int NDPIUnsetField(TIFF*, uint32_t);
extern int NDPIWriteDirectory(TIFF *);
extern int NDPIWriteCustomDirectory(TIFF *, uint64_t *);
extern int NDPICheckpointDirectory(TIFF *);
extern int NDPIRewriteDirectory(TIFF *);
extern int NDPIDeferStrileArrayWriting(TIFF *);
extern int NDPIForceStrileArrayWriting(TIFF* );

#if defined(c_plusplus) || defined(__cplusplus)
extern void NDPIPrintDirectory(TIFF*, FILE*, long = 0);
extern int NDPIReadScanline(TIFF* tif, void* buf, uint32_t row, uint16_t sample = 0);
extern int NDPIWriteScanline(TIFF* tif, void* buf, uint32_t row, uint16_t sample = 0);
extern int NDPIReadRGBAImage(TIFF*, uint32_t, uint32_t, uint32_t*, int = 0);
extern int NDPIReadRGBAImageOriented(TIFF*, uint32_t, uint32_t, uint32_t*,
    int = ORIENTATION_BOTLEFT, int = 0);
#else
extern void NDPIPrintDirectory(TIFF*, FILE*, long);
extern int NDPIReadScanline(TIFF* tif, void* buf, uint32_t row, uint16_t sample);
extern int NDPIWriteScanline(TIFF* tif, void* buf, uint32_t row, uint16_t sample);
extern int NDPIReadRGBAImage(TIFF*, uint32_t, uint32_t, uint32_t*, int);
extern int NDPIReadRGBAImageOriented(TIFF*, uint32_t, uint32_t, uint32_t*, int, int);
#endif

extern int NDPIReadRGBAStrip(TIFF*, uint32_t, uint32_t * );
extern int NDPIReadRGBATile(TIFF*, uint32_t, uint32_t, uint32_t * );
extern int NDPIReadRGBAStripExt(TIFF*, uint32_t, uint32_t *, int stop_on_error );
extern int NDPIReadRGBATileExt(TIFF*, uint32_t, uint32_t, uint32_t *, int stop_on_error );
extern int NDPIRGBAImageOK(TIFF*, char [1024]);
extern int NDPIRGBAImageBegin(TIFFRGBAImage*, TIFF*, int, char [1024]);
extern int NDPIRGBAImageGet(TIFFRGBAImage*, uint32_t*, uint32_t, uint32_t);
extern void NDPIRGBAImageEnd(TIFFRGBAImage*);
extern TIFF* NDPIOpen(const char*, const char*);
# ifdef __WIN32__
extern TIFF* TIFFOpenW(const wchar_t*, const char*);
# endif /* __WIN32__ */
extern TIFF* NDPIdOpen(int, const char*, const char*);
extern TIFF* NDPIClientOpen(const char*, const char*,
	    thandle_t,
	    TIFFReadWriteProc, TIFFReadWriteProc,
	    TIFFSeekProc, TIFFCloseProc,
	    TIFFSizeProc,
	    TIFFMapFileProc, TIFFUnmapFileProc);
extern const char* NDPIFileName(TIFF*);
extern const char* NDPISetFileName(TIFF*, const char *);
extern void NDPIError(const char*, const char*, ...) TIFF_ATTRIBUTE((__format__ (__printf__,2,3)));
extern void NDPIErrorExt(thandle_t, const char*, const char*, ...) TIFF_ATTRIBUTE((__format__ (__printf__,3,4)));
extern void NDPIWarning(const char*, const char*, ...) TIFF_ATTRIBUTE((__format__ (__printf__,2,3)));
extern void NDPIWarningExt(thandle_t, const char*, const char*, ...) TIFF_ATTRIBUTE((__format__ (__printf__,3,4)));
extern TIFFErrorHandler NDPISetErrorHandler(TIFFErrorHandler);
extern TIFFErrorHandlerExt NDPISetErrorHandlerExt(TIFFErrorHandlerExt);
extern TIFFErrorHandler NDPISetWarningHandler(TIFFErrorHandler);
extern TIFFErrorHandlerExt TIFFSetWarningHandlerExt(TIFFErrorHandlerExt);
extern TIFFExtendProc NDPISetTagExtender(TIFFExtendProc);
extern uint32_t NDPIComputeTile(TIFF* tif, uint32_t x, uint32_t y, uint32_t z, uint16_t s);
extern int NDPICheckTile(TIFF* tif, uint32_t x, uint32_t y, uint32_t z, uint16_t s);
extern uint32_t NDPINumberOfTiles(TIFF*);
extern tmsize_t NDPIReadTile(TIFF* tif, void* buf, uint32_t x, uint32_t y, uint32_t z, uint16_t s);
extern tmsize_t NDPIWriteTile(TIFF* tif, void* buf, uint32_t x, uint32_t y, uint32_t z, uint16_t s);
extern uint32_t NDPIComputeStrip(TIFF*, uint32_t, uint16_t);
extern uint32_t NDPINumberOfStrips(TIFF*);
extern tmsize_t NDPIReadEncodedStrip(TIFF* tif, uint32_t strip, void* buf, tmsize_t size);
extern tmsize_t NDPIReadRawStrip(TIFF* tif, uint32_t strip, void* buf, tmsize_t size);
extern tmsize_t NDPIReadEncodedTile(TIFF* tif, uint32_t tile, void* buf, tmsize_t size);
extern tmsize_t NDPIReadRawTile(TIFF* tif, uint32_t tile, void* buf, tmsize_t size);
extern int      NDPIReadFromUserBuffer(TIFF* tif, uint32_t strile,
                                       void* inbuf, tmsize_t insize,
                                       void* outbuf, tmsize_t outsize);
extern tmsize_t NDPIWriteEncodedStrip(TIFF* tif, uint32_t strip, void* data, tmsize_t cc);
extern tmsize_t NDPIWriteRawStrip(TIFF* tif, uint32_t strip, void* data, tmsize_t cc);
extern tmsize_t NDPIWriteEncodedTile(TIFF* tif, uint32_t tile, void* data, tmsize_t cc);
extern tmsize_t NDPIWriteRawTile(TIFF* tif, uint32_t tile, void* data, tmsize_t cc);
extern int TIFFDataWidth(TIFFDataType);    /* table of tag datatype widths */
extern void NDPISetWriteOffset(TIFF* tif, toff_t off);
extern void NDPISwabShort(uint16_t*);
extern void NDPISwabLong(uint32_t*);
extern void NDPISwabLong8(uint64_t*);
extern void TIFFSwabFloat(float*);
extern void TIFFSwabDouble(double*);
extern void NDPISwabArrayOfShort(uint16_t* wp, tmsize_t n);
extern void NDPISwabArrayOfTriples(uint8_t* tp, tmsize_t n);
extern void NDPISwabArrayOfLong(uint32_t* lp, tmsize_t n);
extern void TIFFSwabArrayOfLong8(uint64_t* lp, tmsize_t n);
extern void TIFFSwabArrayOfFloat(float* fp, tmsize_t n);
extern void NDPISwabArrayOfDouble(double* dp, tmsize_t n);
extern void NDPIReverseBits(uint8_t* cp, tmsize_t n);
extern const unsigned char* TIFFGetBitRevTable(int);

extern uint64_t NDPIGetStrileOffset(TIFF *tif, uint32_t strile);
extern uint64_t NDPIGetStrileByteCount(TIFF *tif, uint32_t strile);
extern uint64_t TIFFGetStrileOffsetWithErr(TIFF *tif, uint32_t strile, int *pbErr);
extern uint64_t TIFFGetStrileByteCountWithErr(TIFF *tif, uint32_t strile, int *pbErr);

#ifdef LOGLUV_PUBLIC
#define U_NEU		0.210526316
#define V_NEU		0.473684211
#define UVSCALE		410.
extern double LogL16toY(int);
extern double LogL10toY(int);
extern void XYZtoRGB24(float*, uint8_t*);
extern int uv_decode(double*, double*, int);
extern void LogLuv24toXYZ(uint32_t, float*);
extern void LogLuv32toXYZ(uint32_t, float*);
#if defined(c_plusplus) || defined(__cplusplus)
extern int LogL16fromY(double, int = SGILOGENCODE_NODITHER);
extern int LogL10fromY(double, int = SGILOGENCODE_NODITHER);
extern int uv_encode(double, double, int = SGILOGENCODE_NODITHER);
extern uint32_t LogLuv24fromXYZ(float*, int = SGILOGENCODE_NODITHER);
extern uint32_t LogLuv32fromXYZ(float*, int = SGILOGENCODE_NODITHER);
#else
extern int LogL16fromY(double, int);
extern int LogL10fromY(double, int);
extern int uv_encode(double, double, int);
extern uint32_t LogLuv24fromXYZ(float*, int);
extern uint32_t LogLuv32fromXYZ(float*, int);
#endif
#endif /* LOGLUV_PUBLIC */

extern int NDPICIELabToRGBInit(TIFFCIELabToRGB*, const TIFFDisplay *, float*);
extern void NDPICIELabToXYZ(TIFFCIELabToRGB *, uint32_t, int32_t, int32_t,
                            float *, float *, float *);
extern void NDPIXYZToRGB(TIFFCIELabToRGB *, float, float, float,
                         uint32_t *, uint32_t *, uint32_t *);

extern int NDPIYCbCrToRGBInit(TIFFYCbCrToRGB*, float*, float*);
extern void NDPIYCbCrtoRGB(TIFFYCbCrToRGB *, uint32_t, int32_t, int32_t,
                           uint32_t *, uint32_t *, uint32_t *);

/****************************************************************************
 *               O B S O L E T E D    I N T E R F A C E S
 *
 * Don't use this stuff in your applications, it may be removed in the future
 * libtiff versions.
 ****************************************************************************/
typedef	struct {
	ttag_t	field_tag;		/* field's tag */
	short	field_readcount;	/* read count/TIFF_VARIABLE/TIFF_SPP */
	short	field_writecount;	/* write count/TIFF_VARIABLE */
	TIFFDataType field_type;	/* type of associated data */
        unsigned short field_bit;	/* bit in fieldsset bit vector */
	unsigned char field_oktochange;	/* if true, can change while writing */
	unsigned char field_passcount;	/* if true, pass dir count on set */
	char	*field_name;		/* ASCII name */
} TIFFFieldInfo;

extern int TIFFMergeFieldInfo(TIFF*, const TIFFFieldInfo[], uint32_t);
        
#if defined(c_plusplus) || defined(__cplusplus)
}
#endif

#endif /* _TIFFIO_ */

/* vim: set ts=8 sts=8 sw=8 noet: */
/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 8
 * fill-column: 78
 * End:
 */
