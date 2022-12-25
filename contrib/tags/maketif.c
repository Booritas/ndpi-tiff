/*
 * maketif.c -- creates a little TIFF file, with
 *   the XTIFF extended tiff example tags.
 */

#include <stdlib.h>
#include "xtiffio.h"


void SetUpTIFFDirectory(TIFF *tif);
void WriteImage(TIFF *tif);

#define WIDTH 20
#define HEIGHT 20

void main()
{
	TIFF *tif=(TIFF*)0;  /* TIFF-level descriptor */
	
	tif=XTIFFOpen("newtif.tif","w");
	if (!tif) goto failure;
	
	SetUpTIFFDirectory(tif);
	WriteImage(tif);
	
	XTIFFClose(tif);
	exit (0);
	
failure:
	printf("failure in maketif\n");
	if (tif) XTIFFClose(tif);
	exit (-1);
}


void SetUpTIFFDirectory(TIFF *tif)
{
	double mymulti[6]={0.0,1.0,2.0,  3.1415926, 5.0,1.0};
	uint32_t mysingle=3456;
	char *ascii="This file was produced by Steven Spielberg. NOT";

	NDPISetField(tif,TIFFTAG_IMAGEWIDTH,WIDTH);
	NDPISetField(tif,TIFFTAG_IMAGELENGTH,HEIGHT);
	NDPISetField(tif,TIFFTAG_COMPRESSION,COMPRESSION_NONE);
	NDPISetField(tif,TIFFTAG_PHOTOMETRIC,PHOTOMETRIC_MINISBLACK);
	NDPISetField(tif,TIFFTAG_PLANARCONFIG,PLANARCONFIG_CONTIG);
	NDPISetField(tif,TIFFTAG_BITSPERSAMPLE,8);
	NDPISetField(tif,TIFFTAG_ROWSPERSTRIP,20);

	/* Install the extended TIFF tag examples */
	NDPISetField(tif,TIFFTAG_EXAMPLE_MULTI,6,mymulti);
	NDPISetField(tif,TIFFTAG_EXAMPLE_SINGLE,mysingle);
	NDPISetField(tif,TIFFTAG_EXAMPLE_ASCII,ascii);
}


void WriteImage(TIFF *tif)
{
	int i;
	char buffer[WIDTH];
	
	memset(buffer,0,sizeof(buffer));
	for (i=0;i<HEIGHT;i++)
		if (!NDPIWriteScanline(tif, buffer, i, 0))
			NDPIErrorExt(tif->tif_clientdata, "WriteImage","failure in WriteScanline\n");
}




/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 8
 * fill-column: 78
 * End:
 */
