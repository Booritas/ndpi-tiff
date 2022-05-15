#include <gtest/gtest.h>
#include "tiff.h"
#include "tiffio.h"

// Demonstrate some basic assertions.
TEST(NDPITest, openFile) {
	//std::string path = R"(d:\Projects\slideio\images\hamamatsu\2017-02-27 15.29.08.ndpi)";
    std::string path = R"(d:\Projects\slideio\images\hamamatsu\DM0014 - 2020-04-02 10.25.21.ndpi)";
    TIFF* tif = TIFFOpen(path.c_str(), "r");
    ASSERT_TRUE(tif != NULL);
    int dircount = TIFFNumberOfDirectories(tif);
    //ASSERT_EQ(5, dircount);
    for(int dir=0; dir<dircount; ++dir)
    {
        int success = TIFFSetDirectory(tif, dir);
        ASSERT_EQ(1, success);
        char* description(nullptr);
        short dirchnls(0), dirbits(0);
        uint16_t compress(0);
        short  planar_config(0);
        int width(0), height(0), tile_width(0), tile_height(0);
        TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &dirchnls);
        TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &dirbits);
        TIFFGetField(tif, TIFFTAG_COMPRESSION, &compress);
        TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
        TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
        TIFFGetField(tif, TIFFTAG_TILEWIDTH, &tile_width);
        TIFFGetField(tif, TIFFTAG_TILELENGTH, &tile_height);
        TIFFGetField(tif, TIFFTAG_IMAGEDESCRIPTION, &description);
        TIFFGetField(tif, TIFFTAG_PLANARCONFIG, &planar_config);
        float resx(0), resy(0);
        uint16_t units(0);
        TIFFGetField(tif, TIFFTAG_XRESOLUTION, &resx);
        TIFFGetField(tif, TIFFTAG_YRESOLUTION, &resy);
        TIFFGetField(tif, TIFFTAG_RESOLUTIONUNIT, &units);
        float posx(0), posy(0);
        TIFFGetField(tif, TIFFTAG_XPOSITION, &posx);
        TIFFGetField(tif, TIFFTAG_YPOSITION, &posy);
        int32_t rowsPerStripe(0);
        TIFFGetField(tif, TIFFTAG_ROWSPERSTRIP, &rowsPerStripe);
        TIFFDataType dt(TIFF_NOTYPE);
        TIFFGetField(tif, TIFFTAG_DATATYPE, &dt);
        short ph(0);
        TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &ph);
        short YCbCrSubsampling[2] = { 2,2 };
        TIFFGetField(tif, TIFFTAG_YCBCRSUBSAMPLING, &YCbCrSubsampling[0], &YCbCrSubsampling[0]);
        int tiled = TIFFIsTiled(tif);
        int a = 0;
        std::cout << "Directory: " << dir << "------------" << std::endl;
        std::cout << "Image size: " << width << " x " << height << std::endl;
        std::cout << "Resolution: " << resx << " , " << resy << std::endl;
        std::cout << "Tiled: " << tiled << std::endl;
    }
    TIFFClose(tif);
}