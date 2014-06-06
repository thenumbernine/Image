#if defined(SUPPORT_TIFF)
#include "Image/IO.h"
#include "Common/Exception.h"
#include "Common/Finally.h"
#include <tiff.h>
#include <tiffio.h>
#include <stdlib.h>
#include <vector>

#ifdef WIN32
#define strcasecmp _stricmp
#endif

using namespace Common;

namespace Image {

struct TIFF_IO : public IO {
	virtual ~TIFF_IO(){}
	virtual std::string name(void) { return "TIFF_IO"; }
	virtual bool supportsExtension(std::string extension);
	virtual std::shared_ptr<IImage> read(std::string filename);
	virtual void write(std::string filename, std::shared_ptr<const IImage> img);
};

using namespace std;

bool TIFF_IO::supportsExtension(std::string extension) {
	return !strcasecmp(extension.c_str(), "tif")
		|| !strcasecmp(extension.c_str(), "tiff");
}

std::shared_ptr<IImage> TIFF_IO::read(std::string filename) {
	try {
		TIFF *tiff = TIFFOpen(filename.c_str(), "r");
		if (!tiff) throw Exception() << " couldn't open file " << filename;
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
			throw Exception() << " failed to read the image into a RGBA format";
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
	} catch (const exception &t) {
		throw Exception() << "TIFF_IO::read(" << filename << ") error: " << t.what();
	}	
}

void TIFF_IO::write(std::string filename, std::shared_ptr<const IImage> img) {
	throw Exception() << "not implemented yet";
}

Singleton<TIFF_IO> tiffIO;

};
#endif	//SUPPORT_TIFF

