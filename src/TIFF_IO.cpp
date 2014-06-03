#ifndef WIN32
#include <stdlib.h>

#include <tiff.h>
#include <tiffio.h>

#include "Common/Exception.h"

#include "Image/IO.h"

#ifdef WIN32
#define strcasecmp _stricmp
#endif

using namespace Common;

namespace Image {

struct TIFF_IO : public IO {
	virtual ~TIFF_IO(){}
	virtual const char *name(void) { return "TIFF_IO"; }
	virtual bool supportsExt(const char *fileExt);
	virtual IImage *load(const char *filename);
	virtual void save(const IImage *img, const char *filename);
};

using namespace std;

bool TIFF_IO::supportsExt(const char *fileExt) {
	return !strcasecmp(fileExt, "tif")
		|| !strcasecmp(fileExt, "tiff");
}

IImage *TIFF_IO::load(const char *filename) {
	TIFF *in = NULL;
	unsigned char *imgdata = NULL;
	IImage *img = NULL;
	
	try {
		if (!(in = TIFFOpen(filename, "r"))) throw Exception() << " couldn't open file " << filename;

		uint32 width = 0;
		uint32 height = 0;
		uint32 bytespp = 0;
		TIFFGetField(in, TIFFTAG_IMAGEWIDTH, &width);
		TIFFGetField(in, TIFFTAG_IMAGELENGTH, &height);
		TIFFGetField(in, TIFFTAG_SAMPLESPERPIXEL, &bytespp);
		
		//alloc 4 bytes per pixel in the img data because tiff decodes to rgba, no questions asked
		//from there we can decide whether we need the alpha component
		imgdata = new unsigned char[height * width * 4];
		
		if (!TIFFReadRGBAImageOriented(in, width, height, (uint32*)imgdata, ORIENTATION_TOPLEFT, 0)) {
			throw Exception() << " failed to read the image into a RGBA format";
		}

		if (bytespp == 3) {
			int	pixel_count = width * height;
			unsigned char *src = imgdata;
			unsigned char *dst = imgdata;
			while( pixel_count > 0 ) {
				*(dst++) = *(src++);
				*(dst++) = *(src++);
				*(dst++) = *(src++);
				src++;
				pixel_count--;
			}
		}
		//img's existence signifies that we've made it
		img = new Image(Tensor::Vector<int,2>(width, height), imgdata, bytespp);
	} catch (const exception &t) {
		//finally
		if (in) TIFFClose(in);
		//all else
		delete[] imgdata;
		throw Exception() << "TIFF_IO::load(" << filename << ") error: " << t.what();
	}
	
	if (in) TIFFClose(in);
	assert(img);
	return img;
}

void TIFF_IO::save(const IImage *img, const char *filename) {
	throw Exception() << "not implemented yet";
}

Singleton<TIFF_IO> tiffIO;

};
#endif
