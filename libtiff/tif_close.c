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
 * TIFF Library.
 */
#include "tiffiop.h"
#include <string.h>

/************************************************************************/
/*                            NDPICleanup()                             */
/************************************************************************/

/**
 * Auxiliary function to free the TIFF structure. Given structure will be
 * completely freed, so you should save opened file handle and pointer
 * to the close procedure in external variables before calling
 * _TIFFCleanup(), if you will need these ones to close the file.
 * 
 * @param tif A TIFF pointer.
 */

void
NDPICleanup(TIFF* tif)
{
	/*
         * Flush buffered data and directory (if dirty).
         */
	if (tif->tif_mode != O_RDONLY)
		NDPIFlush(tif);
	(*tif->tif_cleanup)(tif);
	NDPIFreeDirectory(tif);

	if (tif->tif_dirlist)
		_NDPIfree(tif->tif_dirlist);

	/*
         * Clean up client info links.
         */
	while( tif->tif_clientinfo )
	{
		TIFFClientInfoLink *psLink = tif->tif_clientinfo;

		tif->tif_clientinfo = psLink->next;
		_NDPIfree( psLink->name );
		_NDPIfree( psLink );
	}

	if (tif->tif_rawdata && (tif->tif_flags&TIFF_MYBUFFER))
		_NDPIfree(tif->tif_rawdata);
	if (isMapped(tif))
		TIFFUnmapFileContents(tif, tif->tif_base, (toff_t)tif->tif_size);

	/*
         * Clean up custom fields.
         */
	if (tif->tif_fields && tif->tif_nfields > 0) {
		uint32_t i;

		for (i = 0; i < tif->tif_nfields; i++) {
			TIFFField *fld = tif->tif_fields[i];
			if (fld->field_bit == FIELD_CUSTOM &&
			    strncmp("Tag ", fld->field_name, 4) == 0) {
				_NDPIfree(fld->field_name);
				_NDPIfree(fld);
			}
		}

		_NDPIfree(tif->tif_fields);
	}

        if (tif->tif_nfieldscompat > 0) {
                uint32_t i;

                for (i = 0; i < tif->tif_nfieldscompat; i++) {
                        if (tif->tif_fieldscompat[i].allocated_size)
                                _NDPIfree(tif->tif_fieldscompat[i].fields);
                }
                _NDPIfree(tif->tif_fieldscompat);
        }

	_NDPIfree(tif);
}

/************************************************************************/
/*                            NDPIClose()                               */
/************************************************************************/

/**
 * Close a previously opened TIFF file.
 *
 * NDPIClose closes a file that was previously opened with NDPIOpen().
 * Any buffered data are flushed to the file, including the contents of
 * the current directory (if modified); and all resources are reclaimed.
 * 
 * @param tif A TIFF pointer.
 */

void
NDPIClose(TIFF* tif)
{
	TIFFCloseProc closeproc = tif->tif_closeproc;
	thandle_t fd = tif->tif_clientdata;

	NDPICleanup(tif);
	(void) (*closeproc)(fd);
}

/* vim: set ts=8 sts=8 sw=8 noet: */

/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 8
 * fill-column: 78
 * End:
 */
