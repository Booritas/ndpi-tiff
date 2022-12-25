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

set(man1_MANS
  fax2ps.1
  fax2tiff.1
  pal2rgb.1
  ppm2tiff.1
  raw2tiff.1
  tiff2bw.1
  tiff2pdf.1
  tiff2ps.1
  tiff2rgba.1
  tiffcmp.1
  tiffcp.1
  tiffcrop.1
  tiffdither.1
  tiffdump.1
  tiffgt.1
  tiffinfo.1
  tiffmedian.1
  tiffset.1
  tiffsplit.1)

set(man3_MANS
  libtiff.3tiff
  TIFFbuffer.3tiff
  NDPIClose.3tiff
  TIFFcodec.3tiff
  TIFFcolor.3tiff
  TIFFDataWidth.3tiff
  NDPIError.3tiff
  NDPIFieldDataType.3tiff
  NDPIFieldName.3tiff
  NDPIFieldPassCount.3tiff
  TIFFFieldReadCount.3tiff
  NDPIFieldTag.3tiff
  NDPIFieldWriteCount.3tiff
  NDPIFlush.3tiff
  NDPIGetField.3tiff
  TIFFmemory.3tiff
  NDPIOpen.3tiff
  NDPIPrintDirectory.3tiff
  TIFFquery.3tiff
  NDPIReadDirectory.3tiff
  NDPIReadEncodedStrip.3tiff
  NDPIReadEncodedTile.3tiff
  NDPIReadRawStrip.3tiff
  NDPIReadRawTile.3tiff
  NDPIReadRGBAImage.3tiff
  NDPIReadRGBAStrip.3tiff
  NDPIReadRGBATile.3tiff
  NDPIReadScanline.3tiff
  NDPIReadTile.3tiff
  TIFFRGBAImage.3tiff
  NDPISetDirectory.3tiff
  NDPISetField.3tiff
  TIFFsize.3tiff
  TIFFstrip.3tiff
  TIFFswab.3tiff
  TIFFtile.3tiff
  NDPIWarning.3tiff
  NDPIWriteDirectory.3tiff
  NDPIWriteEncodedStrip.3tiff
  NDPIWriteEncodedTile.3tiff
  NDPIWriteRawStrip.3tiff
  NDPIWriteRawTile.3tiff
  NDPIWriteScanline.3tiff
  NDPIWriteTile.3tiff)

install(FILES ${man1_MANS}
        DESTINATION "${CMAKE_INSTALL_FULL_MANDIR}/man1")
install(FILES ${man3_MANS}
        DESTINATION "${CMAKE_INSTALL_FULL_MANDIR}/man3")
