#if defined(Image_supports_tiff)
#include "Image/TIFF_IO.h"
#include "Common/Exception.h"
#include "Common/Finally.h"
#include <tiff.h>
#include <tiffio.h>
#include <stdlib.h>
#include <vector>

#ifdef PLATFORM_msvc
#define strcasecmp _stricmp
#endif

namespace Image {

TIFF_IO::~TIFF_IO() {}
	
std::string TIFF_IO::name(void) { return "TIFF_IO"; }

bool TIFF_IO::supportsExtension(const std::string& extension) {
	return !strcasecmp(extension.c_str(), "tif")
		|| !strcasecmp(extension.c_str(), "tiff");
}

std::shared_ptr<IImage> TIFF_IO::read(const std::string& filename) {
	try {
		TIFF *tiff = TIFFOpen(filename.c_str(), "r");
		if (!tiff) throw Common::Exception() << " couldn't open file " << filename;
		Common::Finally tiffFinally([&](){ TIFFClose(tiff); });

		uint32 width = 0;
		uint32 height = 0;
		uint32 bytespp = 0;
		TIFFGetField(tiff, TIFFTAG_IMAGEWIDTH, &width);
		TIFFGetField(tiff, TIFFTAG_IMAGELENGTH, &height);
		TIFFGetField(tiff, TIFFTAG_SAMPLESPERPIXEL, &bytespp);
		
		//alloc 4 bytes per pixel in the img data because tiff decodes to rgba, no questions asked
		//from there we can decide whether we need the alpha component
		std::vector<unsigned char> imgdata(height * width * 4);
		
		if (!TIFFReadRGBAImageOriented(tiff, width, height, (uint32*)&imgdata[0], ORIENTATION_TOPLEFT, 0)) {
			throw Common::Exception() << " failed to read the image into a RGBA format";
		}

		if (bytespp == 3) {
			int	pixel_count = width * height;
			unsigned char *src = &imgdata[0];
			unsigned char *dst = &imgdata[0];
			while( pixel_count > 0 ) {
				*(dst++) = *(src++);
				*(dst++) = *(src++);
				*(dst++) = *(src++);
				src++;
				pixel_count--;
			}
		}
		//img's existence signifies that we've made it
		return std::make_shared<Image>(Tensor::Vector<int,2>(width, height), &imgdata[0], bytespp);
	} catch (const std::exception &t) {
		throw Common::Exception() << "TIFF_IO::read(" << filename << ") error: " << t.what();
	}	
}

void TIFF_IO::write(const std::string& filename, std::shared_ptr<const IImage> img) {
	throw Common::Exception() << "not implemented yet";
}

}
#endif	//Image_supports_tiff
