# Tag Image File Format (TIFF) Software
#
# Copyright (C) 2004, Andrey Kiselev <dron@ak4719.spb.edu>
#
# Permission to use, copy, modify, distribute, and sell this software and 
# its documentation for any purpose is hereby granted without fee, provided
# that (i) the above copyright notices and this permission notice appear in
# all copies of the software and related documentation, and (ii) the names of
# Sam Leffler and Silicon Graphics may not be used in any advertising or
# publicity relating to the software without the specific, prior written
# permission of Sam Leffler and Silicon Graphics.
# 
# THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND, 
# EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY 
# WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  
# 
# IN NO EVENT SHALL SAM LEFFLER OR SILICON GRAPHICS BE LIABLE FOR
# ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
# OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
# WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF 
# LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE 
# OF THIS SOFTWARE.

# Process this file with automake to produce Makefile.in.

docdir = $(LIBTIFF_DOCDIR)/html/man
MANSRCDIR = $(top_srcdir)/man
HTMLMANDIR = $(top_srcdir)/html/man

GROFF = groff -Thtml -mandoc
ECHO = echo

indexfile = index.html
docfiles = \
	libtiff.3tiff.html \
	TIFFbuffer.3tiff.html \
	NDPIClose.3tiff.html \
	TIFFcodec.3tiff.html \
	TIFFcolor.3tiff.html \
	TIFFDataWidth.3tiff.html \
	NDPIError.3tiff.html \
	NDPIFieldDataType.3tiff.html \
	NDPIFieldName.3tiff.html \
	NDPIFieldPassCount.3tiff.html \
	TIFFFieldReadCount.3tiff.html \
	NDPIFieldTag.3tiff.html \
	NDPIFieldWriteCount.3tiff.html \
	NDPIFlush.3tiff.html \
	NDPIGetField.3tiff.html \
	TIFFmemory.3tiff.html \
	NDPIOpen.3tiff.html \
	NDPIPrintDirectory.3tiff.html \
	TIFFquery.3tiff.html \
	NDPIReadDirectory.3tiff.html \
	NDPIReadEncodedStrip.3tiff.html \
	NDPIReadEncodedTile.3tiff.html \
	NDPIReadRawStrip.3tiff.html \
	NDPIReadRawTile.3tiff.html \
	NDPIReadRGBAImage.3tiff.html \
	NDPIReadRGBAStrip.3tiff.html \
	NDPIReadRGBATile.3tiff.html \
	NDPIReadScanline.3tiff.html \
	NDPIReadTile.3tiff.html \
	TIFFRGBAImage.3tiff.html \
	NDPISetDirectory.3tiff.html \
	NDPISetField.3tiff.html \
	TIFFsize.3tiff.html \
	TIFFstrip.3tiff.html \
	TIFFswab.3tiff.html \
	TIFFtile.3tiff.html \
	NDPIWarning.3tiff.html \
	NDPIWriteDirectory.3tiff.html \
	NDPIWriteEncodedStrip.3tiff.html \
	NDPIWriteEncodedTile.3tiff.html \
	NDPIWriteRawStrip.3tiff.html \
	NDPIWriteRawTile.3tiff.html \
	NDPIWriteScanline.3tiff.html \
	NDPIWriteTile.3tiff.html \
	fax2ps.1.html \
	fax2tiff.1.html \
	pal2rgb.1.html \
	ppm2tiff.1.html \
	raw2tiff.1.html \
	tiff2bw.1.html \
	tiff2pdf.1.html \
	tiff2ps.1.html \
	tiff2rgba.1.html \
	tiffcmp.1.html \
	tiffcp.1.html \
	tiffcrop.1.html \
	tiffdither.1.html \
	tiffdump.1.html \
	tiffgt.1.html \
	tiffinfo.1.html \
	tiffmedian.1.html \
	tiffset.1.html \
	tiffsplit.1.html

dist_doc_DATA = $(indexfile) $(docfiles)

INDEXSTART = '<HTML><HEAD><TITLE>Libtiff HTML manpage index</TITLE></HEAD><BODY BGCOLOR=white><ul><H2>Man Pages</h2><p>'
INDEXEND = '</ul></BODY></HTML>'

.PHONY: index
index:
	${ECHO} ${INDEXSTART} > $(indexfile)
	for i in $(docfiles); do					\
		${ECHO} '<li><A HREF='$$i'>'$$i'</a>' >> $(indexfile);  \
	done
	${ECHO} ${INDEXEND} >> $(indexfile)

manpages = $(docfiles:.html=)

.PHONY: htmldoc
htmldoc:
	for i in $(manpages); do					\
		${GROFF} $(MANSRCDIR)/$$i > $(HTMLMANDIR)/$$i.html;	\
	done

EXTRA_DIST = \
	CMakeLists.txt \
	HtmlDoc.cmake
