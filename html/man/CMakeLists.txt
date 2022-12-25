# CMake build for libtiff
#
# Copyright © 2015 Open Microscopy Environment / University of Dundee
# Written by Roger Leigh <rleigh@codelibre.net>
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

set(indexfile index.html)

set(docfiles
    libtiff.3tiff.html
    TIFFbuffer.3tiff.html
    NDPIClose.3tiff.html
    TIFFcodec.3tiff.html
    TIFFcolor.3tiff.html
    TIFFDataWidth.3tiff.html
    NDPIError.3tiff.html
    TIFFFieldDataType.3tiff.html
    TIFFFieldName.3tiff.html
    TIFFFieldPassCount.3tiff.html
    TIFFFieldReadCount.3tiff.html
    TIFFFieldTag.3tiff.html
    TIFFFieldWriteCount.3tiff.html
    NDPIFlush.3tiff.html
    NDPIGetField.3tiff.html
    TIFFmemory.3tiff.html
    NDPIOpen.3tiff.html
    NDPIPrintDirectory.3tiff.html
    TIFFquery.3tiff.html
    NDPIReadDirectory.3tiff.html
    NDPIReadEncodedStrip.3tiff.html
    NDPIReadEncodedTile.3tiff.html
    NDPIReadRawStrip.3tiff.html
    NDPIReadRawTile.3tiff.html
    NDPIReadRGBAImage.3tiff.html
    NDPIReadRGBAStrip.3tiff.html
    NDPIReadRGBATile.3tiff.html
    NDPIReadScanline.3tiff.html
    NDPIReadTile.3tiff.html
    TIFFRGBAImage.3tiff.html
    NDPISetDirectory.3tiff.html
    NDPISetField.3tiff.html
    TIFFsize.3tiff.html
    TIFFstrip.3tiff.html
    TIFFswab.3tiff.html
    TIFFtile.3tiff.html
    TIFFWarning.3tiff.html
    NDPIWriteDirectory.3tiff.html
    TIFFWriteEncodedStrip.3tiff.html
    TIFFWriteEncodedTile.3tiff.html
    TIFFWriteRawStrip.3tiff.html
    TIFFWriteRawTile.3tiff.html
    TIFFWriteScanline.3tiff.html
    TIFFWriteTile.3tiff.html
    fax2ps.1.html
    fax2tiff.1.html
    pal2rgb.1.html
    ppm2tiff.1.html
    raw2tiff.1.html
    tiff2bw.1.html
    tiff2pdf.1.html
    tiff2ps.1.html
    tiff2rgba.1.html
    tiffcmp.1.html
    tiffcp.1.html
    tiffcrop.1.html
    tiffdither.1.html
    tiffdump.1.html
    tiffgt.1.html
    tiffinfo.1.html
    tiffmedian.1.html
    tiffset.1.html
    tiffsplit.1.html)

set(doc_DATA ${indexfile} ${docfiles})

install(FILES ${doc_DATA}
        DESTINATION "${LIBTIFF_DOCDIR}/html/man")

# htmldoc target to regenerate HTML files
string(REPLACE ";" "^" escaped_docfiles "${docfiles}")
add_custom_target(htmldoc
  COMMAND "${CMAKE_COMMAND}"
  "-DMANSRCDIR=${PROJECT_SOURCE_DIR}/man"
  "-DHTMLMANDIR=${PROJECT_SOURCE_DIR}/html/man"
  "-DINDEXFILE=${indexfile}"
  "-DDOCFILES=${escaped_docfiles}"
  -P "${CMAKE_CURRENT_SOURCE_DIR}/HtmlDoc.cmake")

