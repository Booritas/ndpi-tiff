#include <gtest/gtest.h>
#include "tiff.h"
#include "tiffio.h"

// Demonstrate some basic assertions.
TEST(NDPITest, openFile) {
	std::string path = R"(d:\Projects\slideio\images\hamamatsu\2017-02-27 15.29.08.ndpi)";
    TIFF* tif = TIFFOpen(path.c_str(), "r");
    ASSERT_TRUE(tif != NULL);
    int dircount = 0;
    do {
        dircount++;

    } while (TIFFReadDirectory(tif));
    TIFFClose(tif);
}