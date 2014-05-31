#include "Common/Exception.h"
#include "Image/IO.h"

#include "TensorMath/Vector.h"

#define __CINT__
extern "C" {
#include <fitsio.h>
}

// http://www.mitchr.me/SS/exampleCode/cfitsio/fits2tga.c.html
// http://heasarc.gsfc.nasa.gov/fitsio/c/c_user/node40.html#ffgidt

#ifdef WIN32
#define strcasecmp _stricmp
#endif

using namespace Common;

namespace Image {

struct FITS_IO : public IO {
	virtual ~FITS_IO() {}
	virtual const char *name() { return "FITS_IO"; }
	virtual bool supportsExt(const char *fileExt);
	virtual IImage *load(const char *filename);
	virtual void save(const IImage *img, const char *filename);
};

using namespace std;

bool FITS_IO::supportsExt(const char *fileExt) {
	return !strcasecmp(fileExt, "fits");
}

IImage *FITS_IO::load(const char *filename) {
	fitsfile *fitsFilePtr;
	int status = 0;
	ffopen(&fitsFilePtr, filename, READONLY, &status);
	if (status) throw Exception() << "ffopen failed with " << status;
	
	int bytesPerPixel = 0;
	int bitPixType;
	ffgidt(fitsFilePtr, &bitPixType, &status);
	if (status) throw Exception() << "ffgidt failed with " << status;

	int imgType = 0;
	switch (bitPixType) {
	case BYTE_IMG:
		bytesPerPixel = 1;
		imgType = TBYTE;
		break;
	case SHORT_IMG:
		bytesPerPixel = 2;
		imgType = TSHORT;
		break;
	case LONG_IMG:
		bytesPerPixel = 4;
		imgType = TLONG;
		break;
	case FLOAT_IMG:
		bytesPerPixel = 4;
		imgType = TFLOAT;
		break;
	case DOUBLE_IMG:
		bytesPerPixel = 8;
		imgType = TDOUBLE;
		break;
	default:
		throw Exception() << "image is an unsupported FITS type " << bitPixType;
	}
	
	int dim;
	ffgidm(fitsFilePtr, &dim, &status);
	if (status) throw Exception() << "ffgidm failed with " << status;
	if (dim != 3) throw Exception() << "image is an unsupported dimension " << dim;
	
	long int fpixel[3];
	for (int i = 0; i < dim; i++) {
		fpixel[i] = 1;
	}
	
	long int sizes[3];
	ffgisz(fitsFilePtr, 3, sizes, &status);
	if (status) throw Exception() << "ffgisz failed with " << status;
	int width = sizes[0];
	int height = sizes[1];
	Vector<int,2> size(width,height);
	int channels = sizes[2];
	
	int numPixels = width * height * channels;
	unsigned char *data = new unsigned char[numPixels * bytesPerPixel];
	ffgpxv(fitsFilePtr, imgType, fpixel, numPixels, NULL, data, NULL, &status);
	if (status) throw Exception() << "ffgpxv failed with " << status;
	
	IImage *img;
	switch (bitPixType) {
	case BYTE_IMG:
		switch (channels) {
		case 1: img = new ImageType<char>(size); break;
		case 2: img = new ImageType<Vector<char,2>>(size); break;
		case 3: img = new ImageType<Vector<char,3>>(size); break;
		default:
			throw Exception() << "unsupported channels for byte type: " << channels;
		}
		break;
	case SHORT_IMG:
		switch (channels) {
		case 1: img = new ImageType<short>(size); break;
		case 2: img = new ImageType<Vector<short,2>>(size); break;
		case 3: img = new ImageType<Vector<short,3>>(size); break;
		default:
			throw Exception() << "unsupported channels for short type: " << channels;
		}
		break;
	case LONG_IMG:
		switch (channels) {
		case 1: img = new ImageType<int>(size); break;
		case 2: img = new ImageType<Vector<int,2>>(size); break;
		case 3: img = new ImageType<Vector<int,3>>(size); break;
		default:
			throw Exception() << "unsupported channels for int type: " << channels;
		}
		break;
	case FLOAT_IMG:
		switch (channels) {
		case 1: img = new ImageType<float>(size); break;
		case 2: img = new ImageType<Vector<float,2>>(size); break;
		case 3: img = new ImageType<Vector<float,3>>(size); break;
		default:
			throw Exception() << "unsupported channels for float type: " << channels;
		}
		break;
	case DOUBLE_IMG:
		switch (channels) {
		case 1: img = new ImageType<double>(size); break;
		case 2: img = new ImageType<Vector<double,2>>(size); break;
		case 3: img = new ImageType<Vector<double,3>>(size); break;
		default:
			throw Exception() << "unsupported channels for double type: " << channels;
		}
		break;
	default:
		throw Exception() << "uncoded read type " << bitPixType;
	}
	memcpy(img->getData(), data, numPixels * bytesPerPixel);
	ffclos(fitsFilePtr, &status);
	if (status) throw Exception() << "ffclos failed with " << status;
	
	return img;
}

static void saveType(const IImage *img, const char *filename, int imgType, int bitPixType, int DIM) {

	FILE *file = fopen(filename, "r");
	if (file) {
		fclose(file);
		remove(filename);
	}

	int status = 0;
	
	fitsfile *fitsFilePtr;
	ffinit(&fitsFilePtr, filename, &status);
	if (status) throw Exception() << "ffinit failed with " << status;

	long sizes[3] = {img->getSize()(0), img->getSize()(1), DIM};
	
	status = ffphps(fitsFilePtr, bitPixType, 3, sizes, &status);
	if (status) throw Exception() << "ffphps failed with " << status;
	
	long int firstpix[3];
	for (int i = 0; i < 3; i++) {
		firstpix[i] = 1;
	}
	
	int numPixels = img->getSize()(0) * img->getSize()(1) * DIM;
	
	ffppx(fitsFilePtr, imgType, firstpix, numPixels, (void*)img->getData(), &status);
	if (status) throw Exception() << "ffppx failed with " << status;
	
	ffclos(fitsFilePtr, &status);
	if (status) throw Exception() << "ffclos failed with " << status;
}

/*
but all we have is channel size, not type ...
time to dynamic-cast and find the right type ...
*/
void FITS_IO::save(const IImage *img, const char *filename) {
#define CHECK_SAVE_TYPE(T, imgType, bitPixType, DIM)	\
	{	\
		const ImageType<T> *img_ = dynamic_cast<const ImageType<T>*>(img);	\
		if (img_) {	\
			saveType(img, filename, imgType, bitPixType, DIM);	\
			return;	\
		}	\
	}
#define COMMA ,
	CHECK_SAVE_TYPE(char, TBYTE, BYTE_IMG, 1)
	CHECK_SAVE_TYPE(Vector<char COMMA 2>, TBYTE, BYTE_IMG, 2)
	CHECK_SAVE_TYPE(Vector<char COMMA 3>, TBYTE, BYTE_IMG, 3)
	CHECK_SAVE_TYPE(short, TSHORT, SHORT_IMG, 1)
	CHECK_SAVE_TYPE(Vector<short COMMA 2>, TSHORT, SHORT_IMG, 2)
	CHECK_SAVE_TYPE(Vector<short COMMA 3>, TSHORT, SHORT_IMG, 3)
	CHECK_SAVE_TYPE(int, TLONG, LONG_IMG, 1)
	CHECK_SAVE_TYPE(Vector<int COMMA 2>, TLONG, LONG_IMG, 2)
	CHECK_SAVE_TYPE(Vector<int COMMA 3>, TLONG, LONG_IMG, 3)
	CHECK_SAVE_TYPE(float, TFLOAT, FLOAT_IMG, 1)
	CHECK_SAVE_TYPE(Vector<float COMMA 2>, TFLOAT, FLOAT_IMG, 2)
	CHECK_SAVE_TYPE(Vector<float COMMA 3>, TFLOAT, FLOAT_IMG, 3)
	CHECK_SAVE_TYPE(double, TDOUBLE, DOUBLE_IMG, 1)
	CHECK_SAVE_TYPE(Vector<double COMMA 2>, TDOUBLE, DOUBLE_IMG, 2)
	CHECK_SAVE_TYPE(Vector<double COMMA 3>, TDOUBLE, DOUBLE_IMG, 3)
	
	throw Exception() << "failed to find RTTI for image";
}

Singleton<FITS_IO> fitsIO;

};
