/******************************************************************************
 * Project:  TIFF Overview Builder
 * Purpose:  Library functions to maintain two rows of tiles or two strips
 *           of data for output overviews as an output cache. 
 * Author:   Frank Warmerdam, warmerdam@pobox.com
 *
 ******************************************************************************
 * Copyright (c) 2000, Frank Warmerdam
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 ******************************************************************************
 */

#include "tiffiop.h"
#include "tif_ovrcache.h"
#include <assert.h>

/************************************************************************/
/*                         NDPICreateOvrCache()                         */
/*                                                                      */
/*      Create an overview cache to hold two rows of blocks from an     */
/*      existing TIFF directory.                                        */
/************************************************************************/

TIFFOvrCache *NDPICreateOvrCache( TIFF *hTIFF, toff_t nDirOffset )

{
    TIFFOvrCache	*psCache;
    toff_t		nBaseDirOffset;

    psCache = (TIFFOvrCache *) _NDPImalloc(sizeof(TIFFOvrCache));
    psCache->nDirOffset = nDirOffset;
    psCache->hTIFF = hTIFF;
    
/* -------------------------------------------------------------------- */
/*      Get definition of this raster from the TIFF file itself.        */
/* -------------------------------------------------------------------- */
    nBaseDirOffset = NDPICurrentDirOffset( psCache->hTIFF );
    NDPISetSubDirectory( hTIFF, nDirOffset );
    
    NDPIGetField( hTIFF, TIFFTAG_IMAGEWIDTH, &(psCache->nXSize) );
    NDPIGetField( hTIFF, TIFFTAG_IMAGELENGTH, &(psCache->nYSize) );

    NDPIGetField( hTIFF, TIFFTAG_BITSPERSAMPLE, &(psCache->nBitsPerPixel) );
    NDPIGetField( hTIFF, TIFFTAG_SAMPLESPERPIXEL, &(psCache->nSamples) );
    NDPIGetField( hTIFF, TIFFTAG_PLANARCONFIG, &(psCache->nPlanarConfig) );

    if( !NDPIIsTiled( hTIFF ) )
    {
        NDPIGetField( hTIFF, TIFFTAG_ROWSPERSTRIP, &(psCache->nBlockYSize) );
        psCache->nBlockXSize = psCache->nXSize;
        psCache->nBytesPerBlock = NDPIStripSize(hTIFF);
        psCache->bTiled = FALSE;
    }
    else
    {
        NDPIGetField( hTIFF, TIFFTAG_TILEWIDTH, &(psCache->nBlockXSize) );
        NDPIGetField( hTIFF, TIFFTAG_TILELENGTH, &(psCache->nBlockYSize) );
        psCache->nBytesPerBlock = NDPITileSize(hTIFF);
        psCache->bTiled = TRUE;
    }

/* -------------------------------------------------------------------- */
/*      Compute some values from this.                                  */
/* -------------------------------------------------------------------- */

    psCache->nBlocksPerRow = (psCache->nXSize + psCache->nBlockXSize - 1)
        		/ psCache->nBlockXSize;
    psCache->nBlocksPerColumn = (psCache->nYSize + psCache->nBlockYSize - 1)
        		/ psCache->nBlockYSize;

    if (psCache->nPlanarConfig == PLANARCONFIG_SEPARATE)
        psCache->nBytesPerRow = psCache->nBytesPerBlock
            * psCache->nBlocksPerRow * psCache->nSamples;
    else
        psCache->nBytesPerRow =
            psCache->nBytesPerBlock * psCache->nBlocksPerRow;


/* -------------------------------------------------------------------- */
/*      Allocate and initialize the data buffers.                       */
/* -------------------------------------------------------------------- */

    psCache->pabyRow1Blocks =
        (unsigned char *) _NDPImalloc(psCache->nBytesPerRow);
    psCache->pabyRow2Blocks =
        (unsigned char *) _NDPImalloc(psCache->nBytesPerRow);

    if ( psCache->pabyRow1Blocks == NULL
         || psCache->pabyRow2Blocks == NULL )
    {
		NDPIErrorExt( hTIFF->tif_clientdata, hTIFF->tif_name,
					  "Can't allocate memory for overview cache." );
                /* TODO: use of NDPIError is inconsistent with use of fprintf in addtiffo.c, sort out */
                if (psCache->pabyRow1Blocks) _NDPIfree(psCache->pabyRow1Blocks);
                if (psCache->pabyRow2Blocks) _NDPIfree(psCache->pabyRow2Blocks);
                _NDPIfree( psCache );
        return NULL;
    }

    _NDPImemset( psCache->pabyRow1Blocks, 0, psCache->nBytesPerRow );
    _NDPImemset( psCache->pabyRow2Blocks, 0, psCache->nBytesPerRow );

    psCache->nBlockOffset = 0;

    NDPISetSubDirectory( psCache->hTIFF, nBaseDirOffset );
    
    return psCache;
}

/************************************************************************/
/*                          NDPIWriteOvrRow()                           */
/*                                                                      */
/*      Write one entire row of blocks (row 1) to the tiff file, and    */
/*      then rotate the block buffers, essentially moving things        */
/*      down by one block.                                              */
/************************************************************************/

static void NDPIWriteOvrRow( TIFFOvrCache * psCache )

{
    int		nRet, iTileX, iTileY = psCache->nBlockOffset;
    unsigned char *pabyData;
    toff_t	nBaseDirOffset;
    uint32_t  RowsInStrip;

/* -------------------------------------------------------------------- */
/*      If the output cache is multi-byte per sample, and the file      */
/*      being written to is of a different byte order than the current  */
/*      platform, we will need to byte swap the data.                   */
/* -------------------------------------------------------------------- */
    if( NDPIIsByteSwapped(psCache->hTIFF) )
    {
        if( psCache->nBitsPerPixel == 16 )
            NDPISwabArrayOfShort( (uint16_t *) psCache->pabyRow1Blocks,
                      (psCache->nBytesPerBlock * psCache->nSamples) / 2 );

        else if( psCache->nBitsPerPixel == 32 )
            NDPISwabArrayOfLong( (uint32_t *) psCache->pabyRow1Blocks,
                         (psCache->nBytesPerBlock * psCache->nSamples) / 4 );

        else if( psCache->nBitsPerPixel == 64 )
            NDPISwabArrayOfDouble( (double *) psCache->pabyRow1Blocks,
                         (psCache->nBytesPerBlock * psCache->nSamples) / 8 );
    }

/* -------------------------------------------------------------------- */
/*      Record original directory position, so we can restore it at     */
/*      end.                                                            */
/* -------------------------------------------------------------------- */
    nBaseDirOffset = NDPICurrentDirOffset( psCache->hTIFF );
    nRet = NDPISetSubDirectory( psCache->hTIFF, psCache->nDirOffset );
    (void) nRet;
    assert( nRet == 1 );

/* -------------------------------------------------------------------- */
/*      Write blocks to TIFF file.                                      */
/* -------------------------------------------------------------------- */
	for( iTileX = 0; iTileX < psCache->nBlocksPerRow; iTileX++ )
	{
		int nTileID;

		if (psCache->nPlanarConfig == PLANARCONFIG_SEPARATE)
		{
			int iSample;

			for( iSample = 0; iSample < psCache->nSamples; iSample++ )
			{
				pabyData = NDPIGetOvrBlock( psCache, iTileX, iTileY, iSample );

				if( psCache->bTiled )
				{
					nTileID = NDPIComputeTile( psCache->hTIFF,
					    iTileX * psCache->nBlockXSize,
					    iTileY * psCache->nBlockYSize,
					    0, (tsample_t) iSample );
					NDPIWriteEncodedTile( psCache->hTIFF, nTileID,
					    pabyData,
					    NDPITileSize(psCache->hTIFF) );
				}
				else
				{
					nTileID = NDPIComputeStrip( psCache->hTIFF,
					    iTileY * psCache->nBlockYSize,
					    (tsample_t) iSample );
					RowsInStrip=psCache->nBlockYSize;
					if ((iTileY+1)*psCache->nBlockYSize>psCache->nYSize)
						RowsInStrip=psCache->nYSize-iTileY*psCache->nBlockYSize;
					NDPIWriteEncodedStrip( psCache->hTIFF, nTileID,
					    pabyData,
					    NDPIVStripSize(psCache->hTIFF,RowsInStrip) );
				}
			}

		}
		else
		{
			pabyData = NDPIGetOvrBlock( psCache, iTileX, iTileY, 0 );

			if( psCache->bTiled )
			{
				nTileID = NDPIComputeTile( psCache->hTIFF,
				    iTileX * psCache->nBlockXSize,
				    iTileY * psCache->nBlockYSize,
				    0, 0 );
				NDPIWriteEncodedTile( psCache->hTIFF, nTileID,
				    pabyData,
				    NDPITileSize(psCache->hTIFF) );
			}
			else
			{
				nTileID = NDPIComputeStrip( psCache->hTIFF,
				    iTileY * psCache->nBlockYSize,
				    0 );
				RowsInStrip=psCache->nBlockYSize;
				if ((iTileY+1)*psCache->nBlockYSize>psCache->nYSize)
					RowsInStrip=psCache->nYSize-iTileY*psCache->nBlockYSize;
				NDPIWriteEncodedStrip( psCache->hTIFF, nTileID,
				    pabyData,
				    NDPIVStripSize(psCache->hTIFF,RowsInStrip) );
			}
		}
	}
	/* TODO: add checks on error status return of NDPIWriteEncodedTile and NDPIWriteEncodedStrip */

/* -------------------------------------------------------------------- */
/*      Rotate buffers.                                                 */
/* -------------------------------------------------------------------- */
    pabyData = psCache->pabyRow1Blocks;
    psCache->pabyRow1Blocks = psCache->pabyRow2Blocks;
    psCache->pabyRow2Blocks = pabyData;

    _NDPImemset( pabyData, 0, psCache->nBytesPerRow );

    psCache->nBlockOffset++;

/* -------------------------------------------------------------------- */
/*      Restore access to original directory.                           */
/* -------------------------------------------------------------------- */
    NDPIFlush( psCache->hTIFF );
    /* TODO: add checks on error status return of NDPIFlush */
    NDPISetSubDirectory( psCache->hTIFF, nBaseDirOffset );
    /* TODO: add checks on error status return of NDPISetSubDirectory */
}

/************************************************************************/
/*                          NDPIGetOvrBlock()                           */
/************************************************************************/

/* TODO: make TIFF_Downsample handle iSample offset, so that we can
 * do with a single NDPIGetOvrBlock and no longer need NDPIGetOvrBlock_Subsampled */
unsigned char *NDPIGetOvrBlock( TIFFOvrCache *psCache, int iTileX, int iTileY,
                                int iSample )

{
    long	       nRowOffset;

    if ( iTileY > psCache->nBlockOffset + 1 )
        NDPIWriteOvrRow( psCache );

    assert( iTileX >= 0 && iTileX < psCache->nBlocksPerRow );
    assert( iTileY >= 0 && iTileY < psCache->nBlocksPerColumn );
    assert( iTileY >= psCache->nBlockOffset
            && iTileY < psCache->nBlockOffset+2 );
    assert( iSample >= 0 && iSample < psCache->nSamples );

    if (psCache->nPlanarConfig == PLANARCONFIG_SEPARATE)
        nRowOffset = ((((toff_t) iTileX * psCache->nSamples) + iSample)
                      * psCache->nBytesPerBlock);
    else
        nRowOffset = iTileX * psCache->nBytesPerBlock +
            (psCache->nBitsPerPixel + 7) / 8 * iSample;

    if ( iTileY == psCache->nBlockOffset )
        return psCache->pabyRow1Blocks + nRowOffset;
    else
        return psCache->pabyRow2Blocks + nRowOffset;
}

/************************************************************************/
/*                     NDPIGetOvrBlock_Subsampled()                     */
/************************************************************************/

unsigned char *NDPIGetOvrBlock_Subsampled( TIFFOvrCache *psCache, 
                                           int iTileX, int iTileY )

{
    int		nRowOffset;

    if( iTileY > psCache->nBlockOffset + 1 )
        NDPIWriteOvrRow( psCache );

    assert( iTileX >= 0 && iTileX < psCache->nBlocksPerRow );
    assert( iTileY >= 0 && iTileY < psCache->nBlocksPerColumn );
    assert( iTileY >= psCache->nBlockOffset
            && iTileY < psCache->nBlockOffset+2 );
    assert( psCache->nPlanarConfig != PLANARCONFIG_SEPARATE );

    nRowOffset = iTileX * psCache->nBytesPerBlock;

    if( iTileY == psCache->nBlockOffset )
        return psCache->pabyRow1Blocks + nRowOffset;
    else
        return psCache->pabyRow2Blocks + nRowOffset;
}

/************************************************************************/
/*                        NDPIDestroyOvrCache()                         */
/************************************************************************/

void NDPIDestroyOvrCache( TIFFOvrCache * psCache )

{
    while( psCache->nBlockOffset < psCache->nBlocksPerColumn )
        NDPIWriteOvrRow( psCache );

    _NDPIfree( psCache->pabyRow1Blocks );
    _NDPIfree( psCache->pabyRow2Blocks );
    _NDPIfree( psCache );
}
/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 4
 * fill-column: 78
 * End:
 */
