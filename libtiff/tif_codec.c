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

/*
 * TIFF Library
 *
 * Builtin Compression Scheme Configuration Support.
 */
#include "tiffiop.h"

static int NotConfigured(TIFF*, int);

#ifndef LZW_SUPPORT
#define NDPIInitLZW NotConfigured
#endif
#ifndef PACKBITS_SUPPORT
#define NDPIInitPackBits NotConfigured
#endif
#ifndef THUNDER_SUPPORT
#define NDPIInitThunderScan NotConfigured
#endif
#ifndef NEXT_SUPPORT
#define NDPIInitNeXT NotConfigured
#endif
#ifndef JPEG_SUPPORT
#define NDPIInitJPEG NotConfigured
#endif
#ifndef OJPEG_SUPPORT
#define NDPIInitOJPEG NotConfigured
#endif
#ifndef CCITT_SUPPORT
#define NDPIInitCCITTRLE NotConfigured
#define NDPIInitCCITTRLEW NotConfigured
#define NDPIInitCCITTFax3 NotConfigured
#define NDPIInitCCITTFax4 NotConfigured
#endif
#ifndef JBIG_SUPPORT
#define NDPIInitJBIG NotConfigured
#endif
#ifndef ZIP_SUPPORT
#define NDPIInitZIP NotConfigured
#endif
#ifndef PIXARLOG_SUPPORT
#define NDPIInitPixarLog NotConfigured
#endif
#ifndef LOGLUV_SUPPORT
#define NDPIInitSGILog NotConfigured
#endif
#ifndef LERC_SUPPORT
#define NDPIInitLERC NotConfigured
#endif
#ifndef LZMA_SUPPORT
#define NDPIInitLZMA NotConfigured
#endif
#ifndef ZSTD_SUPPORT
#define NDPIInitZSTD NotConfigured
#endif
#ifndef WEBP_SUPPORT
#define NDPIInitWebP NotConfigured
#endif

/*
 * Compression schemes statically built into the library.
 */
const TIFFCodec _NDPIBuiltinCODECS[] = {
    { "None",		COMPRESSION_NONE,	NDPIInitDumpMode },
    { "LZW",		COMPRESSION_LZW,	NDPIInitLZW },
    { "PackBits",	COMPRESSION_PACKBITS,	NDPIInitPackBits },
    { "ThunderScan",	COMPRESSION_THUNDERSCAN,NDPIInitThunderScan },
    { "NeXT",		COMPRESSION_NEXT,	NDPIInitNeXT },
    { "JPEG",		COMPRESSION_JPEG,	NDPIInitJPEG },
    { "Old-style JPEG",	COMPRESSION_OJPEG,	NDPIInitOJPEG },
    { "CCITT RLE",	COMPRESSION_CCITTRLE,	NDPIInitCCITTRLE },
    { "CCITT RLE/W",	COMPRESSION_CCITTRLEW,	NDPIInitCCITTRLEW },
    { "CCITT Group 3",	COMPRESSION_CCITTFAX3,	NDPIInitCCITTFax3 },
    { "CCITT Group 4",	COMPRESSION_CCITTFAX4,	NDPIInitCCITTFax4 },
    { "ISO JBIG",	COMPRESSION_JBIG,	NDPIInitJBIG },
    { "Deflate",	COMPRESSION_DEFLATE,	NDPIInitZIP },
    { "AdobeDeflate",   COMPRESSION_ADOBE_DEFLATE , NDPIInitZIP }, 
    { "PixarLog",	COMPRESSION_PIXARLOG,	NDPIInitPixarLog },
    { "SGILog",		COMPRESSION_SGILOG,	NDPIInitSGILog },
    { "SGILog24",	COMPRESSION_SGILOG24,	NDPIInitSGILog },
    { "LZMA",		COMPRESSION_LZMA,	NDPIInitLZMA },
    { "ZSTD",		COMPRESSION_ZSTD,	NDPIInitZSTD },
    { "WEBP",		COMPRESSION_WEBP,	NDPIInitWebP },
    { "LERC",		COMPRESSION_LERC,	NDPIInitLERC },
    { NULL,             0,                      NULL }
};

static int
_notConfigured(TIFF* tif)
{
	const TIFFCodec* c = NDPIFindCODEC(tif->tif_dir.td_compression);
        char compression_code[20];
        
        sprintf(compression_code, "%"PRIu16, tif->tif_dir.td_compression );
	NDPIErrorExt(tif->tif_clientdata, tif->tif_name,
                     "%s compression support is not configured", 
                     c ? c->name : compression_code );
	return (0);
}

static int
NotConfigured(TIFF* tif, int scheme)
{
	(void) scheme;

	tif->tif_fixuptags = _notConfigured;
	tif->tif_decodestatus = FALSE;
	tif->tif_setupdecode = _notConfigured;
	tif->tif_encodestatus = FALSE;
	tif->tif_setupencode = _notConfigured;
	return (1);
}

/************************************************************************/
/*                       NDPIIsCODECConfigured()                        */
/************************************************************************/

/**
 * Check whether we have working codec for the specific coding scheme.
 *
 * @return returns 1 if the codec is configured and working. Otherwise
 * 0 will be returned.
 */

int
NDPIIsCODECConfigured(uint16_t scheme)
{
	const TIFFCodec* codec = NDPIFindCODEC(scheme);

	if(codec == NULL) {
		return 0;
	}
	if(codec->init == NULL) {
		return 0;
	}
	if(codec->init != NotConfigured){
		return 1;
	}
	return 0;
}

/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 8
 * fill-column: 78
 * End:
 */
