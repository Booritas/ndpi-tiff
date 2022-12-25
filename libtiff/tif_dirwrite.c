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
 *
 * Directory Write Support Routines.
 */
#include "tiffiop.h"
#include <float.h>		/*--: for Rational2Double */
#include <math.h>		/*--: for Rational2Double */

#ifdef HAVE_IEEEFP
#define TIFFCvtNativeToIEEEFloat(tif, n, fp)
#define TIFFCvtNativeToIEEEDouble(tif, n, dp)
#else
extern void TIFFCvtNativeToIEEEFloat(TIFF* tif, uint32_t n, float* fp);
extern void TIFFCvtNativeToIEEEDouble(TIFF* tif, uint32_t n, double* dp);
#endif

static int NDPIWriteDirectorySec(TIFF* tif, int isimage, int imagedone, uint64_t* pdiroff);

static int NDPIWriteDirectoryTagSampleformatArray(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, double* value);
#if 0
static int NDPIWriteDirectoryTagSampleformatPerSample(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, double value);
#endif

static int NDPIWriteDirectoryTagAscii(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, char* value);
static int NDPIWriteDirectoryTagUndefinedArray(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, uint8_t* value);
#ifdef notdef
static int NDPIWriteDirectoryTagByte(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint8_t value);
#endif
static int NDPIWriteDirectoryTagByteArray(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, uint8_t* value);
#if 0
static int NDPIWriteDirectoryTagBytePerSample(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint8_t value);
#endif
#ifdef notdef
static int NDPIWriteDirectoryTagSbyte(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, int8_t value);
#endif
static int NDPIWriteDirectoryTagSbyteArray(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, int8_t* value);
#if 0
static int NDPIWriteDirectoryTagSbytePerSample(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, int8_t value);
#endif
static int NDPIWriteDirectoryTagShort(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint16_t value);
static int NDPIWriteDirectoryTagShortArray(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, uint16_t* value);
static int NDPIWriteDirectoryTagShortPerSample(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint16_t value);
#ifdef notdef
static int NDPIWriteDirectoryTagSshort(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, int16_t value);
#endif
static int NDPIWriteDirectoryTagSshortArray(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, int16_t* value);
#if 0
static int NDPIWriteDirectoryTagSshortPerSample(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, int16_t value);
#endif
static int NDPIWriteDirectoryTagLong(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t value);
static int NDPIWriteDirectoryTagLongArray(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, uint32_t* value);
#if 0
static int NDPIWriteDirectoryTagLongPerSample(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t value);
#endif
#ifdef notdef
static int NDPIWriteDirectoryTagSlong(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, int32_t value);
#endif
static int NDPIWriteDirectoryTagSlongArray(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, int32_t* value);
#if 0
static int NDPIWriteDirectoryTagSlongPerSample(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, int32_t value);
#endif
#ifdef notdef
static int NDPIWriteDirectoryTagLong8(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint64_t value);
#endif
static int NDPIWriteDirectoryTagLong8Array(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, uint64_t* value);
#ifdef notdef
static int NDPIWriteDirectoryTagSlong8(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, int64_t value);
#endif
static int NDPIWriteDirectoryTagSlong8Array(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, int64_t* value);
static int NDPIWriteDirectoryTagRational(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, double value);
static int NDPIWriteDirectoryTagRationalArray(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, float* value);
static int NDPIWriteDirectoryTagSrationalArray(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, float* value);
#ifdef notdef
static int NDPIWriteDirectoryTagFloat(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, float value);
#endif
static int NDPIWriteDirectoryTagFloatArray(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, float* value);
#if 0
static int NDPIWriteDirectoryTagFloatPerSample(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, float value);
#endif
#ifdef notdef
static int NDPIWriteDirectoryTagDouble(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, double value);
#endif
static int NDPIWriteDirectoryTagDoubleArray(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, double* value);
#if 0
static int NDPIWriteDirectoryTagDoublePerSample(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, double value);
#endif
static int NDPIWriteDirectoryTagIfdArray(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, uint32_t* value);
#ifdef notdef
static int NDPIWriteDirectoryTagIfd8Array(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, uint64_t* value);
#endif
static int NDPIWriteDirectoryTagShortLong(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t value);
static int NDPIWriteDirectoryTagLongLong8Array(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, uint64_t* value);
static int NDPIWriteDirectoryTagIfdIfd8Array(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, uint64_t* value);
#ifdef notdef
static int NDPIWriteDirectoryTagShortLongLong8Array(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, uint64_t* value);
#endif
static int NDPIWriteDirectoryTagColormap(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir);
static int NDPIWriteDirectoryTagTransferfunction(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir);
static int NDPIWriteDirectoryTagSubifd(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir);

static int NDPIWriteDirectoryTagCheckedAscii(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, char* value);
static int NDPIWriteDirectoryTagCheckedUndefinedArray(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, uint8_t* value);
#ifdef notdef
static int NDPIWriteDirectoryTagCheckedByte(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint8_t value);
#endif
static int NDPIWriteDirectoryTagCheckedByteArray(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, uint8_t* value);
#ifdef notdef
static int NDPIWriteDirectoryTagCheckedSbyte(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, int8_t value);
#endif
static int NDPIWriteDirectoryTagCheckedSbyteArray(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, int8_t* value);
static int NDPIWriteDirectoryTagCheckedShort(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint16_t value);
static int NDPIWriteDirectoryTagCheckedShortArray(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, uint16_t* value);
#ifdef notdef
static int NDPIWriteDirectoryTagCheckedSshort(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, int16_t value);
#endif
static int NDPIWriteDirectoryTagCheckedSshortArray(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, int16_t* value);
static int NDPIWriteDirectoryTagCheckedLong(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t value);
static int NDPIWriteDirectoryTagCheckedLongArray(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, uint32_t* value);
#ifdef notdef
static int NDPIWriteDirectoryTagCheckedSlong(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, int32_t value);
#endif
static int NDPIWriteDirectoryTagCheckedSlongArray(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, int32_t* value);
#ifdef notdef
static int NDPIWriteDirectoryTagCheckedLong8(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint64_t value);
#endif
static int NDPIWriteDirectoryTagCheckedLong8Array(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, uint64_t* value);
#ifdef notdef
static int NDPIWriteDirectoryTagCheckedSlong8(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, int64_t value);
#endif
static int NDPIWriteDirectoryTagCheckedSlong8Array(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, int64_t* value);
static int NDPIWriteDirectoryTagCheckedRational(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, double value);
static int NDPIWriteDirectoryTagCheckedRationalArray(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, float* value);
static int NDPIWriteDirectoryTagCheckedSrationalArray(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, float* value);

/*--: Rational2Double: New functions to support true double-precision for custom rational tag types. */
static int NDPIWriteDirectoryTagRationalDoubleArray(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, double* value);
static int NDPIWriteDirectoryTagSrationalDoubleArray(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, double* value);
static int NDPIWriteDirectoryTagCheckedRationalDoubleArray(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, double* value);
static int NDPIWriteDirectoryTagCheckedSrationalDoubleArray(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, double* value);
static void DoubleToRational(double value, uint32_t *num, uint32_t *denom);
static void DoubleToSrational(double value, int32_t *num, int32_t *denom);
#if 0
static void DoubleToRational_direct(double value, unsigned long *num, unsigned long *denom);
static void DoubleToSrational_direct(double value, long *num, long *denom);
#endif

#ifdef notdef
static int NDPIWriteDirectoryTagCheckedFloat(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, float value);
#endif
static int NDPIWriteDirectoryTagCheckedFloatArray(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, float* value);
#ifdef notdef
static int NDPIWriteDirectoryTagCheckedDouble(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, double value);
#endif
static int NDPIWriteDirectoryTagCheckedDoubleArray(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, double* value);
static int NDPIWriteDirectoryTagCheckedIfdArray(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, uint32_t* value);
static int NDPIWriteDirectoryTagCheckedIfd8Array(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, uint64_t* value);

static int NDPIWriteDirectoryTagData(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint16_t datatype, uint32_t count, uint32_t datalength, void* data);

static int TIFFLinkDirectory(TIFF*);

/*
 * Write the contents of the current directory
 * to the specified file.  This routine doesn't
 * handle overwriting a directory with auxiliary
 * storage that's been changed.
 */
int
NDPIWriteDirectory(TIFF* tif)
{
	return NDPIWriteDirectorySec(tif,TRUE,TRUE,NULL);
}

/*
 * This is an advanced writing function that must be used in a particular
 * sequence, and generally together with NDPIForceStrileArrayWriting(),
 * to make its intended effect. Its aim is to modify the location
 * where the [Strip/Tile][Offsets/ByteCounts] arrays are located in the file.
 * More precisely, when NDPIWriteCheck() will be called, the tag entries for
 * those arrays will be written with type = count = offset = 0 as a temporary
 * value.
 *
 * Its effect is only valid for the current directory, and before
 * NDPIWriteDirectory() is first called, and  will be reset when
 * changing directory.
 *
 * The typical sequence of calls is:
 * NDPIOpen()
 * [ NDPICreateDirectory(tif) ]
 * Set fields with calls to NDPISetField(tif, ...)
 * NDPIDeferStrileArrayWriting(tif)
 * NDPIWriteCheck(tif, ...)
 * NDPIWriteDirectory(tif)
 * ... potentially create other directories and come back to the above directory
 * NDPIForceStrileArrayWriting(tif): emit the arrays at the end of file
 *
 * Returns 1 in case of success, 0 otherwise.
 */
int NDPIDeferStrileArrayWriting(TIFF* tif)
{
    static const char module[] = "NDPIDeferStrileArrayWriting";
    if (tif->tif_mode == O_RDONLY)
    {
        NDPIErrorExt(tif->tif_clientdata, tif->tif_name,
                     "File opened in read-only mode");
        return 0;
    }
    if( tif->tif_diroff != 0 )
    {
        NDPIErrorExt(tif->tif_clientdata, module,
                     "Directory has already been written");
        return 0;
    }

    tif->tif_dir.td_deferstrilearraywriting = TRUE;
    return 1;
}

/*
 * Similar to NDPIWriteDirectory(), writes the directory out
 * but leaves all data structures in memory so that it can be
 * written again.  This will make a partially written TIFF file
 * readable before it is successfully completed/closed.
 */
int
NDPICheckpointDirectory(TIFF* tif)
{
	int rc;
	/* Setup the strips arrays, if they haven't already been. */
	if (tif->tif_dir.td_stripoffset_p == NULL)
	    (void) NDPISetupStrips(tif);
	rc = NDPIWriteDirectorySec(tif,TRUE,FALSE,NULL);
	(void) NDPISetWriteOffset(tif, NDPISeekFile(tif, 0, SEEK_END));
	return rc;
}

int
NDPIWriteCustomDirectory(TIFF* tif, uint64_t* pdiroff)
{
	return NDPIWriteDirectorySec(tif,FALSE,FALSE,pdiroff);
}

/*
 * Similar to NDPIWriteDirectory(), but if the directory has already
 * been written once, it is relocated to the end of the file, in case it
 * has changed in size.  Note that this will result in the loss of the
 * previously used directory space. 
 */ 
int
NDPIRewriteDirectory( TIFF *tif )
{
	static const char module[] = "NDPIRewriteDirectory";

	/* We don't need to do anything special if it hasn't been written. */
	if( tif->tif_diroff == 0 )
		return NDPIWriteDirectory( tif );

	/*
	 * Find and zero the pointer to this directory, so that TIFFLinkDirectory
	 * will cause it to be added after this directories current pre-link.
	 */

	if (!(tif->tif_flags&TIFF_BIGTIFF))
	{
		if (tif->tif_header.classic.tiff_diroff == tif->tif_diroff)
		{
			tif->tif_header.classic.tiff_diroff = 0;
			tif->tif_diroff = 0;

			NDPISeekFile(tif,4,SEEK_SET);
			if (!WriteOK(tif, &(tif->tif_header.classic.tiff_diroff),4))
			{
				NDPIErrorExt(tif->tif_clientdata, tif->tif_name,
				    "Error updating TIFF header");
				return (0);
			}
		}
		else
		{
			uint32_t nextdir;
			nextdir = tif->tif_header.classic.tiff_diroff;
			while(1) {
				uint16_t dircount;
				uint32_t nextnextdir;

				if (!SeekOK(tif, nextdir) ||
				    !ReadOK(tif, &dircount, 2)) {
					NDPIErrorExt(tif->tif_clientdata, module,
					     "Error fetching directory count");
					return (0);
				}
				if (tif->tif_flags & TIFF_SWAB)
					NDPISwabShort(&dircount);
				(void) NDPISeekFile(tif,
				    nextdir+2+dircount*12, SEEK_SET);
				if (!ReadOK(tif, &nextnextdir, 4)) {
					NDPIErrorExt(tif->tif_clientdata, module,
					     "Error fetching directory link");
					return (0);
				}
				if (tif->tif_flags & TIFF_SWAB)
					NDPISwabLong(&nextnextdir);
				if (nextnextdir==tif->tif_diroff)
				{
					uint32_t m;
					m=0;
					(void) NDPISeekFile(tif,
					    nextdir+2+dircount*12, SEEK_SET);
					if (!WriteOK(tif, &m, 4)) {
						NDPIErrorExt(tif->tif_clientdata, module,
						     "Error writing directory link");
						return (0);
					}
					tif->tif_diroff=0;
					break;
				}
				nextdir=nextnextdir;
			}
		}
	}
	else
	{
		if (tif->tif_header.big.tiff_diroff == tif->tif_diroff)
		{
			tif->tif_header.big.tiff_diroff = 0;
			tif->tif_diroff = 0;

			NDPISeekFile(tif,8,SEEK_SET);
			if (!WriteOK(tif, &(tif->tif_header.big.tiff_diroff),8))
			{
				NDPIErrorExt(tif->tif_clientdata, tif->tif_name,
				    "Error updating TIFF header");
				return (0);
			}
		}
		else
		{
			uint64_t nextdir;
			nextdir = tif->tif_header.big.tiff_diroff;
			while(1) {
				uint64_t dircount64;
				uint16_t dircount;
				uint64_t nextnextdir;

				if (!SeekOK(tif, nextdir) ||
				    !ReadOK(tif, &dircount64, 8)) {
					NDPIErrorExt(tif->tif_clientdata, module,
					     "Error fetching directory count");
					return (0);
				}
				if (tif->tif_flags & TIFF_SWAB)
					NDPISwabLong8(&dircount64);
				if (dircount64>0xFFFF)
				{
					NDPIErrorExt(tif->tif_clientdata, module,
					     "Sanity check on tag count failed, likely corrupt TIFF");
					return (0);
				}
				dircount=(uint16_t)dircount64;
				(void) NDPISeekFile(tif,
				    nextdir+8+dircount*20, SEEK_SET);
				if (!ReadOK(tif, &nextnextdir, 8)) {
					NDPIErrorExt(tif->tif_clientdata, module,
					     "Error fetching directory link");
					return (0);
				}
				if (tif->tif_flags & TIFF_SWAB)
					NDPISwabLong8(&nextnextdir);
				if (nextnextdir==tif->tif_diroff)
				{
					uint64_t m;
					m=0;
					(void) NDPISeekFile(tif,
					    nextdir+8+dircount*20, SEEK_SET);
					if (!WriteOK(tif, &m, 8)) {
						NDPIErrorExt(tif->tif_clientdata, module,
						     "Error writing directory link");
						return (0);
					}
					tif->tif_diroff=0;
					break;
				}
				nextdir=nextnextdir;
			}
		}
	}

	/*
	 * Now use NDPIWriteDirectory() normally.
	 */

	return NDPIWriteDirectory( tif );
}

static int
NDPIWriteDirectorySec(TIFF* tif, int isimage, int imagedone, uint64_t* pdiroff)
{
	static const char module[] = "NDPIWriteDirectorySec";
	uint32_t ndir;
	TIFFDirEntry* dir;
	uint32_t dirsize;
	void* dirmem;
	uint32_t m;
	if (tif->tif_mode == O_RDONLY)
		return (1);

        _NDPIFillStriles( tif );
        
	/*
	 * Clear write state so that subsequent images with
	 * different characteristics get the right buffers
	 * setup for them.
	 */
	if (imagedone)
	{
		if (tif->tif_flags & TIFF_POSTENCODE)
		{
			tif->tif_flags &= ~TIFF_POSTENCODE;
			if (!(*tif->tif_postencode)(tif))
			{
				NDPIErrorExt(tif->tif_clientdata,module,
				    "Error post-encoding before directory write");
				return (0);
			}
		}
		(*tif->tif_close)(tif);       /* shutdown encoder */
		/*
		 * Flush any data that might have been written
		 * by the compression close+cleanup routines.  But
                 * be careful not to write stuff if we didn't add data
                 * in the previous steps as the "rawcc" data may well be
                 * a previously read tile/strip in mixed read/write mode.
		 */
		if (tif->tif_rawcc > 0 
		    && (tif->tif_flags & TIFF_BEENWRITING) != 0 )
		{
		    if( !NDPIFlushData1(tif) )
                    {
			NDPIErrorExt(tif->tif_clientdata, module,
			    "Error flushing data before directory write");
			return (0);
                    }
		}
		if ((tif->tif_flags & TIFF_MYBUFFER) && tif->tif_rawdata)
		{
			_NDPIfree(tif->tif_rawdata);
			tif->tif_rawdata = NULL;
			tif->tif_rawcc = 0;
			tif->tif_rawdatasize = 0;
                        tif->tif_rawdataoff = 0;
                        tif->tif_rawdataloaded = 0;
		}
		tif->tif_flags &= ~(TIFF_BEENWRITING|TIFF_BUFFERSETUP);
	}
	dir=NULL;
	dirmem=NULL;
	dirsize=0;
	while (1)
	{
		ndir=0;
		if (isimage)
		{
			if (NDPIFieldSet(tif,FIELD_IMAGEDIMENSIONS))
			{
				if (!NDPIWriteDirectoryTagShortLong(tif,&ndir,dir,TIFFTAG_IMAGEWIDTH,tif->tif_dir.td_imagewidth))
					goto bad;
				if (!NDPIWriteDirectoryTagShortLong(tif,&ndir,dir,TIFFTAG_IMAGELENGTH,tif->tif_dir.td_imagelength))
					goto bad;
			}
			if (NDPIFieldSet(tif,FIELD_TILEDIMENSIONS))
			{
				if (!NDPIWriteDirectoryTagShortLong(tif,&ndir,dir,TIFFTAG_TILEWIDTH,tif->tif_dir.td_tilewidth))
					goto bad;
				if (!NDPIWriteDirectoryTagShortLong(tif,&ndir,dir,TIFFTAG_TILELENGTH,tif->tif_dir.td_tilelength))
					goto bad;
			}
			if (NDPIFieldSet(tif,FIELD_RESOLUTION))
			{
				if (!NDPIWriteDirectoryTagRational(tif,&ndir,dir,TIFFTAG_XRESOLUTION,tif->tif_dir.td_xresolution))
					goto bad;
				if (!NDPIWriteDirectoryTagRational(tif,&ndir,dir,TIFFTAG_YRESOLUTION,tif->tif_dir.td_yresolution))
					goto bad;
			}
			if (NDPIFieldSet(tif,FIELD_POSITION))
			{
				if (!NDPIWriteDirectoryTagRational(tif,&ndir,dir,TIFFTAG_XPOSITION,tif->tif_dir.td_xposition))
					goto bad;
				if (!NDPIWriteDirectoryTagRational(tif,&ndir,dir,TIFFTAG_YPOSITION,tif->tif_dir.td_yposition))
					goto bad;
			}
			if (NDPIFieldSet(tif,FIELD_SUBFILETYPE))
			{
				if (!NDPIWriteDirectoryTagLong(tif,&ndir,dir,TIFFTAG_SUBFILETYPE,tif->tif_dir.td_subfiletype))
					goto bad;
			}
			if (NDPIFieldSet(tif,FIELD_BITSPERSAMPLE))
			{
				if (!NDPIWriteDirectoryTagShortPerSample(tif,&ndir,dir,TIFFTAG_BITSPERSAMPLE,tif->tif_dir.td_bitspersample))
					goto bad;
			}
			if (NDPIFieldSet(tif,FIELD_COMPRESSION))
			{
				if (!NDPIWriteDirectoryTagShort(tif,&ndir,dir,TIFFTAG_COMPRESSION,tif->tif_dir.td_compression))
					goto bad;
			}
			if (NDPIFieldSet(tif,FIELD_PHOTOMETRIC))
			{
				if (!NDPIWriteDirectoryTagShort(tif,&ndir,dir,TIFFTAG_PHOTOMETRIC,tif->tif_dir.td_photometric))
					goto bad;
			}
			if (NDPIFieldSet(tif,FIELD_THRESHHOLDING))
			{
				if (!NDPIWriteDirectoryTagShort(tif,&ndir,dir,TIFFTAG_THRESHHOLDING,tif->tif_dir.td_threshholding))
					goto bad;
			}
			if (NDPIFieldSet(tif,FIELD_FILLORDER))
			{
				if (!NDPIWriteDirectoryTagShort(tif,&ndir,dir,TIFFTAG_FILLORDER,tif->tif_dir.td_fillorder))
					goto bad;
			}
			if (NDPIFieldSet(tif,FIELD_ORIENTATION))
			{
				if (!NDPIWriteDirectoryTagShort(tif,&ndir,dir,TIFFTAG_ORIENTATION,tif->tif_dir.td_orientation))
					goto bad;
			}
			if (NDPIFieldSet(tif,FIELD_SAMPLESPERPIXEL))
			{
				if (!NDPIWriteDirectoryTagShort(tif,&ndir,dir,TIFFTAG_SAMPLESPERPIXEL,tif->tif_dir.td_samplesperpixel))
					goto bad;
			}
			if (NDPIFieldSet(tif,FIELD_ROWSPERSTRIP))
			{
				if (!NDPIWriteDirectoryTagShortLong(tif,&ndir,dir,TIFFTAG_ROWSPERSTRIP,tif->tif_dir.td_rowsperstrip))
					goto bad;
			}
			if (NDPIFieldSet(tif,FIELD_MINSAMPLEVALUE))
			{
				if (!NDPIWriteDirectoryTagShortPerSample(tif,&ndir,dir,TIFFTAG_MINSAMPLEVALUE,tif->tif_dir.td_minsamplevalue))
					goto bad;
			}
			if (NDPIFieldSet(tif,FIELD_MAXSAMPLEVALUE))
			{
				if (!NDPIWriteDirectoryTagShortPerSample(tif,&ndir,dir,TIFFTAG_MAXSAMPLEVALUE,tif->tif_dir.td_maxsamplevalue))
					goto bad;
			}
			if (NDPIFieldSet(tif,FIELD_PLANARCONFIG))
			{
				if (!NDPIWriteDirectoryTagShort(tif,&ndir,dir,TIFFTAG_PLANARCONFIG,tif->tif_dir.td_planarconfig))
					goto bad;
			}
			if (NDPIFieldSet(tif,FIELD_RESOLUTIONUNIT))
			{
				if (!NDPIWriteDirectoryTagShort(tif,&ndir,dir,TIFFTAG_RESOLUTIONUNIT,tif->tif_dir.td_resolutionunit))
					goto bad;
			}
			if (NDPIFieldSet(tif,FIELD_PAGENUMBER))
			{
				if (!NDPIWriteDirectoryTagShortArray(tif,&ndir,dir,TIFFTAG_PAGENUMBER,2,&tif->tif_dir.td_pagenumber[0]))
					goto bad;
			}
			if (NDPIFieldSet(tif,FIELD_STRIPBYTECOUNTS))
			{
				if (!isTiled(tif))
				{
					if (!NDPIWriteDirectoryTagLongLong8Array(tif,&ndir,dir,TIFFTAG_STRIPBYTECOUNTS,tif->tif_dir.td_nstrips,tif->tif_dir.td_stripbytecount_p))
						goto bad;
				}
				else
				{
					if (!NDPIWriteDirectoryTagLongLong8Array(tif,&ndir,dir,TIFFTAG_TILEBYTECOUNTS,tif->tif_dir.td_nstrips,tif->tif_dir.td_stripbytecount_p))
						goto bad;
				}
			}
			if (NDPIFieldSet(tif,FIELD_STRIPOFFSETS))
			{
				if (!isTiled(tif))
				{
                    /* td_stripoffset_p might be NULL in an odd OJPEG case. See
                     *  tif_dirread.c around line 3634.
                     * XXX: OJPEG hack.
                     * If a) compression is OJPEG, b) it's not a tiled TIFF,
                     * and c) the number of strips is 1,
                     * then we tolerate the absence of stripoffsets tag,
                     * because, presumably, all required data is in the
                     * JpegInterchangeFormat stream.
                     * We can get here when using tiffset on such a file.
                     * See http://bugzilla.maptools.org/show_bug.cgi?id=2500
                    */
                    if (tif->tif_dir.td_stripoffset_p != NULL &&
                        !NDPIWriteDirectoryTagLongLong8Array(tif,&ndir,dir,TIFFTAG_STRIPOFFSETS,tif->tif_dir.td_nstrips,tif->tif_dir.td_stripoffset_p))
                        goto bad;
				}
				else
				{
					if (!NDPIWriteDirectoryTagLongLong8Array(tif,&ndir,dir,TIFFTAG_TILEOFFSETS,tif->tif_dir.td_nstrips,tif->tif_dir.td_stripoffset_p))
						goto bad;
				}
			}
			if (NDPIFieldSet(tif,FIELD_COLORMAP))
			{
				if (!NDPIWriteDirectoryTagColormap(tif,&ndir,dir))
					goto bad;
			}
			if (NDPIFieldSet(tif,FIELD_EXTRASAMPLES))
			{
				if (tif->tif_dir.td_extrasamples)
				{
					uint16_t na;
					uint16_t* nb;
					NDPIGetFieldDefaulted(tif,TIFFTAG_EXTRASAMPLES,&na,&nb);
					if (!NDPIWriteDirectoryTagShortArray(tif,&ndir,dir,TIFFTAG_EXTRASAMPLES,na,nb))
						goto bad;
				}
			}
			if (NDPIFieldSet(tif,FIELD_SAMPLEFORMAT))
			{
				if (!NDPIWriteDirectoryTagShortPerSample(tif,&ndir,dir,TIFFTAG_SAMPLEFORMAT,tif->tif_dir.td_sampleformat))
					goto bad;
			}
			if (NDPIFieldSet(tif,FIELD_SMINSAMPLEVALUE))
			{
				if (!NDPIWriteDirectoryTagSampleformatArray(tif,&ndir,dir,TIFFTAG_SMINSAMPLEVALUE,tif->tif_dir.td_samplesperpixel,tif->tif_dir.td_sminsamplevalue))
					goto bad;
			}
			if (NDPIFieldSet(tif,FIELD_SMAXSAMPLEVALUE))
			{
				if (!NDPIWriteDirectoryTagSampleformatArray(tif,&ndir,dir,TIFFTAG_SMAXSAMPLEVALUE,tif->tif_dir.td_samplesperpixel,tif->tif_dir.td_smaxsamplevalue))
					goto bad;
			}
			if (NDPIFieldSet(tif,FIELD_IMAGEDEPTH))
			{
				if (!NDPIWriteDirectoryTagLong(tif,&ndir,dir,TIFFTAG_IMAGEDEPTH,tif->tif_dir.td_imagedepth))
					goto bad;
			}
			if (NDPIFieldSet(tif,FIELD_TILEDEPTH))
			{
				if (!NDPIWriteDirectoryTagLong(tif,&ndir,dir,TIFFTAG_TILEDEPTH,tif->tif_dir.td_tiledepth))
					goto bad;
			}
			if (NDPIFieldSet(tif,FIELD_HALFTONEHINTS))
			{
				if (!NDPIWriteDirectoryTagShortArray(tif,&ndir,dir,TIFFTAG_HALFTONEHINTS,2,&tif->tif_dir.td_halftonehints[0]))
					goto bad;
			}
			if (NDPIFieldSet(tif,FIELD_YCBCRSUBSAMPLING))
			{
				if (!NDPIWriteDirectoryTagShortArray(tif,&ndir,dir,TIFFTAG_YCBCRSUBSAMPLING,2,&tif->tif_dir.td_ycbcrsubsampling[0]))
					goto bad;
			}
			if (NDPIFieldSet(tif,FIELD_YCBCRPOSITIONING))
			{
				if (!NDPIWriteDirectoryTagShort(tif,&ndir,dir,TIFFTAG_YCBCRPOSITIONING,tif->tif_dir.td_ycbcrpositioning))
					goto bad;
			}
			if (NDPIFieldSet(tif,FIELD_REFBLACKWHITE))
			{
				if (!NDPIWriteDirectoryTagRationalArray(tif,&ndir,dir,TIFFTAG_REFERENCEBLACKWHITE,6,tif->tif_dir.td_refblackwhite))
					goto bad;
			}
			if (NDPIFieldSet(tif,FIELD_TRANSFERFUNCTION))
			{
				if (!NDPIWriteDirectoryTagTransferfunction(tif,&ndir,dir))
					goto bad;
			}
			if (NDPIFieldSet(tif,FIELD_INKNAMES))
			{
				if (!NDPIWriteDirectoryTagAscii(tif,&ndir,dir,TIFFTAG_INKNAMES,tif->tif_dir.td_inknameslen,tif->tif_dir.td_inknames))
					goto bad;
			}
			if (NDPIFieldSet(tif,FIELD_SUBIFD))
			{
				if (!NDPIWriteDirectoryTagSubifd(tif,&ndir,dir))
					goto bad;
			}
			{
				uint32_t n;
				for (n=0; n<tif->tif_nfields; n++) {
					const TIFFField* o;
					o = tif->tif_fields[n];
					if ((o->field_bit>=FIELD_CODEC)&&(NDPIFieldSet(tif,o->field_bit)))
					{
						switch (o->get_field_type)
						{
							case TIFF_SETGET_ASCII:
								{
									uint32_t pa;
									char* pb;
									assert(o->field_type==TIFF_ASCII);
									assert(o->field_readcount==TIFF_VARIABLE);
									assert(o->field_passcount==0);
									NDPIGetField(tif,o->field_tag,&pb);
									pa=(uint32_t)(strlen(pb));
									if (!NDPIWriteDirectoryTagAscii(tif, &ndir, dir, (uint16_t)o->field_tag, pa, pb))
										goto bad;
								}
								break;
							case TIFF_SETGET_UINT16:
								{
									uint16_t p;
									assert(o->field_type==TIFF_SHORT);
									assert(o->field_readcount==1);
									assert(o->field_passcount==0);
									NDPIGetField(tif,o->field_tag,&p);
									if (!NDPIWriteDirectoryTagShort(tif, &ndir, dir, (uint16_t)o->field_tag, p))
										goto bad;
								}
								break;
							case TIFF_SETGET_UINT32:
								{
									uint32_t p;
									assert(o->field_type==TIFF_LONG);
									assert(o->field_readcount==1);
									assert(o->field_passcount==0);
									NDPIGetField(tif,o->field_tag,&p);
									if (!NDPIWriteDirectoryTagLong(tif, &ndir, dir, (uint16_t)o->field_tag, p))
										goto bad;
								}
								break;
							case TIFF_SETGET_C32_UINT8:
								{
									uint32_t pa;
									void* pb;
									assert(o->field_type==TIFF_UNDEFINED);
									assert(o->field_readcount==TIFF_VARIABLE2);
									assert(o->field_passcount==1);
									NDPIGetField(tif,o->field_tag,&pa,&pb);
									if (!NDPIWriteDirectoryTagUndefinedArray(tif, &ndir, dir, (uint16_t)o->field_tag, pa, pb))
										goto bad;
								}
								break;
							default:
								NDPIErrorExt(tif->tif_clientdata,module,
								            "Cannot write tag %"PRIu32" (%s)",
								            TIFFFieldTag(o),
                                                                            o->field_name ? o->field_name : "unknown");
								goto bad;
						}
					}
				}
			}
		}
		for (m=0; m<(uint32_t)(tif->tif_dir.td_customValueCount); m++)
		{
                        uint16_t tag = (uint16_t)tif->tif_dir.td_customValues[m].info->field_tag;
                        uint32_t count = tif->tif_dir.td_customValues[m].count;
			switch (tif->tif_dir.td_customValues[m].info->field_type)
			{
				case TIFF_ASCII:
					if (!NDPIWriteDirectoryTagAscii(tif,&ndir,dir,tag,count,tif->tif_dir.td_customValues[m].value))
						goto bad;
					break;
				case TIFF_UNDEFINED:
					if (!NDPIWriteDirectoryTagUndefinedArray(tif,&ndir,dir,tag,count,tif->tif_dir.td_customValues[m].value))
						goto bad;
					break;
				case TIFF_BYTE:
					if (!NDPIWriteDirectoryTagByteArray(tif,&ndir,dir,tag,count,tif->tif_dir.td_customValues[m].value))
						goto bad;
					break;
				case TIFF_SBYTE:
					if (!NDPIWriteDirectoryTagSbyteArray(tif,&ndir,dir,tag,count,tif->tif_dir.td_customValues[m].value))
						goto bad;
					break;
				case TIFF_SHORT:
					if (!NDPIWriteDirectoryTagShortArray(tif,&ndir,dir,tag,count,tif->tif_dir.td_customValues[m].value))
						goto bad;
					break;
				case TIFF_SSHORT:
					if (!NDPIWriteDirectoryTagSshortArray(tif,&ndir,dir,tag,count,tif->tif_dir.td_customValues[m].value))
						goto bad;
					break;
				case TIFF_LONG:
					if (!NDPIWriteDirectoryTagLongArray(tif,&ndir,dir,tag,count,tif->tif_dir.td_customValues[m].value))
						goto bad;
					break;
				case TIFF_SLONG:
					if (!NDPIWriteDirectoryTagSlongArray(tif,&ndir,dir,tag,count,tif->tif_dir.td_customValues[m].value))
						goto bad;
					break;
				case TIFF_LONG8:
					if (!NDPIWriteDirectoryTagLong8Array(tif,&ndir,dir,tag,count,tif->tif_dir.td_customValues[m].value))
						goto bad;
					break;
				case TIFF_SLONG8:
					if (!NDPIWriteDirectoryTagSlong8Array(tif,&ndir,dir,tag,count,tif->tif_dir.td_customValues[m].value))
						goto bad;
					break;
				case TIFF_RATIONAL:
					{
						/*-- Rational2Double: For Rationals evaluate "set_field_type" to determine internal storage size. */
						int tv_size;
						tv_size = _NDPISetGetFieldSize(tif->tif_dir.td_customValues[m].info->set_field_type);
						if (tv_size == 8) {
							if (!NDPIWriteDirectoryTagRationalDoubleArray(tif,&ndir,dir,tag,count,tif->tif_dir.td_customValues[m].value))
								goto bad;
						} else {
							/*-- default should be tv_size == 4 */
							if (!NDPIWriteDirectoryTagRationalArray(tif,&ndir,dir,tag,count,tif->tif_dir.td_customValues[m].value))
								goto bad;
							/*-- ToDo: After Testing, this should be removed and tv_size==4 should be set as default. */
							if (tv_size != 4) {
								NDPIErrorExt(0,"TIFFLib: _TIFFWriteDirectorySec()", "Rational2Double: .set_field_type in not 4 but %d", tv_size); 
							}
						}
					}
					break;
				case TIFF_SRATIONAL:
					{
						/*-- Rational2Double: For Rationals evaluate "set_field_type" to determine internal storage size. */
						int tv_size;
						tv_size = _NDPISetGetFieldSize(tif->tif_dir.td_customValues[m].info->set_field_type);
						if (tv_size == 8) {
							if (!NDPIWriteDirectoryTagSrationalDoubleArray(tif,&ndir,dir,tag,count,tif->tif_dir.td_customValues[m].value))
								goto bad;
						} else {
							/*-- default should be tv_size == 4 */
							if (!NDPIWriteDirectoryTagSrationalArray(tif,&ndir,dir,tag,count,tif->tif_dir.td_customValues[m].value))
								goto bad;
							/*-- ToDo: After Testing, this should be removed and tv_size==4 should be set as default. */
							if (tv_size != 4) {
								NDPIErrorExt(0,"TIFFLib: _TIFFWriteDirectorySec()", "Rational2Double: .set_field_type in not 4 but %d", tv_size); 
							}
						}
					}
					break;
				case TIFF_FLOAT:
					if (!NDPIWriteDirectoryTagFloatArray(tif,&ndir,dir,tag,count,tif->tif_dir.td_customValues[m].value))
						goto bad;
					break;
				case TIFF_DOUBLE:
					if (!NDPIWriteDirectoryTagDoubleArray(tif,&ndir,dir,tag,count,tif->tif_dir.td_customValues[m].value))
						goto bad;
					break;
				case TIFF_IFD:
					if (!NDPIWriteDirectoryTagIfdArray(tif,&ndir,dir,tag,count,tif->tif_dir.td_customValues[m].value))
						goto bad;
					break;
				case TIFF_IFD8:
					if (!NDPIWriteDirectoryTagIfdIfd8Array(tif,&ndir,dir,tag,count,tif->tif_dir.td_customValues[m].value))
						goto bad;
					break;
				default:
					assert(0);   /* we should never get here */
					break;
			}
		}
		if (dir!=NULL)
			break;
		dir=_NDPImalloc(ndir*sizeof(TIFFDirEntry));
		if (dir==NULL)
		{
			NDPIErrorExt(tif->tif_clientdata,module,"Out of memory");
			goto bad;
		}
		if (isimage)
		{
			if ((tif->tif_diroff==0)&&(!TIFFLinkDirectory(tif)))
				goto bad;
		}
		else
			tif->tif_diroff=(NDPISeekFile(tif,0,SEEK_END)+1)&(~((toff_t)1));
		if (pdiroff!=NULL)
			*pdiroff=tif->tif_diroff;
		if (!(tif->tif_flags&TIFF_BIGTIFF))
			dirsize=2+ndir*12+4;
		else
			dirsize=8+ndir*20+8;
		tif->tif_dataoff=tif->tif_diroff+dirsize;
		if (!(tif->tif_flags&TIFF_BIGTIFF))
			tif->tif_dataoff=(uint32_t)tif->tif_dataoff;
		if ((tif->tif_dataoff<tif->tif_diroff)||(tif->tif_dataoff<(uint64_t)dirsize))
		{
			NDPIErrorExt(tif->tif_clientdata,module,"Maximum TIFF file size exceeded");
			goto bad;
		}
		if (tif->tif_dataoff&1)
			tif->tif_dataoff++;
		if (isimage)
			tif->tif_curdir++;
	}
	if (isimage)
	{
		if (NDPIFieldSet(tif,FIELD_SUBIFD)&&(tif->tif_subifdoff==0))
		{
			uint32_t na;
			TIFFDirEntry* nb;
			for (na=0, nb=dir; ; na++, nb++)
			{
				if( na == ndir )
                                {
                                    NDPIErrorExt(tif->tif_clientdata,module,
                                                 "Cannot find SubIFD tag");
                                    goto bad;
                                }
				if (nb->tdir_tag==TIFFTAG_SUBIFD)
					break;
			}
			if (!(tif->tif_flags&TIFF_BIGTIFF))
				tif->tif_subifdoff=tif->tif_diroff+2+na*12+8;
			else
				tif->tif_subifdoff=tif->tif_diroff+8+na*20+12;
		}
	}
	dirmem=_NDPImalloc(dirsize);
	if (dirmem==NULL)
	{
		NDPIErrorExt(tif->tif_clientdata,module,"Out of memory");
		goto bad;
	}
	if (!(tif->tif_flags&TIFF_BIGTIFF))
	{
		uint8_t* n;
		uint32_t nTmp;
		TIFFDirEntry* o;
		n=dirmem;
		*(uint16_t*)n=(uint16_t)ndir;
		if (tif->tif_flags&TIFF_SWAB)
			NDPISwabShort((uint16_t*)n);
		n+=2;
		o=dir;
		for (m=0; m<ndir; m++)
		{
			*(uint16_t*)n=o->tdir_tag;
			if (tif->tif_flags&TIFF_SWAB)
				NDPISwabShort((uint16_t*)n);
			n+=2;
			*(uint16_t*)n=o->tdir_type;
			if (tif->tif_flags&TIFF_SWAB)
				NDPISwabShort((uint16_t*)n);
			n+=2;
			nTmp = (uint32_t)o->tdir_count;
			_NDPImemcpy(n,&nTmp,4);
			if (tif->tif_flags&TIFF_SWAB)
				NDPISwabLong((uint32_t*)n);
			n+=4;
			/* This is correct. The data has been */
			/* swabbed previously in NDPIWriteDirectoryTagData */
			_NDPImemcpy(n,&o->tdir_offset,4);
			n+=4;
			o++;
		}
		nTmp = (uint32_t)tif->tif_nextdiroff;
		if (tif->tif_flags&TIFF_SWAB)
			NDPISwabLong(&nTmp);
		_NDPImemcpy(n,&nTmp,4);
	}
	else
	{
		uint8_t* n;
		TIFFDirEntry* o;
		n=dirmem;
		*(uint64_t*)n=ndir;
		if (tif->tif_flags&TIFF_SWAB)
			NDPISwabLong8((uint64_t*)n);
		n+=8;
		o=dir;
		for (m=0; m<ndir; m++)
		{
			*(uint16_t*)n=o->tdir_tag;
			if (tif->tif_flags&TIFF_SWAB)
				NDPISwabShort((uint16_t*)n);
			n+=2;
			*(uint16_t*)n=o->tdir_type;
			if (tif->tif_flags&TIFF_SWAB)
				NDPISwabShort((uint16_t*)n);
			n+=2;
			_NDPImemcpy(n,&o->tdir_count,8);
			if (tif->tif_flags&TIFF_SWAB)
				NDPISwabLong8((uint64_t*)n);
			n+=8;
			_NDPImemcpy(n,&o->tdir_offset,8);
			n+=8;
			o++;
		}
		_NDPImemcpy(n,&tif->tif_nextdiroff,8);
		if (tif->tif_flags&TIFF_SWAB)
			NDPISwabLong8((uint64_t*)n);
	}
	_NDPIfree(dir);
	dir=NULL;
	if (!SeekOK(tif,tif->tif_diroff))
	{
		NDPIErrorExt(tif->tif_clientdata,module,"IO error writing directory");
		goto bad;
	}
	if (!WriteOK(tif,dirmem,(tmsize_t)dirsize))
	{
		NDPIErrorExt(tif->tif_clientdata,module,"IO error writing directory");
		goto bad;
	}
	_NDPIfree(dirmem);
	if (imagedone)
	{
		NDPIFreeDirectory(tif);
		tif->tif_flags &= ~TIFF_DIRTYDIRECT;
		tif->tif_flags &= ~TIFF_DIRTYSTRIP;
		(*tif->tif_cleanup)(tif);
		/*
		* Reset directory-related state for subsequent
		* directories.
		*/
		NDPICreateDirectory(tif);
	}
	return(1);
bad:
	if (dir!=NULL)
		_NDPIfree(dir);
	if (dirmem!=NULL)
		_NDPIfree(dirmem);
	return(0);
}

static int8_t NDPIClampDoubleToInt8(double val )
{
    if( val > 127 )
        return 127;
    if( val < -128 || val != val )
        return -128;
    return (int8_t)val;
}

static int16_t NDPIClampDoubleToInt16(double val )
{
    if( val > 32767 )
        return 32767;
    if( val < -32768 || val != val )
        return -32768;
    return (int16_t)val;
}

static int32_t NDPIClampDoubleToInt32(double val )
{
    if( val > 0x7FFFFFFF )
        return 0x7FFFFFFF;
    if( val < -0x7FFFFFFF-1 || val != val )
        return -0x7FFFFFFF-1;
    return (int32_t)val;
}

static uint8_t NDPIClampDoubleToUInt8(double val )
{
    if( val < 0 )
        return 0;
    if( val > 255 || val != val )
        return 255;
    return (uint8_t)val;
}

static uint16_t NDPIClampDoubleToUInt16(double val )
{
    if( val < 0 )
        return 0;
    if( val > 65535 || val != val )
        return 65535;
    return (uint16_t)val;
}

static uint32_t NDPIClampDoubleToUInt32(double val )
{
    if( val < 0 )
        return 0;
    if( val > 0xFFFFFFFFU || val != val )
        return 0xFFFFFFFFU;
    return (uint32_t)val;
}

static int
NDPIWriteDirectoryTagSampleformatArray(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, double* value)
{
	static const char module[] = "NDPIWriteDirectoryTagSampleformatArray";
	void* conv;
	uint32_t i;
	int ok;
	conv = _NDPImalloc(count*sizeof(double));
	if (conv == NULL)
	{
		NDPIErrorExt(tif->tif_clientdata, module, "Out of memory");
		return (0);
	}

	switch (tif->tif_dir.td_sampleformat)
	{
		case SAMPLEFORMAT_IEEEFP:
			if (tif->tif_dir.td_bitspersample<=32)
			{
				for (i = 0; i < count; ++i)
					((float*)conv)[i] = _NDPIClampDoubleToFloat(value[i]);
				ok = NDPIWriteDirectoryTagFloatArray(tif,ndir,dir,tag,count,(float*)conv);
			}
			else
			{
				ok = NDPIWriteDirectoryTagDoubleArray(tif,ndir,dir,tag,count,value);
			}
			break;
		case SAMPLEFORMAT_INT:
			if (tif->tif_dir.td_bitspersample<=8)
			{
				for (i = 0; i < count; ++i)
					((int8_t*)conv)[i] = NDPIClampDoubleToInt8(value[i]);
				ok = NDPIWriteDirectoryTagSbyteArray(tif,ndir,dir,tag,count,(int8_t*)conv);
			}
			else if (tif->tif_dir.td_bitspersample<=16)
			{
				for (i = 0; i < count; ++i)
					((int16_t*)conv)[i] = NDPIClampDoubleToInt16(value[i]);
				ok = NDPIWriteDirectoryTagSshortArray(tif,ndir,dir,tag,count,(int16_t*)conv);
			}
			else
			{
				for (i = 0; i < count; ++i)
					((int32_t*)conv)[i] = NDPIClampDoubleToInt32(value[i]);
				ok = NDPIWriteDirectoryTagSlongArray(tif,ndir,dir,tag,count,(int32_t*)conv);
			}
			break;
		case SAMPLEFORMAT_UINT:
			if (tif->tif_dir.td_bitspersample<=8)
			{
				for (i = 0; i < count; ++i)
					((uint8_t*)conv)[i] = NDPIClampDoubleToUInt8(value[i]);
				ok = NDPIWriteDirectoryTagByteArray(tif,ndir,dir,tag,count,(uint8_t*)conv);
			}
			else if (tif->tif_dir.td_bitspersample<=16)
			{
				for (i = 0; i < count; ++i)
					((uint16_t*)conv)[i] = NDPIClampDoubleToUInt16(value[i]);
				ok = NDPIWriteDirectoryTagShortArray(tif,ndir,dir,tag,count,(uint16_t*)conv);
			}
			else
			{
				for (i = 0; i < count; ++i)
					((uint32_t*)conv)[i] = NDPIClampDoubleToUInt32(value[i]);
				ok = NDPIWriteDirectoryTagLongArray(tif,ndir,dir,tag,count,(uint32_t*)conv);
			}
			break;
		default:
			ok = 0;
	}

	_NDPIfree(conv);
	return (ok);
}

#if 0
static int
NDPIWriteDirectoryTagSampleformatPerSample(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, double value)
{
	switch (tif->tif_dir.td_sampleformat)
	{
		case SAMPLEFORMAT_IEEEFP:
			if (tif->tif_dir.td_bitspersample<=32)
				return(NDPIWriteDirectoryTagFloatPerSample(tif,ndir,dir,tag,(float)value));
			else
				return(NDPIWriteDirectoryTagDoublePerSample(tif,ndir,dir,tag,value));
		case SAMPLEFORMAT_INT:
			if (tif->tif_dir.td_bitspersample<=8)
				return(NDPIWriteDirectoryTagSbytePerSample(tif,ndir,dir,tag,(int8_t)value));
			else if (tif->tif_dir.td_bitspersample<=16)
				return(NDPIWriteDirectoryTagSshortPerSample(tif,ndir,dir,tag,(int16_t)value));
			else
				return(NDPIWriteDirectoryTagSlongPerSample(tif,ndir,dir,tag,(int32_t)value));
		case SAMPLEFORMAT_UINT:
			if (tif->tif_dir.td_bitspersample<=8)
				return(NDPIWriteDirectoryTagBytePerSample(tif,ndir,dir,tag,(uint8_t)value));
			else if (tif->tif_dir.td_bitspersample<=16)
				return(NDPIWriteDirectoryTagShortPerSample(tif,ndir,dir,tag,(uint16_t)value));
			else
				return(NDPIWriteDirectoryTagLongPerSample(tif,ndir,dir,tag,(uint32_t)value));
		default:
			return(1);
	}
}
#endif

static int
NDPIWriteDirectoryTagAscii(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, char* value)
{
	if (dir==NULL)
	{
		(*ndir)++;
		return(1);
	}
	return(NDPIWriteDirectoryTagCheckedAscii(tif,ndir,dir,tag,count,value));
}

static int
NDPIWriteDirectoryTagUndefinedArray(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, uint8_t* value)
{
	if (dir==NULL)
	{
		(*ndir)++;
		return(1);
	}
	return(NDPIWriteDirectoryTagCheckedUndefinedArray(tif,ndir,dir,tag,count,value));
}

#ifdef notdef
static int
NDPIWriteDirectoryTagByte(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint8_t value)
{
	if (dir==NULL)
	{
		(*ndir)++;
		return(1);
	}
	return(NDPIWriteDirectoryTagCheckedByte(tif,ndir,dir,tag,value));
}
#endif

static int
NDPIWriteDirectoryTagByteArray(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, uint8_t* value)
{
	if (dir==NULL)
	{
		(*ndir)++;
		return(1);
	}
	return(NDPIWriteDirectoryTagCheckedByteArray(tif,ndir,dir,tag,count,value));
}

#if 0
static int
NDPIWriteDirectoryTagBytePerSample(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint8_t value)
{
	static const char module[] = "NDPIWriteDirectoryTagBytePerSample";
	uint8_t* m;
	uint8_t* na;
	uint16_t nb;
	int o;
	if (dir==NULL)
	{
		(*ndir)++;
		return(1);
	}
	m=_NDPImalloc(tif->tif_dir.td_samplesperpixel*sizeof(uint8_t));
	if (m==NULL)
	{
		NDPIErrorExt(tif->tif_clientdata,module,"Out of memory");
		return(0);
	}
	for (na=m, nb=0; nb<tif->tif_dir.td_samplesperpixel; na++, nb++)
		*na=value;
	o=NDPIWriteDirectoryTagCheckedByteArray(tif,ndir,dir,tag,tif->tif_dir.td_samplesperpixel,m);
	_NDPIfree(m);
	return(o);
}
#endif

#ifdef notdef
static int
NDPIWriteDirectoryTagSbyte(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, int8_t value)
{
	if (dir==NULL)
	{
		(*ndir)++;
		return(1);
	}
	return(NDPIWriteDirectoryTagCheckedSbyte(tif,ndir,dir,tag,value));
}
#endif

static int
NDPIWriteDirectoryTagSbyteArray(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, int8_t* value)
{
	if (dir==NULL)
	{
		(*ndir)++;
		return(1);
	}
	return(NDPIWriteDirectoryTagCheckedSbyteArray(tif,ndir,dir,tag,count,value));
}

#if 0
static int
NDPIWriteDirectoryTagSbytePerSample(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, int8_t value)
{
	static const char module[] = "NDPIWriteDirectoryTagSbytePerSample";
	int8_t* m;
	int8_t* na;
	uint16_t nb;
	int o;
	if (dir==NULL)
	{
		(*ndir)++;
		return(1);
	}
	m=_NDPImalloc(tif->tif_dir.td_samplesperpixel*sizeof(int8_t));
	if (m==NULL)
	{
		NDPIErrorExt(tif->tif_clientdata,module,"Out of memory");
		return(0);
	}
	for (na=m, nb=0; nb<tif->tif_dir.td_samplesperpixel; na++, nb++)
		*na=value;
	o=NDPIWriteDirectoryTagCheckedSbyteArray(tif,ndir,dir,tag,tif->tif_dir.td_samplesperpixel,m);
	_NDPIfree(m);
	return(o);
}
#endif

static int
NDPIWriteDirectoryTagShort(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint16_t value)
{
	if (dir==NULL)
	{
		(*ndir)++;
		return(1);
	}
	return(NDPIWriteDirectoryTagCheckedShort(tif,ndir,dir,tag,value));
}

static int
NDPIWriteDirectoryTagShortArray(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, uint16_t* value)
{
	if (dir==NULL)
	{
		(*ndir)++;
		return(1);
	}
	return(NDPIWriteDirectoryTagCheckedShortArray(tif,ndir,dir,tag,count,value));
}

static int
NDPIWriteDirectoryTagShortPerSample(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint16_t value)
{
	static const char module[] = "NDPIWriteDirectoryTagShortPerSample";
	uint16_t* m;
	uint16_t* na;
	uint16_t nb;
	int o;
	if (dir==NULL)
	{
		(*ndir)++;
		return(1);
	}
	m=_NDPImalloc(tif->tif_dir.td_samplesperpixel*sizeof(uint16_t));
	if (m==NULL)
	{
		NDPIErrorExt(tif->tif_clientdata,module,"Out of memory");
		return(0);
	}
	for (na=m, nb=0; nb<tif->tif_dir.td_samplesperpixel; na++, nb++)
		*na=value;
	o=NDPIWriteDirectoryTagCheckedShortArray(tif,ndir,dir,tag,tif->tif_dir.td_samplesperpixel,m);
	_NDPIfree(m);
	return(o);
}

#ifdef notdef
static int
NDPIWriteDirectoryTagSshort(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, int16_t value)
{
	if (dir==NULL)
	{
		(*ndir)++;
		return(1);
	}
	return(NDPIWriteDirectoryTagCheckedSshort(tif,ndir,dir,tag,value));
}
#endif

static int
NDPIWriteDirectoryTagSshortArray(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, int16_t* value)
{
	if (dir==NULL)
	{
		(*ndir)++;
		return(1);
	}
	return(NDPIWriteDirectoryTagCheckedSshortArray(tif,ndir,dir,tag,count,value));
}

#if 0
static int
NDPIWriteDirectoryTagSshortPerSample(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, int16_t value)
{
	static const char module[] = "NDPIWriteDirectoryTagSshortPerSample";
	int16_t* m;
	int16_t* na;
	uint16_t nb;
	int o;
	if (dir==NULL)
	{
		(*ndir)++;
		return(1);
	}
	m=_NDPImalloc(tif->tif_dir.td_samplesperpixel*sizeof(int16_t));
	if (m==NULL)
	{
		NDPIErrorExt(tif->tif_clientdata,module,"Out of memory");
		return(0);
	}
	for (na=m, nb=0; nb<tif->tif_dir.td_samplesperpixel; na++, nb++)
		*na=value;
	o=NDPIWriteDirectoryTagCheckedSshortArray(tif,ndir,dir,tag,tif->tif_dir.td_samplesperpixel,m);
	_NDPIfree(m);
	return(o);
}
#endif

static int
NDPIWriteDirectoryTagLong(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t value)
{
	if (dir==NULL)
	{
		(*ndir)++;
		return(1);
	}
	return(NDPIWriteDirectoryTagCheckedLong(tif,ndir,dir,tag,value));
}

static int
NDPIWriteDirectoryTagLongArray(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, uint32_t* value)
{
	if (dir==NULL)
	{
		(*ndir)++;
		return(1);
	}
	return(NDPIWriteDirectoryTagCheckedLongArray(tif,ndir,dir,tag,count,value));
}

#if 0
static int
NDPIWriteDirectoryTagLongPerSample(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t value)
{
	static const char module[] = "NDPIWriteDirectoryTagLongPerSample";
	uint32_t* m;
	uint32_t* na;
	uint16_t nb;
	int o;
	if (dir==NULL)
	{
		(*ndir)++;
		return(1);
	}
	m=_NDPImalloc(tif->tif_dir.td_samplesperpixel*sizeof(uint32_t));
	if (m==NULL)
	{
		NDPIErrorExt(tif->tif_clientdata,module,"Out of memory");
		return(0);
	}
	for (na=m, nb=0; nb<tif->tif_dir.td_samplesperpixel; na++, nb++)
		*na=value;
	o=NDPIWriteDirectoryTagCheckedLongArray(tif,ndir,dir,tag,tif->tif_dir.td_samplesperpixel,m);
	_NDPIfree(m);
	return(o);
}
#endif

#ifdef notdef
static int
NDPIWriteDirectoryTagSlong(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, int32_t value)
{
	if (dir==NULL)
	{
		(*ndir)++;
		return(1);
	}
	return(NDPIWriteDirectoryTagCheckedSlong(tif,ndir,dir,tag,value));
}
#endif

static int
NDPIWriteDirectoryTagSlongArray(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, int32_t* value)
{
	if (dir==NULL)
	{
		(*ndir)++;
		return(1);
	}
	return(NDPIWriteDirectoryTagCheckedSlongArray(tif,ndir,dir,tag,count,value));
}

#if 0
static int
NDPIWriteDirectoryTagSlongPerSample(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, int32_t value)
{
	static const char module[] = "NDPIWriteDirectoryTagSlongPerSample";
	int32_t* m;
	int32_t* na;
	uint16_t nb;
	int o;
	if (dir==NULL)
	{
		(*ndir)++;
		return(1);
	}
	m=_NDPImalloc(tif->tif_dir.td_samplesperpixel*sizeof(int32_t));
	if (m==NULL)
	{
		NDPIErrorExt(tif->tif_clientdata,module,"Out of memory");
		return(0);
	}
	for (na=m, nb=0; nb<tif->tif_dir.td_samplesperpixel; na++, nb++)
		*na=value;
	o=NDPIWriteDirectoryTagCheckedSlongArray(tif,ndir,dir,tag,tif->tif_dir.td_samplesperpixel,m);
	_NDPIfree(m);
	return(o);
}
#endif

#ifdef notdef
static int
NDPIWriteDirectoryTagLong8(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint64_t value)
{
	if (dir==NULL)
	{
		(*ndir)++;
		return(1);
	}
	return(NDPIWriteDirectoryTagCheckedLong8(tif,ndir,dir,tag,value));
}
#endif

static int
NDPIWriteDirectoryTagLong8Array(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, uint64_t* value)
{
	if (dir==NULL)
	{
		(*ndir)++;
		return(1);
	}
	return(NDPIWriteDirectoryTagCheckedLong8Array(tif,ndir,dir,tag,count,value));
}

#ifdef notdef
static int
NDPIWriteDirectoryTagSlong8(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, int64_t value)
{
	if (dir==NULL)
	{
		(*ndir)++;
		return(1);
	}
	return(NDPIWriteDirectoryTagCheckedSlong8(tif,ndir,dir,tag,value));
}
#endif

static int
NDPIWriteDirectoryTagSlong8Array(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, int64_t* value)
{
	if (dir==NULL)
	{
		(*ndir)++;
		return(1);
	}
	return(NDPIWriteDirectoryTagCheckedSlong8Array(tif,ndir,dir,tag,count,value));
}

static int
NDPIWriteDirectoryTagRational(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, double value)
{
	if (dir==NULL)
	{
		(*ndir)++;
		return(1);
	}
	return(NDPIWriteDirectoryTagCheckedRational(tif,ndir,dir,tag,value));
}

static int
NDPIWriteDirectoryTagRationalArray(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, float* value)
{
	if (dir==NULL)
	{
		(*ndir)++;
		return(1);
	}
	return(NDPIWriteDirectoryTagCheckedRationalArray(tif,ndir,dir,tag,count,value));
}

static int
NDPIWriteDirectoryTagSrationalArray(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, float* value)
{
	if (dir==NULL)
	{
		(*ndir)++;
		return(1);
	}
	return(NDPIWriteDirectoryTagCheckedSrationalArray(tif,ndir,dir,tag,count,value));
}

/*-- Rational2Double: additional write functions */
static int
NDPIWriteDirectoryTagRationalDoubleArray(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, double* value)
{
	if (dir==NULL)
	{
		(*ndir)++;
		return(1);
	}
	return(NDPIWriteDirectoryTagCheckedRationalDoubleArray(tif,ndir,dir,tag,count,value));
}

static int
NDPIWriteDirectoryTagSrationalDoubleArray(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, double* value)
{
	if (dir==NULL)
	{
		(*ndir)++;
		return(1);
	}
	return(NDPIWriteDirectoryTagCheckedSrationalDoubleArray(tif,ndir,dir,tag,count,value));
}

#ifdef notdef
static int NDPIWriteDirectoryTagFloat(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, float value)
{
	if (dir==NULL)
	{
		(*ndir)++;
		return(1);
	}
	return(NDPIWriteDirectoryTagCheckedFloat(tif,ndir,dir,tag,value));
}
#endif

static int NDPIWriteDirectoryTagFloatArray(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, float* value)
{
	if (dir==NULL)
	{
		(*ndir)++;
		return(1);
	}
	return(NDPIWriteDirectoryTagCheckedFloatArray(tif,ndir,dir,tag,count,value));
}

#if 0
static int NDPIWriteDirectoryTagFloatPerSample(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, float value)
{
	static const char module[] = "NDPIWriteDirectoryTagFloatPerSample";
	float* m;
	float* na;
	uint16_t nb;
	int o;
	if (dir==NULL)
	{
		(*ndir)++;
		return(1);
	}
	m=_NDPImalloc(tif->tif_dir.td_samplesperpixel*sizeof(float));
	if (m==NULL)
	{
		NDPIErrorExt(tif->tif_clientdata,module,"Out of memory");
		return(0);
	}
	for (na=m, nb=0; nb<tif->tif_dir.td_samplesperpixel; na++, nb++)
		*na=value;
	o=NDPIWriteDirectoryTagCheckedFloatArray(tif,ndir,dir,tag,tif->tif_dir.td_samplesperpixel,m);
	_NDPIfree(m);
	return(o);
}
#endif

#ifdef notdef
static int NDPIWriteDirectoryTagDouble(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, double value)
{
	if (dir==NULL)
	{
		(*ndir)++;
		return(1);
	}
	return(NDPIWriteDirectoryTagCheckedDouble(tif,ndir,dir,tag,value));
}
#endif

static int NDPIWriteDirectoryTagDoubleArray(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, double* value)
{
	if (dir==NULL)
	{
		(*ndir)++;
		return(1);
	}
	return(NDPIWriteDirectoryTagCheckedDoubleArray(tif,ndir,dir,tag,count,value));
}

#if 0
static int NDPIWriteDirectoryTagDoublePerSample(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, double value)
{
	static const char module[] = "NDPIWriteDirectoryTagDoublePerSample";
	double* m;
	double* na;
	uint16_t nb;
	int o;
	if (dir==NULL)
	{
		(*ndir)++;
		return(1);
	}
	m=_NDPImalloc(tif->tif_dir.td_samplesperpixel*sizeof(double));
	if (m==NULL)
	{
		NDPIErrorExt(tif->tif_clientdata,module,"Out of memory");
		return(0);
	}
	for (na=m, nb=0; nb<tif->tif_dir.td_samplesperpixel; na++, nb++)
		*na=value;
	o=NDPIWriteDirectoryTagCheckedDoubleArray(tif,ndir,dir,tag,tif->tif_dir.td_samplesperpixel,m);
	_NDPIfree(m);
	return(o);
}
#endif

static int
NDPIWriteDirectoryTagIfdArray(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, uint32_t* value)
{
	if (dir==NULL)
	{
		(*ndir)++;
		return(1);
	}
	return(NDPIWriteDirectoryTagCheckedIfdArray(tif,ndir,dir,tag,count,value));
}

#ifdef notdef
static int
NDPIWriteDirectoryTagIfd8Array(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, uint64_t* value)
{
	if (dir==NULL)
	{
		(*ndir)++;
		return(1);
	}
	return(NDPIWriteDirectoryTagCheckedIfd8Array(tif,ndir,dir,tag,count,value));
}
#endif

static int
NDPIWriteDirectoryTagShortLong(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t value)
{
	if (dir==NULL)
	{
		(*ndir)++;
		return(1);
	}
	if (value<=0xFFFF)
		return(NDPIWriteDirectoryTagCheckedShort(tif,ndir,dir,tag,(uint16_t)value));
	else
		return(NDPIWriteDirectoryTagCheckedLong(tif,ndir,dir,tag,value));
}

static int _WriteAsType(TIFF* tif, uint64_t strile_size, uint64_t uncompressed_threshold)
{
    const uint16_t compression = tif->tif_dir.td_compression;
    if ( compression == COMPRESSION_NONE )
    {
        return strile_size > uncompressed_threshold;
    }
    else if ( compression == COMPRESSION_JPEG ||
              compression == COMPRESSION_LZW ||
              compression == COMPRESSION_ADOBE_DEFLATE ||
              compression == COMPRESSION_LZMA ||
              compression == COMPRESSION_LERC ||
              compression == COMPRESSION_ZSTD ||
              compression == COMPRESSION_WEBP )
    {
        /* For a few select compression types, we assume that in the worst */
        /* case the compressed size will be 10 times the uncompressed size */
        /* This is overly pessismistic ! */
        return strile_size >= uncompressed_threshold / 10;
    }
    return 1;
}

static int WriteAsLong8(TIFF* tif, uint64_t strile_size)
{
    return _WriteAsType(tif, strile_size, 0xFFFFFFFFU);
}

static int WriteAsLong4(TIFF* tif, uint64_t strile_size)
{
    return _WriteAsType(tif, strile_size, 0xFFFFU);
}

/************************************************************************/
/*                NDPIWriteDirectoryTagLongLong8Array()                 */
/*                                                                      */
/*      Write out LONG8 array and write a SHORT/LONG/LONG8 depending    */
/*      on strile size and Classic/BigTIFF mode.                        */
/************************************************************************/

static int
NDPIWriteDirectoryTagLongLong8Array(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, uint64_t* value)
{
    static const char module[] = "NDPIWriteDirectoryTagLongLong8Array";
    int o;
    int write_aslong4;

    /* is this just a counting pass? */
    if (dir==NULL)
    {
        (*ndir)++;
        return(1);
    }

    if( tif->tif_dir.td_deferstrilearraywriting )
    {
        return NDPIWriteDirectoryTagData(tif, ndir, dir, tag, TIFF_NOTYPE, 0, 0, NULL);
    }

    if( tif->tif_flags&TIFF_BIGTIFF )
    {
        int write_aslong8 = 1;
        /* In the case of ByteCounts array, we may be able to write them on */
        /* LONG if the strip/tilesize is not too big. */
        /* Also do that for count > 1 in the case someone would want to create */
        /* a single-strip file with a growing height, in which case using */
        /* LONG8 will be safer. */
        if( count > 1 && tag == TIFFTAG_STRIPBYTECOUNTS )
        {
            write_aslong8 = WriteAsLong8(tif, NDPIStripSize64(tif));
        }
        else if( count > 1 && tag == TIFFTAG_TILEBYTECOUNTS )
        {
            write_aslong8 = WriteAsLong8(tif, NDPITileSize64(tif));
        }
        if( write_aslong8 )
        {
            return NDPIWriteDirectoryTagCheckedLong8Array(tif,ndir,dir,
                                                        tag,count,value);
        }
    }

    write_aslong4 = 1;
    if( count > 1 && tag == TIFFTAG_STRIPBYTECOUNTS )
    {
        write_aslong4 = WriteAsLong4(tif, NDPIStripSize64(tif));
    }
    else if( count > 1 && tag == TIFFTAG_TILEBYTECOUNTS )
    {
        write_aslong4 = WriteAsLong4(tif, NDPITileSize64(tif));
    }
    if( write_aslong4 )
    {
        /*
        ** For classic tiff we want to verify everything is in range for LONG
        ** and convert to long format.
        */

        uint32_t* p = _NDPImalloc(count * sizeof(uint32_t));
        uint32_t* q;
        uint64_t* ma;
        uint32_t mb;

        if (p==NULL)
        {
            NDPIErrorExt(tif->tif_clientdata,module,"Out of memory");
            return(0);
        }

        for (q=p, ma=value, mb=0; mb<count; ma++, mb++, q++)
        {
            if (*ma>0xFFFFFFFF)
            {
                NDPIErrorExt(tif->tif_clientdata,module,
                            "Attempt to write value larger than 0xFFFFFFFF in LONG array.");
                _NDPIfree(p);
                return(0);
            }
            *q= (uint32_t)(*ma);
        }

        o=NDPIWriteDirectoryTagCheckedLongArray(tif,ndir,dir,tag,count,p);
        _NDPIfree(p);
    }
    else
    {
        uint16_t* p = _NDPImalloc(count * sizeof(uint16_t));
        uint16_t* q;
        uint64_t* ma;
        uint32_t mb;

        if (p==NULL)
        {
            NDPIErrorExt(tif->tif_clientdata,module,"Out of memory");
            return(0);
        }

        for (q=p, ma=value, mb=0; mb<count; ma++, mb++, q++)
        {
            if (*ma>0xFFFF)
            {
                /* Should not happen normally given the check we did before */
                NDPIErrorExt(tif->tif_clientdata,module,
                            "Attempt to write value larger than 0xFFFF in SHORT array.");
                _NDPIfree(p);
                return(0);
            }
            *q= (uint16_t)(*ma);
        }

        o=NDPIWriteDirectoryTagCheckedShortArray(tif,ndir,dir,tag,count,p);
        _NDPIfree(p);
    }

    return(o);
}

/************************************************************************/
/*                 NDPIWriteDirectoryTagIfdIfd8Array()                  */
/*                                                                      */
/*      Write either IFD8 or IFD array depending on file type.          */
/************************************************************************/

static int
NDPIWriteDirectoryTagIfdIfd8Array(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, uint64_t* value)
{
    static const char module[] = "NDPIWriteDirectoryTagIfdIfd8Array";
    uint64_t* ma;
    uint32_t mb;
    uint32_t* p;
    uint32_t* q;
    int o;

    /* is this just a counting pass? */
    if (dir==NULL)
    {
        (*ndir)++;
        return(1);
    }

    /* We always write IFD8 for BigTIFF, no checking needed. */
    if( tif->tif_flags&TIFF_BIGTIFF )
        return NDPIWriteDirectoryTagCheckedIfd8Array(tif,ndir,dir,
                                                     tag,count,value);

    /*
    ** For classic tiff we want to verify everything is in range for IFD
    ** and convert to long format.
    */

    p = _NDPImalloc(count*sizeof(uint32_t));
    if (p==NULL)
    {
        NDPIErrorExt(tif->tif_clientdata,module,"Out of memory");
        return(0);
    }

    for (q=p, ma=value, mb=0; mb<count; ma++, mb++, q++)
    {
        if (*ma>0xFFFFFFFF)
        {
            NDPIErrorExt(tif->tif_clientdata,module,
                         "Attempt to write value larger than 0xFFFFFFFF in Classic TIFF file.");
            _NDPIfree(p);
            return(0);
        }
        *q= (uint32_t)(*ma);
    }

    o=NDPIWriteDirectoryTagCheckedIfdArray(tif,ndir,dir,tag,count,p);
    _NDPIfree(p);

    return(o);
}

#ifdef notdef
static int
NDPIWriteDirectoryTagShortLongLong8Array(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, uint64_t* value)
{
	static const char module[] = "NDPIWriteDirectoryTagShortLongLong8Array";
	uint64_t* ma;
	uint32_t mb;
	uint8_t n;
	int o;
	if (dir==NULL)
	{
		(*ndir)++;
		return(1);
	}
	n=0;
	for (ma=value, mb=0; mb<count; ma++, mb++)
	{
		if ((n==0)&&(*ma>0xFFFF))
			n=1;
		if ((n==1)&&(*ma>0xFFFFFFFF))
		{
			n=2;
			break;
		}
	}
	if (n==0)
	{
		uint16_t* p;
		uint16_t* q;
		p=_NDPImalloc(count*sizeof(uint16_t));
		if (p==NULL)
		{
			NDPIErrorExt(tif->tif_clientdata,module,"Out of memory");
			return(0);
		}
		for (ma=value, mb=0, q=p; mb<count; ma++, mb++, q++)
			*q=(uint16_t)(*ma);
		o=NDPIWriteDirectoryTagCheckedShortArray(tif,ndir,dir,tag,count,p);
		_NDPIfree(p);
	}
	else if (n==1)
	{
		uint32_t* p;
		uint32_t* q;
		p=_NDPImalloc(count*sizeof(uint32_t));
		if (p==NULL)
		{
			NDPIErrorExt(tif->tif_clientdata,module,"Out of memory");
			return(0);
		}
		for (ma=value, mb=0, q=p; mb<count; ma++, mb++, q++)
			*q=(uint32_t)(*ma);
		o=NDPIWriteDirectoryTagCheckedLongArray(tif,ndir,dir,tag,count,p);
		_NDPIfree(p);
	}
	else
	{
		assert(n==2);
		o=NDPIWriteDirectoryTagCheckedLong8Array(tif,ndir,dir,tag,count,value);
	}
	return(o);
}
#endif
static int
NDPIWriteDirectoryTagColormap(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir)
{
	static const char module[] = "NDPIWriteDirectoryTagColormap";
	uint32_t m;
	uint16_t* n;
	int o;
	if (dir==NULL)
	{
		(*ndir)++;
		return(1);
	}
	m=(1<<tif->tif_dir.td_bitspersample);
	n=_NDPImalloc(3*m*sizeof(uint16_t));
	if (n==NULL)
	{
		NDPIErrorExt(tif->tif_clientdata,module,"Out of memory");
		return(0);
	}
	_NDPImemcpy(&n[0],tif->tif_dir.td_colormap[0],m*sizeof(uint16_t));
	_NDPImemcpy(&n[m],tif->tif_dir.td_colormap[1],m*sizeof(uint16_t));
	_NDPImemcpy(&n[2*m],tif->tif_dir.td_colormap[2],m*sizeof(uint16_t));
	o=NDPIWriteDirectoryTagCheckedShortArray(tif,ndir,dir,TIFFTAG_COLORMAP,3*m,n);
	_NDPIfree(n);
	return(o);
}

static int
NDPIWriteDirectoryTagTransferfunction(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir)
{
	static const char module[] = "NDPIWriteDirectoryTagTransferfunction";
	uint32_t m;
	uint16_t n;
	uint16_t* o;
	int p;
	if (dir==NULL)
	{
		(*ndir)++;
		return(1);
	}
	m=(1<<tif->tif_dir.td_bitspersample);
	n=tif->tif_dir.td_samplesperpixel-tif->tif_dir.td_extrasamples;
	/*
	 * Check if the table can be written as a single column,
	 * or if it must be written as 3 columns.  Note that we
	 * write a 3-column tag if there are 2 samples/pixel and
	 * a single column of data won't suffice--hmm.
	 */
	if (n>3)
		n=3;
	if (n==3)
	{
		if (tif->tif_dir.td_transferfunction[2] == NULL ||
		    !_TIFFmemcmp(tif->tif_dir.td_transferfunction[0],tif->tif_dir.td_transferfunction[2],m*sizeof(uint16_t)))
			n=2;
	}
	if (n==2)
	{
		if (tif->tif_dir.td_transferfunction[1] == NULL ||
		    !_TIFFmemcmp(tif->tif_dir.td_transferfunction[0],tif->tif_dir.td_transferfunction[1],m*sizeof(uint16_t)))
			n=1;
	}
	if (n==0)
		n=1;
	o=_NDPImalloc(n*m*sizeof(uint16_t));
	if (o==NULL)
	{
		NDPIErrorExt(tif->tif_clientdata,module,"Out of memory");
		return(0);
	}
	_NDPImemcpy(&o[0],tif->tif_dir.td_transferfunction[0],m*sizeof(uint16_t));
	if (n>1)
		_NDPImemcpy(&o[m],tif->tif_dir.td_transferfunction[1],m*sizeof(uint16_t));
	if (n>2)
		_NDPImemcpy(&o[2*m],tif->tif_dir.td_transferfunction[2],m*sizeof(uint16_t));
	p=NDPIWriteDirectoryTagCheckedShortArray(tif,ndir,dir,TIFFTAG_TRANSFERFUNCTION,n*m,o);
	_NDPIfree(o);
	return(p);
}

static int
NDPIWriteDirectoryTagSubifd(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir)
{
	static const char module[] = "NDPIWriteDirectoryTagSubifd";
	uint64_t m;
	int n;
	if (tif->tif_dir.td_nsubifd==0)
		return(1);
	if (dir==NULL)
	{
		(*ndir)++;
		return(1);
	}
	m=tif->tif_dataoff;
	if (!(tif->tif_flags&TIFF_BIGTIFF))
	{
		uint32_t* o;
		uint64_t* pa;
		uint32_t* pb;
		uint16_t p;
		o=_NDPImalloc(tif->tif_dir.td_nsubifd*sizeof(uint32_t));
		if (o==NULL)
		{
			NDPIErrorExt(tif->tif_clientdata,module,"Out of memory");
			return(0);
		}
		pa=tif->tif_dir.td_subifd;
		pb=o;
		for (p=0; p < tif->tif_dir.td_nsubifd; p++)
		{
                        assert(pa != 0);

                        /* Could happen if an classicTIFF has a SubIFD of type LONG8 (which is illegal) */
                        if( *pa > 0xFFFFFFFFUL)
                        {
                            NDPIErrorExt(tif->tif_clientdata,module,"Illegal value for SubIFD tag");
                            _NDPIfree(o);
                            return(0);
                        }
			*pb++=(uint32_t)(*pa++);
		}
		n=NDPIWriteDirectoryTagCheckedIfdArray(tif,ndir,dir,TIFFTAG_SUBIFD,tif->tif_dir.td_nsubifd,o);
		_NDPIfree(o);
	}
	else
		n=NDPIWriteDirectoryTagCheckedIfd8Array(tif,ndir,dir,TIFFTAG_SUBIFD,tif->tif_dir.td_nsubifd,tif->tif_dir.td_subifd);
	if (!n)
		return(0);
	/*
	 * Total hack: if this directory includes a SubIFD
	 * tag then force the next <n> directories to be
	 * written as ``sub directories'' of this one.  This
	 * is used to write things like thumbnails and
	 * image masks that one wants to keep out of the
	 * normal directory linkage access mechanism.
	 */
	tif->tif_flags|=TIFF_INSUBIFD;
	tif->tif_nsubifd=tif->tif_dir.td_nsubifd;
	if (tif->tif_dir.td_nsubifd==1)
		tif->tif_subifdoff=0;
	else
		tif->tif_subifdoff=m;
	return(1);
}

static int
NDPIWriteDirectoryTagCheckedAscii(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, char* value)
{
	assert(sizeof(char)==1);
	return(NDPIWriteDirectoryTagData(tif,ndir,dir,tag,TIFF_ASCII,count,count,value));
}

static int
NDPIWriteDirectoryTagCheckedUndefinedArray(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, uint8_t* value)
{
	assert(sizeof(uint8_t) == 1);
	return(NDPIWriteDirectoryTagData(tif,ndir,dir,tag,TIFF_UNDEFINED,count,count,value));
}

#ifdef notdef
static int
NDPIWriteDirectoryTagCheckedByte(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint8_t value)
{
	assert(sizeof(uint8_t)==1);
	return(NDPIWriteDirectoryTagData(tif,ndir,dir,tag,TIFF_BYTE,1,1,&value));
}
#endif

static int
NDPIWriteDirectoryTagCheckedByteArray(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, uint8_t* value)
{
	assert(sizeof(uint8_t) == 1);
	return(NDPIWriteDirectoryTagData(tif,ndir,dir,tag,TIFF_BYTE,count,count,value));
}

#ifdef notdef
static int
NDPIWriteDirectoryTagCheckedSbyte(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, int8_t value)
{
	assert(sizeof(int8_t)==1);
	return(NDPIWriteDirectoryTagData(tif,ndir,dir,tag,TIFF_SBYTE,1,1,&value));
}
#endif

static int
NDPIWriteDirectoryTagCheckedSbyteArray(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, int8_t* value)
{
	assert(sizeof(int8_t) == 1);
	return(NDPIWriteDirectoryTagData(tif,ndir,dir,tag,TIFF_SBYTE,count,count,value));
}

static int
NDPIWriteDirectoryTagCheckedShort(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint16_t value)
{
	uint16_t m;
	assert(sizeof(uint16_t) == 2);
	m=value;
	if (tif->tif_flags&TIFF_SWAB)
		NDPISwabShort(&m);
	return(NDPIWriteDirectoryTagData(tif,ndir,dir,tag,TIFF_SHORT,1,2,&m));
}

static int
NDPIWriteDirectoryTagCheckedShortArray(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, uint16_t* value)
{
	assert(count<0x80000000);
	assert(sizeof(uint16_t) == 2);
	if (tif->tif_flags&TIFF_SWAB)
		NDPISwabArrayOfShort(value,count);
	return(NDPIWriteDirectoryTagData(tif,ndir,dir,tag,TIFF_SHORT,count,count*2,value));
}

#ifdef notdef
static int
NDPIWriteDirectoryTagCheckedSshort(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, int16_t value)
{
	int16_t m;
	assert(sizeof(int16_t)==2);
	m=value;
	if (tif->tif_flags&TIFF_SWAB)
		NDPISwabShort((uint16_t*)(&m));
	return(NDPIWriteDirectoryTagData(tif,ndir,dir,tag,TIFF_SSHORT,1,2,&m));
}
#endif

static int
NDPIWriteDirectoryTagCheckedSshortArray(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, int16_t* value)
{
	assert(count<0x80000000);
	assert(sizeof(int16_t) == 2);
	if (tif->tif_flags&TIFF_SWAB)
		NDPISwabArrayOfShort((uint16_t*)value, count);
	return(NDPIWriteDirectoryTagData(tif,ndir,dir,tag,TIFF_SSHORT,count,count*2,value));
}

static int
NDPIWriteDirectoryTagCheckedLong(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t value)
{
	uint32_t m;
	assert(sizeof(uint32_t) == 4);
	m=value;
	if (tif->tif_flags&TIFF_SWAB)
		NDPISwabLong(&m);
	return(NDPIWriteDirectoryTagData(tif,ndir,dir,tag,TIFF_LONG,1,4,&m));
}

static int
NDPIWriteDirectoryTagCheckedLongArray(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, uint32_t* value)
{
	assert(count<0x40000000);
	assert(sizeof(uint32_t) == 4);
	if (tif->tif_flags&TIFF_SWAB)
		NDPISwabArrayOfLong(value,count);
	return(NDPIWriteDirectoryTagData(tif,ndir,dir,tag,TIFF_LONG,count,count*4,value));
}

#ifdef notdef
static int
NDPIWriteDirectoryTagCheckedSlong(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, int32_t value)
{
	int32_t m;
	assert(sizeof(int32_t)==4);
	m=value;
	if (tif->tif_flags&TIFF_SWAB)
		NDPISwabLong((uint32_t*)(&m));
	return(NDPIWriteDirectoryTagData(tif,ndir,dir,tag,TIFF_SLONG,1,4,&m));
}
#endif

static int
NDPIWriteDirectoryTagCheckedSlongArray(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, int32_t* value)
{
	assert(count<0x40000000);
	assert(sizeof(int32_t) == 4);
	if (tif->tif_flags&TIFF_SWAB)
		NDPISwabArrayOfLong((uint32_t*)value, count);
	return(NDPIWriteDirectoryTagData(tif,ndir,dir,tag,TIFF_SLONG,count,count*4,value));
}

#ifdef notdef
static int
NDPIWriteDirectoryTagCheckedLong8(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint64_t value)
{
	uint64_t m;
	assert(sizeof(uint64_t)==8);
	if( !(tif->tif_flags&TIFF_BIGTIFF) ) {
		NDPIErrorExt(tif->tif_clientdata,"NDPIWriteDirectoryTagCheckedLong8","LONG8 not allowed for ClassicTIFF");
		return(0);
	}
	m=value;
	if (tif->tif_flags&TIFF_SWAB)
		NDPISwabLong8(&m);
	return(NDPIWriteDirectoryTagData(tif,ndir,dir,tag,TIFF_LONG8,1,8,&m));
}
#endif

static int
NDPIWriteDirectoryTagCheckedLong8Array(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, uint64_t* value)
{
	assert(count<0x20000000);
	assert(sizeof(uint64_t) == 8);
	if( !(tif->tif_flags&TIFF_BIGTIFF) ) {
		NDPIErrorExt(tif->tif_clientdata,"NDPIWriteDirectoryTagCheckedLong8Array","LONG8 not allowed for ClassicTIFF");
		return(0);
	}
	if (tif->tif_flags&TIFF_SWAB)
		TIFFSwabArrayOfLong8(value,count);
	return(NDPIWriteDirectoryTagData(tif,ndir,dir,tag,TIFF_LONG8,count,count*8,value));
}

#ifdef notdef
static int
NDPIWriteDirectoryTagCheckedSlong8(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, int64_t value)
{
	int64_t m;
	assert(sizeof(int64_t)==8);
	if( !(tif->tif_flags&TIFF_BIGTIFF) ) {
		NDPIErrorExt(tif->tif_clientdata,"NDPIWriteDirectoryTagCheckedSlong8","SLONG8 not allowed for ClassicTIFF");
		return(0);
	}
	m=value;
	if (tif->tif_flags&TIFF_SWAB)
		NDPISwabLong8((uint64_t*)(&m));
	return(NDPIWriteDirectoryTagData(tif,ndir,dir,tag,TIFF_SLONG8,1,8,&m));
}
#endif

static int
NDPIWriteDirectoryTagCheckedSlong8Array(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, int64_t* value)
{
	assert(count<0x20000000);
	assert(sizeof(int64_t) == 8);
	if( !(tif->tif_flags&TIFF_BIGTIFF) ) {
		NDPIErrorExt(tif->tif_clientdata,"NDPIWriteDirectoryTagCheckedSlong8Array","SLONG8 not allowed for ClassicTIFF");
		return(0);
	}
	if (tif->tif_flags&TIFF_SWAB)
		TIFFSwabArrayOfLong8((uint64_t*)value, count);
	return(NDPIWriteDirectoryTagData(tif,ndir,dir,tag,TIFF_SLONG8,count,count*8,value));
}

static int
NDPIWriteDirectoryTagCheckedRational(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, double value)
{
	static const char module[] = "NDPIWriteDirectoryTagCheckedRational";
	uint32_t m[2];
	assert(sizeof(uint32_t) == 4);
	if (value < 0) 
	{
		NDPIErrorExt(tif->tif_clientdata, module, "Negative value is illegal");
		return 0;
	} 
	else if (value != value) 
	{
		NDPIErrorExt(tif->tif_clientdata, module, "Not-a-number value is illegal");
		return 0;
	}
#ifdef not_def
	else if (value==0.0)
	{
		m[0]=0;
		m[1]=1;
	}
	else if (value <= 0xFFFFFFFFU && value==(double)(uint32_t)value)
	{
		m[0]=(uint32_t)value;
		m[1]=1;
	}
	else if (value<1.0)
	{
		m[0]=(uint32_t)(value*0xFFFFFFFF);
		m[1]=0xFFFFFFFF;
	}
	else
	{
		m[0]=0xFFFFFFFF;
		m[1]=(uint32_t)(0xFFFFFFFF/value);
	}
#else
	/*--Rational2Double: New function also used for non-custom rational tags. 
	 *  However, could be omitted here, because NDPIWriteDirectoryTagCheckedRational() is not used by code for custom tags,
	 *  only by code for named-tiff-tags like FIELD_RESOLUTION and FIELD_POSITION */
	else {
	DoubleToRational(value, &m[0], &m[1]);
	}
#endif

	if (tif->tif_flags&TIFF_SWAB)
	{
		NDPISwabLong(&m[0]);
		NDPISwabLong(&m[1]);
	}
	return(NDPIWriteDirectoryTagData(tif,ndir,dir,tag,TIFF_RATIONAL,1,8,&m[0]));
}

static int
NDPIWriteDirectoryTagCheckedRationalArray(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, float* value)
{
	static const char module[] = "NDPIWriteDirectoryTagCheckedRationalArray";
	uint32_t* m;
	float* na;
	uint32_t* nb;
	uint32_t nc;
	int o;
	assert(sizeof(uint32_t) == 4);
	m=_NDPImalloc(count*2*sizeof(uint32_t));
	if (m==NULL)
	{
		NDPIErrorExt(tif->tif_clientdata,module,"Out of memory");
		return(0);
	}
	for (na=value, nb=m, nc=0; nc<count; na++, nb+=2, nc++)
	{
#ifdef not_def
		if (*na<=0.0 || *na != *na)
		{
			nb[0]=0;
			nb[1]=1;
		}
		else if (*na >= 0 && *na <= (float)0xFFFFFFFFU &&
                         *na==(float)(uint32_t)(*na))
		{
			nb[0]=(uint32_t)(*na);
			nb[1]=1;
		}
		else if (*na<1.0)
		{
			nb[0]=(uint32_t)((double)(*na)*0xFFFFFFFF);
			nb[1]=0xFFFFFFFF;
		}
		else
		{
			nb[0]=0xFFFFFFFF;
			nb[1]=(uint32_t)((double)0xFFFFFFFF/(*na));
		}
#else
		/*-- Rational2Double: Also for float precision accuracy is sometimes enhanced --*/
		DoubleToRational(*na, &nb[0], &nb[1]);
#endif
	}
	if (tif->tif_flags&TIFF_SWAB)
		NDPISwabArrayOfLong(m,count*2);
	o=NDPIWriteDirectoryTagData(tif,ndir,dir,tag,TIFF_RATIONAL,count,count*8,&m[0]);
	_NDPIfree(m);
	return(o);
}

static int
NDPIWriteDirectoryTagCheckedSrationalArray(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, float* value)
{
	static const char module[] = "NDPIWriteDirectoryTagCheckedSrationalArray";
	int32_t* m;
	float* na;
	int32_t* nb;
	uint32_t nc;
	int o;
	assert(sizeof(int32_t) == 4);
	m=_NDPImalloc(count*2*sizeof(int32_t));
	if (m==NULL)
	{
		NDPIErrorExt(tif->tif_clientdata,module,"Out of memory");
		return(0);
	}
	for (na=value, nb=m, nc=0; nc<count; na++, nb+=2, nc++)
	{
#ifdef not_def
		if (*na<0.0)
		{
			if (*na==(int32_t)(*na))
			{
				nb[0]=(int32_t)(*na);
				nb[1]=1;
			}
			else if (*na>-1.0)
			{
				nb[0]=-(int32_t)((double)(-*na)*0x7FFFFFFF);
				nb[1]=0x7FFFFFFF;
			}
			else
			{
				nb[0]=-0x7FFFFFFF;
				nb[1]=(int32_t)((double)0x7FFFFFFF/(-*na));
			}
		}
		else
		{
			if (*na==(int32_t)(*na))
			{
				nb[0]=(int32_t)(*na);
				nb[1]=1;
			}
			else if (*na<1.0)
			{
				nb[0]=(int32_t)((double)(*na)*0x7FFFFFFF);
				nb[1]=0x7FFFFFFF;
			}
			else
			{
				nb[0]=0x7FFFFFFF;
				nb[1]=(int32_t)((double)0x7FFFFFFF/(*na));
			}
		}
#else
		/*-- Rational2Double: Also for float precision accuracy is sometimes enhanced --*/
		DoubleToSrational(*na, &nb[0], &nb[1]);
#endif
	}
	if (tif->tif_flags&TIFF_SWAB)
		NDPISwabArrayOfLong((uint32_t*)m, count * 2);
	o=NDPIWriteDirectoryTagData(tif,ndir,dir,tag,TIFF_SRATIONAL,count,count*8,&m[0]);
	_NDPIfree(m);
	return(o);
}

/*-- Rational2Double: additional write functions for double arrays */
static int
NDPIWriteDirectoryTagCheckedRationalDoubleArray(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, double* value)
{
	static const char module[] = "NDPIWriteDirectoryTagCheckedRationalDoubleArray";
	uint32_t* m;
	double* na;
	uint32_t* nb;
	uint32_t nc;
	int o;
	assert(sizeof(uint32_t) == 4);
	m=_NDPImalloc(count*2*sizeof(uint32_t));
	if (m==NULL)
	{
		NDPIErrorExt(tif->tif_clientdata,module,"Out of memory");
		return(0);
	}
	for (na=value, nb=m, nc=0; nc<count; na++, nb+=2, nc++)
	{
		DoubleToRational(*na, &nb[0], &nb[1]);
	}
	if (tif->tif_flags&TIFF_SWAB)
		NDPISwabArrayOfLong(m,count*2);
	o=NDPIWriteDirectoryTagData(tif,ndir,dir,tag,TIFF_RATIONAL,count,count*8,&m[0]);
	_NDPIfree(m);
	return(o);
} /*-- NDPIWriteDirectoryTagCheckedRationalDoubleArray() ------- */

static int
NDPIWriteDirectoryTagCheckedSrationalDoubleArray(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, double* value)
{
	static const char module[] = "NDPIWriteDirectoryTagCheckedSrationalDoubleArray";
	int32_t* m;
	double* na;
	int32_t* nb;
	uint32_t nc;
	int o;
	assert(sizeof(int32_t) == 4);
	m=_NDPImalloc(count*2*sizeof(int32_t));
	if (m==NULL)
	{
		NDPIErrorExt(tif->tif_clientdata,module,"Out of memory");
		return(0);
	}
	for (na=value, nb=m, nc=0; nc<count; na++, nb+=2, nc++)
	{
		DoubleToSrational(*na, &nb[0], &nb[1]);
	}
	if (tif->tif_flags&TIFF_SWAB)
		NDPISwabArrayOfLong((uint32_t*)m, count * 2);
	o=NDPIWriteDirectoryTagData(tif,ndir,dir,tag,TIFF_SRATIONAL,count,count*8,&m[0]);
	_NDPIfree(m);
	return(o);
} /*--- NDPIWriteDirectoryTagCheckedSrationalDoubleArray() -------- */

#if 0
static
void DoubleToRational_direct(double value, unsigned long *num, unsigned long *denom)
{
	/*--- OLD Code for debugging and comparison  ---- */
	/* code merged from NDPIWriteDirectoryTagCheckedRationalArray() and NDPIWriteDirectoryTagCheckedRational() */

	/* First check for zero and also check for negative numbers (which are illegal for RATIONAL) 
	 * and also check for "not-a-number". In each case just set this to zero to support also rational-arrays.
	  */
	if (value<=0.0 || value != value)
	{
		*num=0;
		*denom=1;
	}
	else if (value <= 0xFFFFFFFFU &&  (value==(double)(uint32_t)(value)))	/* check for integer values */
	{
		*num=(uint32_t)(value);
		*denom=1;
	}
	else if (value<1.0)
	{
		*num = (uint32_t)((value) * (double)0xFFFFFFFFU);
		*denom=0xFFFFFFFFU;
	}
	else
	{
		*num=0xFFFFFFFFU;
		*denom=(uint32_t)((double)0xFFFFFFFFU/(value));
	}
}  /*-- DoubleToRational_direct() -------------- */
#endif

#if 0
static
void DoubleToSrational_direct(double value,  long *num,  long *denom)
{
	/*--- OLD Code for debugging and comparison -- SIGNED-version ----*/
	/*  code was amended from original NDPIWriteDirectoryTagCheckedSrationalArray() */

	/* First check for zero and also check for negative numbers (which are illegal for RATIONAL)
	 * and also check for "not-a-number". In each case just set this to zero to support also rational-arrays.
	  */
	if (value<0.0)
		{
			if (value==(int32_t)(value))
			{
				*num=(int32_t)(value);
				*denom=1;
			}
			else if (value>-1.0)
			{
				*num=-(int32_t)((-value) * (double)0x7FFFFFFF);
				*denom=0x7FFFFFFF;
			}
			else
			{
				*num=-0x7FFFFFFF;
				*denom=(int32_t)((double)0x7FFFFFFF / (-value));
			}
		}
		else
		{
			if (value==(int32_t)(value))
			{
				*num=(int32_t)(value);
				*denom=1;
			}
			else if (value<1.0)
			{
				*num=(int32_t)((value)  *(double)0x7FFFFFFF);
				*denom=0x7FFFFFFF;
			}
			else
			{
				*num=0x7FFFFFFF;
				*denom=(int32_t)((double)0x7FFFFFFF / (value));
			}
		}
}  /*-- DoubleToSrational_direct() --------------*/
#endif

//#define DOUBLE2RAT_DEBUGOUTPUT
/** -----  Rational2Double: Double To Rational Conversion ----------------------------------------------------------
* There is a mathematical theorem to convert real numbers into a rational (integer fraction) number.
* This is called "continuous fraction" which uses the Euclidean algorithm to find the greatest common divisor (GCD).
*  (ref. e.g. https://de.wikipedia.org/wiki/Kettenbruch or https://en.wikipedia.org/wiki/Continued_fraction
*             https://en.wikipedia.org/wiki/Euclidean_algorithm)
* The following functions implement the
* - ToRationalEuclideanGCD()		auxiliary function which mainly implements euclidean GCD
* - DoubleToRational()			conversion function for un-signed rationals
* - DoubleToSrational()			conversion function for signed rationals
------------------------------------------------------------------------------------------------------------------*/

/**---- ToRationalEuclideanGCD() -----------------------------------------
* Calculates the rational fractional of a double input value
* using the Euclidean algorithm to find the greatest common divisor (GCD)
------------------------------------------------------------------------*/
static
void ToRationalEuclideanGCD(double value, int blnUseSignedRange, int blnUseSmallRange, uint64_t *ullNum, uint64_t *ullDenom)
{
	/* Internally, the integer variables can be bigger than the external ones,
	* as long as the result will fit into the external variable size.
	*/
	uint64_t numSum[3] = { 0, 1, 0 }, denomSum[3] = { 1, 0, 0 };
	uint64_t aux, bigNum, bigDenom;
	uint64_t returnLimit;
	int i;
	uint64_t nMax;
	double fMax;
	unsigned long maxDenom;
	/*-- nMax and fMax defines the initial accuracy of the starting fractional,
	*   or better, the highest used integer numbers used within the starting fractional (bigNum/bigDenom).
	*   There are two approaches, which can accidentally lead to different accuracies just depending on the value.
	*   Therefore, blnUseSmallRange steers this behavior.
	*   For long long nMax = ((9223372036854775807-1)/2); for long nMax = ((2147483647-1)/2);
	*/
	if (blnUseSmallRange) {
		nMax = (uint64_t)((2147483647 - 1) / 2); /* for ULONG range */
	}
	else {
		nMax = ((9223372036854775807 - 1) / 2);				/* for ULLONG range */
	}
	fMax = (double)nMax;

	/*-- For the Euclidean GCD define the denominator range, so that it stays within size of unsigned long variables.
	*   maxDenom should be LONG_MAX for negative values and ULONG_MAX for positive ones.
	*   Also the final returned value of ullNum and ullDenom is limited according to signed- or unsigned-range.
	*/
	if (blnUseSignedRange) {
		maxDenom = 2147483647UL;  /*LONG_MAX = 0x7FFFFFFFUL*/
		returnLimit = maxDenom;
	}
	else {
		maxDenom = 0xFFFFFFFFUL;  /*ULONG_MAX = 0xFFFFFFFFUL*/
		returnLimit = maxDenom;
	}

	/*-- First generate a rational fraction (bigNum/bigDenom) which represents the value
	*   as a rational number with the highest accuracy. Therefore, uint64_t (uint64_t) is needed.
	*   This rational fraction is then reduced using the Euclidean algorithm to find the greatest common divisor (GCD).
	*   bigNum   = big numinator of value without fraction (or cut residual fraction)
	*   bigDenom = big denominator of value
	*-- Break-criteria so that uint64_t cast to "bigNum" introduces no error and bigDenom has no overflow,
	*   and stop with enlargement of fraction when the double-value of it reaches an integer number without fractional part.
	*/
	bigDenom = 1;
	while ((value != floor(value)) && (value < fMax) && (bigDenom < nMax)) {
		bigDenom <<= 1;
		value *= 2;
	}
	bigNum = (uint64_t)value;

	/*-- Start Euclidean algorithm to find the greatest common divisor (GCD) -- */
#define MAX_ITERATIONS 64
	for (i = 0; i < MAX_ITERATIONS; i++) {
		uint64_t val;
		/* if bigDenom is not zero, calculate integer part of fraction. */
		if (bigDenom == 0) {
			break;
		}
		val = bigNum / bigDenom;

		/* Set bigDenom to reminder of bigNum/bigDenom and bigNum to previous denominator bigDenom. */
		aux = bigNum;
		bigNum = bigDenom;
		bigDenom = aux % bigDenom;

		/* calculate next denominator and check for its given maximum */
		aux = val;
		if (denomSum[1] * val + denomSum[0] >= maxDenom) {
			aux = (maxDenom - denomSum[0]) / denomSum[1];
			if (aux * 2 >= val || denomSum[1] >= maxDenom)
				i = (MAX_ITERATIONS + 1);			/* exit but execute rest of for-loop */
			else
				break;
		}
		/* calculate next numerator to numSum2 and save previous one to numSum0; numSum1 just copy of numSum2. */
		numSum[2] = aux * numSum[1] + numSum[0];
		numSum[0] = numSum[1];
		numSum[1] = numSum[2];
		/* calculate next denominator to denomSum2 and save previous one to denomSum0; denomSum1 just copy of denomSum2. */
		denomSum[2] = aux * denomSum[1] + denomSum[0];
		denomSum[0] = denomSum[1];
		denomSum[1] = denomSum[2];
	}

	/*-- Check and adapt for final variable size and return values; reduces internal accuracy; denominator is kept in ULONG-range with maxDenom -- */
	while (numSum[1] > returnLimit || denomSum[1] > returnLimit) {
		numSum[1] = numSum[1] / 2;
		denomSum[1] = denomSum[1] / 2;
	}

	/* return values */
	*ullNum = numSum[1];
	*ullDenom = denomSum[1];

}  /*-- ToRationalEuclideanGCD() -------------- */


/**---- DoubleToRational() -----------------------------------------------
* Calculates the rational fractional of a double input value
* for UN-SIGNED rationals,
* using the Euclidean algorithm to find the greatest common divisor (GCD)
------------------------------------------------------------------------*/
static
void DoubleToRational(double value, uint32_t *num, uint32_t *denom)
{
	/*---- UN-SIGNED RATIONAL ---- */
	double dblDiff, dblDiff2;
	uint64_t ullNum, ullDenom, ullNum2, ullDenom2;

	/*-- Check for negative values. If so it is an error. */
        /* Test written that way to catch NaN */
	if (!(value >= 0)) {
		*num = *denom = 0;
		NDPIErrorExt(0, "TIFFLib: DoubleToRational()", " Negative Value for Unsigned Rational given.");
		return;
	}

	/*-- Check for too big numbers (> ULONG_MAX) -- */
	if (value > 0xFFFFFFFFUL) {
		*num = 0xFFFFFFFFU;
		*denom = 0;
		return;
	}
	/*-- Check for easy integer numbers -- */
	if (value == (uint32_t)(value)) {
		*num = (uint32_t)value;
		*denom = 1;
		return;
	}
	/*-- Check for too small numbers for "unsigned long" type rationals -- */
	if (value < 1.0 / (double)0xFFFFFFFFUL) {
		*num = 0;
		*denom = 0xFFFFFFFFU;
		return;
	}

	/*-- There are two approaches using the Euclidean algorithm,
	*   which can accidentally lead to different accuracies just depending on the value.
	*   Try both and define which one was better.
	*/
	ToRationalEuclideanGCD(value, FALSE, FALSE, &ullNum, &ullDenom);
	ToRationalEuclideanGCD(value, FALSE, TRUE, &ullNum2, &ullDenom2);
	/*-- Double-Check, that returned values fit into ULONG :*/
	if (ullNum > 0xFFFFFFFFUL || ullDenom > 0xFFFFFFFFUL || ullNum2 > 0xFFFFFFFFUL || ullDenom2 > 0xFFFFFFFFUL) {
		NDPIErrorExt(0, "TIFFLib: DoubleToRational()", " Num or Denom exceeds ULONG: val=%14.6f, num=%12"PRIu64", denom=%12"PRIu64" | num2=%12"PRIu64", denom2=%12"PRIu64"", value, ullNum, ullDenom, ullNum2, ullDenom2);
		assert(0);
	}

	/* Check, which one has higher accuracy and take that. */
	dblDiff = fabs(value - ((double)ullNum / (double)ullDenom));
	dblDiff2 = fabs(value - ((double)ullNum2 / (double)ullDenom2));
	if (dblDiff < dblDiff2) {
		*num = (uint32_t)ullNum;
		*denom = (uint32_t)ullDenom;
	}
	else {
		*num = (uint32_t)ullNum2;
		*denom = (uint32_t)ullDenom2;
	}
}  /*-- DoubleToRational() -------------- */

/**---- DoubleToSrational() -----------------------------------------------
* Calculates the rational fractional of a double input value
* for SIGNED rationals,
* using the Euclidean algorithm to find the greatest common divisor (GCD)
------------------------------------------------------------------------*/
static
void DoubleToSrational(double value, int32_t *num, int32_t *denom)
{
	/*---- SIGNED RATIONAL ----*/
	int neg = 1;
	double dblDiff, dblDiff2;
	uint64_t ullNum, ullDenom, ullNum2, ullDenom2;

	/*-- Check for negative values and use then the positive one for internal calculations, but take the sign into account before returning. */
	if (value < 0) { neg = -1; value = -value; }

	/*-- Check for too big numbers (> LONG_MAX) -- */
	if (value > 0x7FFFFFFFL) {
		*num = 0x7FFFFFFFL;
		*denom = 0;
		return;
	}
	/*-- Check for easy numbers -- */
	if (value == (int32_t)(value)) {
		*num = (int32_t)(neg * value);
		*denom = 1;
		return;
	}
	/*-- Check for too small numbers for "long" type rationals -- */
	if (value < 1.0 / (double)0x7FFFFFFFL) {
		*num = 0;
		*denom = 0x7FFFFFFFL;
		return;
	}

	/*-- There are two approaches using the Euclidean algorithm,
	*   which can accidentally lead to different accuracies just depending on the value.
	*   Try both and define which one was better.
	*   Furthermore, set behavior of ToRationalEuclideanGCD() to the range of signed-long.
	*/
	ToRationalEuclideanGCD(value, TRUE, FALSE, &ullNum, &ullDenom);
	ToRationalEuclideanGCD(value, TRUE, TRUE, &ullNum2, &ullDenom2);
	/*-- Double-Check, that returned values fit into LONG :*/
	if (ullNum > 0x7FFFFFFFL || ullDenom > 0x7FFFFFFFL || ullNum2 > 0x7FFFFFFFL || ullDenom2 > 0x7FFFFFFFL) {
		NDPIErrorExt(0, "TIFFLib: DoubleToSrational()", " Num or Denom exceeds LONG: val=%14.6f, num=%12"PRIu64", denom=%12"PRIu64" | num2=%12"PRIu64", denom2=%12"PRIu64"", neg*value, ullNum, ullDenom, ullNum2, ullDenom2);
		assert(0);
	}

	/* Check, which one has higher accuracy and take that. */
	dblDiff = fabs(value - ((double)ullNum / (double)ullDenom));
	dblDiff2 = fabs(value - ((double)ullNum2 / (double)ullDenom2));
	if (dblDiff < dblDiff2) {
		*num = (int32_t)(neg * (long)ullNum);
		*denom = (int32_t)ullDenom;
	}
	else {
		*num = (int32_t)(neg * (long)ullNum2);
		*denom = (int32_t)ullDenom2;
	}
}  /*-- DoubleToSrational() --------------*/





#ifdef notdef
static int
NDPIWriteDirectoryTagCheckedFloat(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, float value)
{
	float m;
	assert(sizeof(float)==4);
	m=value;
	TIFFCvtNativeToIEEEFloat(tif,1,&m);
	if (tif->tif_flags&TIFF_SWAB)
		TIFFSwabFloat(&m);
	return(NDPIWriteDirectoryTagData(tif,ndir,dir,tag,TIFF_FLOAT,1,4,&m));
}
#endif

static int
NDPIWriteDirectoryTagCheckedFloatArray(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, float* value)
{
	assert(count<0x40000000);
	assert(sizeof(float)==4);
	TIFFCvtNativeToIEEEFloat(tif,count,&value);
	if (tif->tif_flags&TIFF_SWAB)
		TIFFSwabArrayOfFloat(value,count);
	return(NDPIWriteDirectoryTagData(tif,ndir,dir,tag,TIFF_FLOAT,count,count*4,value));
}

#ifdef notdef
static int
NDPIWriteDirectoryTagCheckedDouble(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, double value)
{
	double m;
	assert(sizeof(double)==8);
	m=value;
	TIFFCvtNativeToIEEEDouble(tif,1,&m);
	if (tif->tif_flags&TIFF_SWAB)
		TIFFSwabDouble(&m);
	return(NDPIWriteDirectoryTagData(tif,ndir,dir,tag,TIFF_DOUBLE,1,8,&m));
}
#endif

static int
NDPIWriteDirectoryTagCheckedDoubleArray(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, double* value)
{
	assert(count<0x20000000);
	assert(sizeof(double)==8);
	TIFFCvtNativeToIEEEDouble(tif,count,&value);
	if (tif->tif_flags&TIFF_SWAB)
		NDPISwabArrayOfDouble(value,count);
	return(NDPIWriteDirectoryTagData(tif,ndir,dir,tag,TIFF_DOUBLE,count,count*8,value));
}

static int
NDPIWriteDirectoryTagCheckedIfdArray(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, uint32_t* value)
{
	assert(count<0x40000000);
	assert(sizeof(uint32_t) == 4);
	if (tif->tif_flags&TIFF_SWAB)
		NDPISwabArrayOfLong(value,count);
	return(NDPIWriteDirectoryTagData(tif,ndir,dir,tag,TIFF_IFD,count,count*4,value));
}

static int
NDPIWriteDirectoryTagCheckedIfd8Array(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint32_t count, uint64_t* value)
{
	assert(count<0x20000000);
	assert(sizeof(uint64_t) == 8);
	assert(tif->tif_flags&TIFF_BIGTIFF);
	if (tif->tif_flags&TIFF_SWAB)
		TIFFSwabArrayOfLong8(value,count);
	return(NDPIWriteDirectoryTagData(tif,ndir,dir,tag,TIFF_IFD8,count,count*8,value));
}

static int
NDPIWriteDirectoryTagData(TIFF* tif, uint32_t* ndir, TIFFDirEntry* dir, uint16_t tag, uint16_t datatype, uint32_t count, uint32_t datalength, void* data)
{
	static const char module[] = "NDPIWriteDirectoryTagData";
	uint32_t m;
	m=0;
	while (m<(*ndir))
	{
		assert(dir[m].tdir_tag!=tag);
		if (dir[m].tdir_tag>tag)
			break;
		m++;
	}
	if (m<(*ndir))
	{
		uint32_t n;
		for (n=*ndir; n>m; n--)
			dir[n]=dir[n-1];
	}
	dir[m].tdir_tag=tag;
	dir[m].tdir_type=datatype;
	dir[m].tdir_count=count;
	dir[m].tdir_offset.toff_long8 = 0;
	if (datalength<=((tif->tif_flags&TIFF_BIGTIFF)?0x8U:0x4U))
        {
            if( data && datalength )
            {
                _NDPImemcpy(&dir[m].tdir_offset,data,datalength);
            }
        }
	else
	{
		uint64_t na,nb;
		na=tif->tif_dataoff;
		nb=na+datalength;
		if (!(tif->tif_flags&TIFF_BIGTIFF))
			nb=(uint32_t)nb;
		if ((nb<na)||(nb<datalength))
		{
			NDPIErrorExt(tif->tif_clientdata,module,"Maximum TIFF file size exceeded");
			return(0);
		}
		if (!SeekOK(tif,na))
		{
			NDPIErrorExt(tif->tif_clientdata,module,"IO error writing tag data");
			return(0);
		}
		assert(datalength<0x80000000UL);
		if (!WriteOK(tif,data,(tmsize_t)datalength))
		{
			NDPIErrorExt(tif->tif_clientdata,module,"IO error writing tag data");
			return(0);
		}
		tif->tif_dataoff=nb;
		if (tif->tif_dataoff&1)
			tif->tif_dataoff++;
		if (!(tif->tif_flags&TIFF_BIGTIFF))
		{
			uint32_t o;
			o=(uint32_t)na;
			if (tif->tif_flags&TIFF_SWAB)
				NDPISwabLong(&o);
			_NDPImemcpy(&dir[m].tdir_offset,&o,4);
		}
		else
		{
			dir[m].tdir_offset.toff_long8 = na;
			if (tif->tif_flags&TIFF_SWAB)
				NDPISwabLong8(&dir[m].tdir_offset.toff_long8);
		}
	}
	(*ndir)++;
	return(1);
}

/*
 * Link the current directory into the directory chain for the file.
 */
static int
TIFFLinkDirectory(TIFF* tif)
{
	static const char module[] = "TIFFLinkDirectory";

	tif->tif_diroff = (NDPISeekFile(tif,0,SEEK_END)+1) & (~((toff_t)1));

	/*
	 * Handle SubIFDs
	 */
	if (tif->tif_flags & TIFF_INSUBIFD)
	{
		if (!(tif->tif_flags&TIFF_BIGTIFF))
		{
			uint32_t m;
			m = (uint32_t)tif->tif_diroff;
			if (tif->tif_flags & TIFF_SWAB)
				NDPISwabLong(&m);
			(void) NDPISeekFile(tif, tif->tif_subifdoff, SEEK_SET);
			if (!WriteOK(tif, &m, 4)) {
				NDPIErrorExt(tif->tif_clientdata, module,
				     "Error writing SubIFD directory link");
				return (0);
			}
			/*
			 * Advance to the next SubIFD or, if this is
			 * the last one configured, revert back to the
			 * normal directory linkage.
			 */
			if (--tif->tif_nsubifd)
				tif->tif_subifdoff += 4;
			else
				tif->tif_flags &= ~TIFF_INSUBIFD;
			return (1);
		}
		else
		{
			uint64_t m;
			m = tif->tif_diroff;
			if (tif->tif_flags & TIFF_SWAB)
				NDPISwabLong8(&m);
			(void) NDPISeekFile(tif, tif->tif_subifdoff, SEEK_SET);
			if (!WriteOK(tif, &m, 8)) {
				NDPIErrorExt(tif->tif_clientdata, module,
				     "Error writing SubIFD directory link");
				return (0);
			}
			/*
			 * Advance to the next SubIFD or, if this is
			 * the last one configured, revert back to the
			 * normal directory linkage.
			 */
			if (--tif->tif_nsubifd)
				tif->tif_subifdoff += 8;
			else
				tif->tif_flags &= ~TIFF_INSUBIFD;
			return (1);
		}
	}

	if (!(tif->tif_flags&TIFF_BIGTIFF))
	{
		uint32_t m;
		uint32_t nextdir;
		m = (uint32_t)(tif->tif_diroff);
		if (tif->tif_flags & TIFF_SWAB)
			NDPISwabLong(&m);
		if (tif->tif_header.classic.tiff_diroff == 0) {
			/*
			 * First directory, overwrite offset in header.
			 */
			tif->tif_header.classic.tiff_diroff = (uint32_t) tif->tif_diroff;
			(void) NDPISeekFile(tif,4, SEEK_SET);
			if (!WriteOK(tif, &m, 4)) {
				NDPIErrorExt(tif->tif_clientdata, tif->tif_name,
					     "Error writing TIFF header");
				return (0);
			}
			return (1);
		}
		/*
		 * Not the first directory, search to the last and append.
		 */
		nextdir = tif->tif_header.classic.tiff_diroff;
		while(1) {
			uint16_t dircount;
			uint32_t nextnextdir;

			if (!SeekOK(tif, nextdir) ||
			    !ReadOK(tif, &dircount, 2)) {
				NDPIErrorExt(tif->tif_clientdata, module,
					     "Error fetching directory count");
				return (0);
			}
			if (tif->tif_flags & TIFF_SWAB)
				NDPISwabShort(&dircount);
			(void) NDPISeekFile(tif,
			    nextdir+2+dircount*12, SEEK_SET);
			if (!ReadOK(tif, &nextnextdir, 4)) {
				NDPIErrorExt(tif->tif_clientdata, module,
					     "Error fetching directory link");
				return (0);
			}
			if (tif->tif_flags & TIFF_SWAB)
				NDPISwabLong(&nextnextdir);
			if (nextnextdir==0)
			{
				(void) NDPISeekFile(tif,
				    nextdir+2+dircount*12, SEEK_SET);
				if (!WriteOK(tif, &m, 4)) {
					NDPIErrorExt(tif->tif_clientdata, module,
					     "Error writing directory link");
					return (0);
				}
				break;
			}
			nextdir=nextnextdir;
		}
	}
	else
	{
		uint64_t m;
		uint64_t nextdir;
		m = tif->tif_diroff;
		if (tif->tif_flags & TIFF_SWAB)
			NDPISwabLong8(&m);
		if (tif->tif_header.big.tiff_diroff == 0) {
			/*
			 * First directory, overwrite offset in header.
			 */
			tif->tif_header.big.tiff_diroff = tif->tif_diroff;
			(void) NDPISeekFile(tif,8, SEEK_SET);
			if (!WriteOK(tif, &m, 8)) {
				NDPIErrorExt(tif->tif_clientdata, tif->tif_name,
					     "Error writing TIFF header");
				return (0);
			}
			return (1);
		}
		/*
		 * Not the first directory, search to the last and append.
		 */
		nextdir = tif->tif_header.big.tiff_diroff;
		while(1) {
			uint64_t dircount64;
			uint16_t dircount;
			uint64_t nextnextdir;

			if (!SeekOK(tif, nextdir) ||
			    !ReadOK(tif, &dircount64, 8)) {
				NDPIErrorExt(tif->tif_clientdata, module,
					     "Error fetching directory count");
				return (0);
			}
			if (tif->tif_flags & TIFF_SWAB)
				NDPISwabLong8(&dircount64);
			if (dircount64>0xFFFF)
			{
				NDPIErrorExt(tif->tif_clientdata, module,
					     "Sanity check on tag count failed, likely corrupt TIFF");
				return (0);
			}
			dircount=(uint16_t)dircount64;
			(void) NDPISeekFile(tif,
			    nextdir+8+dircount*20, SEEK_SET);
			if (!ReadOK(tif, &nextnextdir, 8)) {
				NDPIErrorExt(tif->tif_clientdata, module,
					     "Error fetching directory link");
				return (0);
			}
			if (tif->tif_flags & TIFF_SWAB)
				NDPISwabLong8(&nextnextdir);
			if (nextnextdir==0)
			{
				(void) NDPISeekFile(tif,
				    nextdir+8+dircount*20, SEEK_SET);
				if (!WriteOK(tif, &m, 8)) {
					NDPIErrorExt(tif->tif_clientdata, module,
					     "Error writing directory link");
					return (0);
				}
				break;
			}
			nextdir=nextnextdir;
		}
	}
	return (1);
}

/************************************************************************/
/*                          TIFFRewriteField()                          */
/*                                                                      */
/*      Rewrite a field in the directory on disk without regard to      */
/*      updating the TIFF directory structure in memory.  Currently     */
/*      only supported for field that already exist in the on-disk      */
/*      directory.  Mainly used for updating stripoffset /              */
/*      stripbytecount values after the directory is already on         */
/*      disk.                                                           */
/*                                                                      */
/*      Returns zero on failure, and one on success.                    */
/************************************************************************/

int
_NDPIRewriteField(TIFF* tif, uint16_t tag, TIFFDataType in_datatype,
                  tmsize_t count, void* data)
{
    static const char module[] = "TIFFResetField";
    /* const TIFFField* fip = NULL; */
    uint16_t dircount;
    tmsize_t dirsize;
    uint8_t direntry_raw[20];
    uint16_t entry_tag = 0;
    uint16_t entry_type = 0;
    uint64_t entry_count = 0;
    uint64_t entry_offset = 0;
    int    value_in_entry = 0;
    uint64_t read_offset;
    uint8_t *buf_to_write = NULL;
    TIFFDataType datatype;

/* -------------------------------------------------------------------- */
/*      Find field definition.                                          */
/* -------------------------------------------------------------------- */
    /*fip =*/ NDPIFindField(tif, tag, TIFF_ANY);

/* -------------------------------------------------------------------- */
/*      Do some checking this is a straight forward case.               */
/* -------------------------------------------------------------------- */
    if( isMapped(tif) )
    {
        NDPIErrorExt( tif->tif_clientdata, module, 
                      "Memory mapped files not currently supported for this operation." );
        return 0;
    }

    if( tif->tif_diroff == 0 )
    {
        NDPIErrorExt( tif->tif_clientdata, module, 
                      "Attempt to reset field on directory not already on disk." );
        return 0;
    }

/* -------------------------------------------------------------------- */
/*      Read the directory entry count.                                 */
/* -------------------------------------------------------------------- */
    if (!SeekOK(tif, tif->tif_diroff)) {
        NDPIErrorExt(tif->tif_clientdata, module,
                     "%s: Seek error accessing TIFF directory",
                     tif->tif_name);
        return 0;
    }

    read_offset = tif->tif_diroff;

    if (!(tif->tif_flags&TIFF_BIGTIFF))
    {
        if (!ReadOK(tif, &dircount, sizeof (uint16_t))) {
            NDPIErrorExt(tif->tif_clientdata, module,
                         "%s: Can not read TIFF directory count",
                         tif->tif_name);
            return 0;
        }
        if (tif->tif_flags & TIFF_SWAB)
            NDPISwabShort(&dircount);
        dirsize = 12;
        read_offset += 2;
    } else {
        uint64_t dircount64;
        if (!ReadOK(tif, &dircount64, sizeof (uint64_t))) {
            NDPIErrorExt(tif->tif_clientdata, module,
                         "%s: Can not read TIFF directory count",
                         tif->tif_name);
            return 0;
        }
        if (tif->tif_flags & TIFF_SWAB)
            NDPISwabLong8(&dircount64);
        dircount = (uint16_t)dircount64;
        dirsize = 20;
        read_offset += 8;
    }

/* -------------------------------------------------------------------- */
/*      Read through directory to find target tag.                      */
/* -------------------------------------------------------------------- */
    while( dircount > 0 )
    {
        if (!ReadOK(tif, direntry_raw, dirsize)) {
            NDPIErrorExt(tif->tif_clientdata, module,
                         "%s: Can not read TIFF directory entry.",
                         tif->tif_name);
            return 0;
        }

        memcpy( &entry_tag, direntry_raw + 0, sizeof(uint16_t) );
        if (tif->tif_flags&TIFF_SWAB)
            NDPISwabShort( &entry_tag );

        if( entry_tag == tag )
            break;

        read_offset += dirsize;
    }

    if( entry_tag != tag )
    {
        NDPIErrorExt(tif->tif_clientdata, module,
                     "%s: Could not find tag %"PRIu16".",
                     tif->tif_name, tag );
        return 0;
    }

/* -------------------------------------------------------------------- */
/*      Extract the type, count and offset for this entry.              */
/* -------------------------------------------------------------------- */
    memcpy( &entry_type, direntry_raw + 2, sizeof(uint16_t) );
    if (tif->tif_flags&TIFF_SWAB)
        NDPISwabShort( &entry_type );

    if (!(tif->tif_flags&TIFF_BIGTIFF))
    {
        uint32_t value;
        
        memcpy( &value, direntry_raw + 4, sizeof(uint32_t) );
        if (tif->tif_flags&TIFF_SWAB)
            NDPISwabLong( &value );
        entry_count = value;

        memcpy( &value, direntry_raw + 8, sizeof(uint32_t) );
        if (tif->tif_flags&TIFF_SWAB)
            NDPISwabLong( &value );
        entry_offset = value;
    }
    else
    {
        memcpy( &entry_count, direntry_raw + 4, sizeof(uint64_t) );
        if (tif->tif_flags&TIFF_SWAB)
            NDPISwabLong8( &entry_count );

        memcpy( &entry_offset, direntry_raw + 12, sizeof(uint64_t) );
        if (tif->tif_flags&TIFF_SWAB)
            NDPISwabLong8( &entry_offset );
    }

/* -------------------------------------------------------------------- */
/*      When a dummy tag was written due to NDPIDeferStrileArrayWriting() */
/* -------------------------------------------------------------------- */
    if( entry_offset == 0 && entry_count == 0 && entry_type == 0 )
    {
        if( tag == TIFFTAG_TILEOFFSETS || tag == TIFFTAG_STRIPOFFSETS )
        {
            entry_type = (tif->tif_flags&TIFF_BIGTIFF) ? TIFF_LONG8 : TIFF_LONG; 
        }
        else
        {
            int write_aslong8 = 1;
            if( count > 1 && tag == TIFFTAG_STRIPBYTECOUNTS )
            {
                write_aslong8 = WriteAsLong8(tif, NDPIStripSize64(tif));
            }
            else if( count > 1 && tag == TIFFTAG_TILEBYTECOUNTS )
            {
                write_aslong8 = WriteAsLong8(tif, NDPITileSize64(tif));
            }
            if( write_aslong8 )
            {
                entry_type = TIFF_LONG8;
            }
            else
            {
                int write_aslong4 = 1;
                if( count > 1 && tag == TIFFTAG_STRIPBYTECOUNTS )
                {
                    write_aslong4 = WriteAsLong4(tif, NDPIStripSize64(tif));
                }
                else if( count > 1 && tag == TIFFTAG_TILEBYTECOUNTS )
                {
                    write_aslong4 = WriteAsLong4(tif, NDPITileSize64(tif));
                }
                if( write_aslong4 )
                {
                    entry_type = TIFF_LONG;
                }
                else
                {
                    entry_type = TIFF_SHORT;
                }
            }
        }
    }

/* -------------------------------------------------------------------- */
/*      What data type do we want to write this as?                     */
/* -------------------------------------------------------------------- */
    if( TIFFDataWidth(in_datatype) == 8 && !(tif->tif_flags&TIFF_BIGTIFF) )
    {
        if( in_datatype == TIFF_LONG8 )
            datatype = entry_type == TIFF_SHORT ? TIFF_SHORT : TIFF_LONG;
        else if( in_datatype == TIFF_SLONG8 )
            datatype = TIFF_SLONG;
        else if( in_datatype == TIFF_IFD8 )
            datatype = TIFF_IFD;
        else
            datatype = in_datatype;
    }
    else
    {
        if( in_datatype == TIFF_LONG8 &&
            (entry_type == TIFF_SHORT || entry_type == TIFF_LONG ||
             entry_type == TIFF_LONG8 ) )
            datatype = entry_type;
        else if( in_datatype == TIFF_SLONG8 &&
            (entry_type == TIFF_SLONG || entry_type == TIFF_SLONG8 ) )
            datatype = entry_type;
        else if( in_datatype == TIFF_IFD8 &&
            (entry_type == TIFF_IFD || entry_type == TIFF_IFD8 ) )
            datatype = entry_type;
        else
            datatype = in_datatype;
    }

/* -------------------------------------------------------------------- */
/*      Prepare buffer of actual data to write.  This includes          */
/*      swabbing as needed.                                             */
/* -------------------------------------------------------------------- */
    buf_to_write =
	    (uint8_t *)_NDPICheckMalloc(tif, count, TIFFDataWidth(datatype),
                                    "for field buffer.");
    if (!buf_to_write)
        return 0;

    if( datatype == in_datatype )
        memcpy( buf_to_write, data, count * TIFFDataWidth(datatype) );
    else if( datatype == TIFF_SLONG && in_datatype == TIFF_SLONG8 )
    {
	tmsize_t i;

        for( i = 0; i < count; i++ )
        {
            ((int32_t *) buf_to_write)[i] =
                (int32_t) ((int64_t *) data)[i];
            if((int64_t) ((int32_t *) buf_to_write)[i] != ((int64_t *) data)[i] )
            {
                _NDPIfree( buf_to_write );
                NDPIErrorExt( tif->tif_clientdata, module, 
                              "Value exceeds 32bit range of output type." );
                return 0;
            }
        }
    }
    else if( (datatype == TIFF_LONG && in_datatype == TIFF_LONG8)
             || (datatype == TIFF_IFD && in_datatype == TIFF_IFD8) )
    {
	tmsize_t i;

        for( i = 0; i < count; i++ )
        {
            ((uint32_t *) buf_to_write)[i] =
                (uint32_t) ((uint64_t *) data)[i];
            if((uint64_t) ((uint32_t *) buf_to_write)[i] != ((uint64_t *) data)[i] )
            {
                _NDPIfree( buf_to_write );
                NDPIErrorExt( tif->tif_clientdata, module, 
                              "Value exceeds 32bit range of output type." );
                return 0;
            }
        }
    }
    else if( datatype == TIFF_SHORT && in_datatype == TIFF_LONG8 )
    {
	tmsize_t i;

        for( i = 0; i < count; i++ )
        {
            ((uint16_t *) buf_to_write)[i] =
                (uint16_t) ((uint64_t *) data)[i];
            if((uint64_t) ((uint16_t *) buf_to_write)[i] != ((uint64_t *) data)[i] )
            {
                _NDPIfree( buf_to_write );
                NDPIErrorExt( tif->tif_clientdata, module,
                              "Value exceeds 16bit range of output type." );
                return 0;
            }
        }
    }
    else
    {
        NDPIErrorExt( tif->tif_clientdata, module,
                      "Unhandled type conversion." );
        return 0;
    }

    if( TIFFDataWidth(datatype) > 1 && (tif->tif_flags&TIFF_SWAB) )
    {
        if( TIFFDataWidth(datatype) == 2 )
            NDPISwabArrayOfShort((uint16_t *) buf_to_write, count );
        else if( TIFFDataWidth(datatype) == 4 )
            NDPISwabArrayOfLong((uint32_t *) buf_to_write, count );
        else if( TIFFDataWidth(datatype) == 8 )
            TIFFSwabArrayOfLong8((uint64_t *) buf_to_write, count );
    }

/* -------------------------------------------------------------------- */
/*      Is this a value that fits into the directory entry?             */
/* -------------------------------------------------------------------- */
    if (!(tif->tif_flags&TIFF_BIGTIFF))
    {
        if( TIFFDataWidth(datatype) * count <= 4 )
        {
            entry_offset = read_offset + 8;
            value_in_entry = 1;
        }
    }
    else
    {
        if( TIFFDataWidth(datatype) * count <= 8 )
        {
            entry_offset = read_offset + 12;
            value_in_entry = 1;
        }
    }

    if( (tag == TIFFTAG_TILEOFFSETS || tag == TIFFTAG_STRIPOFFSETS) &&
        tif->tif_dir.td_stripoffset_entry.tdir_count == 0 &&
        tif->tif_dir.td_stripoffset_entry.tdir_type == 0 &&
        tif->tif_dir.td_stripoffset_entry.tdir_offset.toff_long8 == 0 )
    {
        tif->tif_dir.td_stripoffset_entry.tdir_type = datatype;
        tif->tif_dir.td_stripoffset_entry.tdir_count = count;
    }
    else if( (tag == TIFFTAG_TILEBYTECOUNTS || tag == TIFFTAG_STRIPBYTECOUNTS) &&
        tif->tif_dir.td_stripbytecount_entry.tdir_count == 0 &&
        tif->tif_dir.td_stripbytecount_entry.tdir_type == 0 &&
        tif->tif_dir.td_stripbytecount_entry.tdir_offset.toff_long8 == 0 )
    {
        tif->tif_dir.td_stripbytecount_entry.tdir_type = datatype;
        tif->tif_dir.td_stripbytecount_entry.tdir_count = count;
    }

/* -------------------------------------------------------------------- */
/*      If the tag type, and count match, then we just write it out     */
/*      over the old values without altering the directory entry at     */
/*      all.                                                            */
/* -------------------------------------------------------------------- */
    if( entry_count == (uint64_t)count && entry_type == (uint16_t) datatype )
    {
        if (!SeekOK(tif, entry_offset)) {
            _NDPIfree( buf_to_write );
            NDPIErrorExt(tif->tif_clientdata, module,
                         "%s: Seek error accessing TIFF directory",
                         tif->tif_name);
            return 0;
        }
        if (!WriteOK(tif, buf_to_write, count*TIFFDataWidth(datatype))) {
            _NDPIfree( buf_to_write );
            NDPIErrorExt(tif->tif_clientdata, module,
                         "Error writing directory link");
            return (0);
        }

        _NDPIfree( buf_to_write );
        return 1;
    }

/* -------------------------------------------------------------------- */
/*      Otherwise, we write the new tag data at the end of the file.    */
/* -------------------------------------------------------------------- */
    if( !value_in_entry )
    {
        entry_offset = NDPISeekFile(tif,0,SEEK_END);
        
        if (!WriteOK(tif, buf_to_write, count*TIFFDataWidth(datatype))) {
            _NDPIfree( buf_to_write );
            NDPIErrorExt(tif->tif_clientdata, module,
                         "Error writing directory link");
            return (0);
        }
    }
    else
    {
        memcpy( &entry_offset, buf_to_write, count*TIFFDataWidth(datatype));
    }

    _NDPIfree( buf_to_write );
    buf_to_write = 0;

/* -------------------------------------------------------------------- */
/*      Adjust the directory entry.                                     */
/* -------------------------------------------------------------------- */
    entry_type = datatype;
    entry_count = (uint64_t)count;
    memcpy( direntry_raw + 2, &entry_type, sizeof(uint16_t) );
    if (tif->tif_flags&TIFF_SWAB)
        NDPISwabShort( (uint16_t *) (direntry_raw + 2) );

    if (!(tif->tif_flags&TIFF_BIGTIFF))
    {
        uint32_t value;

        value = (uint32_t) entry_count;
        memcpy( direntry_raw + 4, &value, sizeof(uint32_t) );
        if (tif->tif_flags&TIFF_SWAB)
            NDPISwabLong( (uint32_t *) (direntry_raw + 4) );

        value = (uint32_t) entry_offset;
        memcpy( direntry_raw + 8, &value, sizeof(uint32_t) );
        if (tif->tif_flags&TIFF_SWAB)
            NDPISwabLong( (uint32_t *) (direntry_raw + 8) );
    }
    else
    {
        memcpy( direntry_raw + 4, &entry_count, sizeof(uint64_t) );
        if (tif->tif_flags&TIFF_SWAB)
            NDPISwabLong8( (uint64_t *) (direntry_raw + 4) );

        memcpy( direntry_raw + 12, &entry_offset, sizeof(uint64_t) );
        if (tif->tif_flags&TIFF_SWAB)
            NDPISwabLong8( (uint64_t *) (direntry_raw + 12) );
    }

/* -------------------------------------------------------------------- */
/*      Write the directory entry out to disk.                          */
/* -------------------------------------------------------------------- */
    if (!SeekOK(tif, read_offset )) {
        NDPIErrorExt(tif->tif_clientdata, module,
                     "%s: Seek error accessing TIFF directory",
                     tif->tif_name);
        return 0;
    }

    if (!WriteOK(tif, direntry_raw,dirsize))
    {
        NDPIErrorExt(tif->tif_clientdata, module,
                     "%s: Can not write TIFF directory entry.",
                     tif->tif_name);
        return 0;
    }
    
    return 1;
}
/* vim: set ts=8 sts=8 sw=8 noet: */
/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 8
 * fill-column: 78
 * End:
 */
