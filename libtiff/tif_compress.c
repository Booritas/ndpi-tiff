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
 * Compression Scheme Configuration Support.
 */
#include "tiffiop.h"

static int
NDPINoEncode(TIFF* tif, const char* method)
{
	const TIFFCodec* c = NDPIFindCODEC(tif->tif_dir.td_compression);

	if (c) {
		NDPIErrorExt(tif->tif_clientdata, tif->tif_name,
			     "%s %s encoding is not implemented",
			     c->name, method);
	} else {
		NDPIErrorExt(tif->tif_clientdata, tif->tif_name,
			"Compression scheme %"PRIu16" %s encoding is not implemented",
			     tif->tif_dir.td_compression, method);
	}
	return (-1);
}

int
_NDPINoRowEncode(TIFF* tif, uint8_t* pp, tmsize_t cc, uint16_t s)
{
	(void) pp; (void) cc; (void) s;
	return (NDPINoEncode(tif, "scanline"));
}

int
_NDPINoStripEncode(TIFF* tif, uint8_t* pp, tmsize_t cc, uint16_t s)
{
	(void) pp; (void) cc; (void) s;
	return (NDPINoEncode(tif, "strip"));
}

int
_NDPINoTileEncode(TIFF* tif, uint8_t* pp, tmsize_t cc, uint16_t s)
{
	(void) pp; (void) cc; (void) s;
	return (NDPINoEncode(tif, "tile"));
}

static int
NDPINoDecode(TIFF* tif, const char* method)
{
	const TIFFCodec* c = NDPIFindCODEC(tif->tif_dir.td_compression);

	if (c)
		NDPIErrorExt(tif->tif_clientdata, tif->tif_name,
			     "%s %s decoding is not implemented",
			     c->name, method);
	else
		NDPIErrorExt(tif->tif_clientdata, tif->tif_name,
			     "Compression scheme %"PRIu16" %s decoding is not implemented",
			     tif->tif_dir.td_compression, method);
	return (0);
}

static int
_NDPINoFixupTags(TIFF* tif)
{
	(void) tif;
	return (1);
}

int
_NDPINoRowDecode(TIFF* tif, uint8_t* pp, tmsize_t cc, uint16_t s)
{
	(void) pp; (void) cc; (void) s;
	return (NDPINoDecode(tif, "scanline"));
}

int
_NDPINoStripDecode(TIFF* tif, uint8_t* pp, tmsize_t cc, uint16_t s)
{
	(void) pp; (void) cc; (void) s;
	return (NDPINoDecode(tif, "strip"));
}

int
_NDPINoTileDecode(TIFF* tif, uint8_t* pp, tmsize_t cc, uint16_t s)
{
	(void) pp; (void) cc; (void) s;
	return (NDPINoDecode(tif, "tile"));
}

int
_NDPINoSeek(TIFF* tif, uint32_t off)
{
	(void) off;
	NDPIErrorExt(tif->tif_clientdata, tif->tif_name,
		     "Compression algorithm does not support random access");
	return (0);
}

int
_NDPINoPreCode(TIFF* tif, uint16_t s)
{
	(void) tif; (void) s;
	return (1);
}

static int _NDPItrue(TIFF* tif) { (void) tif; return (1); }
static void _NDPIvoid(TIFF* tif) { (void) tif; }

void
_NDPISetDefaultCompressionState(TIFF* tif)
{
	tif->tif_fixuptags = _NDPINoFixupTags; 
	tif->tif_decodestatus = TRUE;
	tif->tif_setupdecode = _NDPItrue;
	tif->tif_predecode = _NDPINoPreCode;
	tif->tif_decoderow = _NDPINoRowDecode;  
	tif->tif_decodestrip = _NDPINoStripDecode;
	tif->tif_decodetile = _NDPINoTileDecode;  
	tif->tif_encodestatus = TRUE;
	tif->tif_setupencode = _NDPItrue;
	tif->tif_preencode = _NDPINoPreCode;
	tif->tif_postencode = _NDPItrue;
	tif->tif_encoderow = _NDPINoRowEncode;
	tif->tif_encodestrip = _NDPINoStripEncode;  
	tif->tif_encodetile = _NDPINoTileEncode;  
	tif->tif_close = _NDPIvoid;
	tif->tif_seek = _NDPINoSeek;
	tif->tif_cleanup = _NDPIvoid;
	tif->tif_defstripsize = _NDPIDefaultStripSize;
	tif->tif_deftilesize = _NDPIDefaultTileSize;
	tif->tif_flags &= ~(TIFF_NOBITREV|TIFF_NOREADRAW);
}

int
NDPISetCompressionScheme(TIFF* tif, int scheme)
{
	const TIFFCodec *c = NDPIFindCODEC((uint16_t) scheme);

	_NDPISetDefaultCompressionState(tif);
	/*
	 * Don't treat an unknown compression scheme as an error.
	 * This permits applications to open files with data that
	 * the library does not have builtin support for, but which
	 * may still be meaningful.
	 */
	return (c ? (*c->init)(tif, scheme) : 1);
}

/*
 * Other compression schemes may be registered.  Registered
 * schemes can also override the builtin versions provided
 * by this library.
 */
typedef struct _codec {
	struct _codec* next;
	TIFFCodec* info;
} codec_t;
static codec_t* registeredCODECS = NULL;

const TIFFCodec*
NDPIFindCODEC(uint16_t scheme)
{
	const TIFFCodec* c;
	codec_t* cd;

	for (cd = registeredCODECS; cd; cd = cd->next)
		if (cd->info->scheme == scheme)
			return ((const TIFFCodec*) cd->info);
	for (c = _TIFFBuiltinCODECS; c->name; c++)
		if (c->scheme == scheme)
			return (c);
	return ((const TIFFCodec*) 0);
}

TIFFCodec*
NDPIRegisterCODEC(uint16_t scheme, const char* name, TIFFInitMethod init)
{
	codec_t* cd = (codec_t*)
	    _NDPImalloc((tmsize_t)(sizeof (codec_t) + sizeof (TIFFCodec) + strlen(name)+1));

	if (cd != NULL) {
		cd->info = (TIFFCodec*) ((uint8_t*) cd + sizeof (codec_t));
		cd->info->name = (char*)
		    ((uint8_t*) cd->info + sizeof (TIFFCodec));
		strcpy(cd->info->name, name);
		cd->info->scheme = scheme;
		cd->info->init = init;
		cd->next = registeredCODECS;
		registeredCODECS = cd;
	} else {
		NDPIErrorExt(0, "NDPIRegisterCODEC",
		    "No space to register compression scheme %s", name);
		return NULL;
	}
	return (cd->info);
}

void
NDPIUnRegisterCODEC(TIFFCodec* c)
{
	codec_t* cd;
	codec_t** pcd;

	for (pcd = &registeredCODECS; (cd = *pcd) != NULL; pcd = &cd->next)
		if (cd->info == c) {
			*pcd = cd->next;
			_NDPIfree(cd);
			return;
		}
	NDPIErrorExt(0, "NDPIUnRegisterCODEC",
	    "Cannot remove compression scheme %s; not registered", c->name);
}

/************************************************************************/
/*                       TIFFGetConfisuredCODECs()                      */
/************************************************************************/

/**
 * Get list of configured codecs, both built-in and registered by user.
 * Caller is responsible to free this structure.
 * 
 * @return returns array of TIFFCodec records (the last record should be NULL)
 * or NULL if function failed.
 */

TIFFCodec*
NDPIGetConfiguredCODECs()
{
	int i = 1;
	codec_t *cd;
	const TIFFCodec* c;
	TIFFCodec* codecs = NULL;
	TIFFCodec* new_codecs;

	for (cd = registeredCODECS; cd; cd = cd->next) {
		new_codecs = (TIFFCodec *)
			_NDPIrealloc(codecs, i * sizeof(TIFFCodec));
		if (!new_codecs) {
			_NDPIfree (codecs);
			return NULL;
		}
		codecs = new_codecs;
		_NDPImemcpy(codecs + i - 1, cd->info, sizeof(TIFFCodec));
		i++;
	}
	for (c = _TIFFBuiltinCODECS; c->name; c++) {
		if (NDPIIsCODECConfigured(c->scheme)) {
			new_codecs = (TIFFCodec *)
				_NDPIrealloc(codecs, i * sizeof(TIFFCodec));
			if (!new_codecs) {
				_NDPIfree (codecs);
				return NULL;
			}
			codecs = new_codecs;
			_NDPImemcpy(codecs + i - 1, (const void*)c, sizeof(TIFFCodec));
			i++;
		}
	}

	new_codecs = (TIFFCodec *) _NDPIrealloc(codecs, i * sizeof(TIFFCodec));
	if (!new_codecs) {
		_NDPIfree (codecs);
		return NULL;
	}
	codecs = new_codecs;
	_NDPImemset(codecs + i - 1, 0, sizeof(TIFFCodec));

	return codecs;
}

/* vim: set ts=8 sts=8 sw=8 noet: */
/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 8
 * fill-column: 78
 * End:
 */
