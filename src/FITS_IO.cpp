#if defined(SUPPORT_FITS)
#include "Image/FITS_IO.h"
#include "Common/Exception.h"
#include "Common/File.h"
#include "Tensor/Vector.h"
#include <vector>

// http://www.mitchr.me/SS/exampleCode/cfitsio/fits2tga.c.html
// http://heasarc.gsfc.nasa.gov/fitsio/c/c_user/node40.html#ffgidt

#ifdef WIN32
#define strcasecmp _stricmp
#endif

namespace Image {

FITS_IO::~FITS_IO() {}

std::string FITS_IO::name() { return "FITS_IO"; }

bool FITS_IO::supportsExtension(std::string extension) {
	return !strcasecmp(extension.c_str(), "fits");
}

std::shared_ptr<IImage> FITS_IO::read(std::string filename) {
	fitsfile *fitsFilePtr;
	int status = 0;
	ffopen(&fitsFilePtr, filename.c_str(), READONLY, &status);
	if (status) throw Common::Exception() << "ffopen failed with " << status;
	
	int bytesPerPixel = 0;
	int bitPixType;
	ffgidt(fitsFilePtr, &bitPixType, &status);
	if (status) throw Common::Exception() << "ffgidt failed with " << status;

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
		throw Common::Exception() << "image is an unsupported FITS type " << bitPixType;
	}
	
	int dim;
	ffgidm(fitsFilePtr, &dim, &status);
	if (status) throw Common::Exception() << "ffgidm failed with " << status;
	if (dim != 3) throw Common::Exception() << "image is an unsupported dimension " << dim;
	
	long int fpixel[3];
	for (int i = 0; i < dim; i++) {
		fpixel[i] = 1;
	}
	
	long int sizes[3];
	ffgisz(fitsFilePtr, 3, sizes, &status);
	if (status) throw Common::Exception() << "ffgisz failed with " << status;
	int width = sizes[0];
	int height = sizes[1];
	Tensor::Vector<int,2> size(width,height);
	int channels = sizes[2];
	
	int numPixels = width * height * channels;
	std::vector<unsigned char> data(numPixels * bytesPerPixel);
	ffgpxv(fitsFilePtr, imgType, fpixel, numPixels, NULL, &data[0], NULL, &status);
	if (status) throw Common::Exception() << "ffgpxv failed with " << status;
	
	std::shared_ptr<IImage> img;
	switch (bitPixType) {
	case BYTE_IMG:
		switch (channels) {
		case 1: img = std::make_shared<ImageType<char>>(size); break;
		case 2: img = std::make_shared<ImageType<Tensor::Vector<char,2>>>(size); break;
		case 3: img = std::make_shared<ImageType<Tensor::Vector<char,3>>>(size); break;
		default:
			throw Common::Exception() << "unsupported channels for byte type: " << channels;
		}
		break;
	case SHORT_IMG:
		switch (channels) {
		case 1: img = std::make_shared<ImageType<short>>(size); break;
		case 2: img = std::make_shared<ImageType<Tensor::Vector<short,2>>>(size); break;
		case 3: img = std::make_shared<ImageType<Tensor::Vector<short,3>>>(size); break;
		default:
			throw Common::Exception() << "unsupported channels for short type: " << channels;
		}
		break;
	case LONG_IMG:
		switch (channels) {
		case 1: img = std::make_shared<ImageType<int>>(size); break;
		case 2: img = std::make_shared<ImageType<Tensor::Vector<int,2>>>(size); break;
		case 3: img = std::make_shared<ImageType<Tensor::Vector<int,3>>>(size); break;
		default:
			throw Common::Exception() << "unsupported channels for int type: " << channels;
		}
		break;
	case FLOAT_IMG:
		switch (channels) {
		case 1: img = std::make_shared<ImageType<float>>(size); break;
		case 2: img = std::make_shared<ImageType<Tensor::Vector<float,2>>>(size); break;
		case 3: img = std::make_shared<ImageType<Tensor::Vector<float,3>>>(size); break;
		default:
			throw Common::Exception() << "unsupported channels for float type: " << channels;
		}
		break;
	case DOUBLE_IMG:
		switch (channels) {
		case 1: img = std::make_shared<ImageType<double>>(size); break;
		case 2: img = std::make_shared<ImageType<Tensor::Vector<double,2>>>(size); break;
		case 3: img = std::make_shared<ImageType<Tensor::Vector<double,3>>>(size); break;
		default:
			throw Common::Exception() << "unsupported channels for double type: " << channels;
		}
		break;
	default:
		throw Common::Exception() << "uncoded read type " << bitPixType;
	}
	memcpy(img->getData(), &data[0], numPixels * bytesPerPixel);
	ffclos(fitsFilePtr, &status);
	if (status) throw Common::Exception() << "ffclos failed with " << status;
	return img;
}

void FITS_IO::writeType(std::string filename, std::shared_ptr<const IImage> img, int imgType, int bitPixType, int DIM) {

	if (Common::File::exists(filename)) {
		Common::File::remove(filename);
	}
	
	int status = 0;
	
	fitsfile *fitsFilePtr;
	ffinit(&fitsFilePtr, filename.c_str(), &status);
	if (status) throw Common::Exception() << "ffinit failed with " << status;

	long sizes[3] = {img->getSize()(0), img->getSize()(1), DIM};
	
	status = ffphps(fitsFilePtr, bitPixType, 3, sizes, &status);
	if (status) throw Common::Exception() << "ffphps failed with " << status;
	
	long int firstpix[3];
	for (int i = 0; i < 3; i++) {
		firstpix[i] = 1;
	}
	
	int numPixels = img->getSize()(0) * img->getSize()(1) * DIM;
	
	ffppx(fitsFilePtr, imgType, firstpix, numPixels, (void*)img->getData(), &status);
	if (status) throw Common::Exception() << "ffppx failed with " << status;
	
	ffclos(fitsFilePtr, &status);
	if (status) throw Common::Exception() << "ffclos failed with " << status;
}

/*
but all we have is channel size, not type ...
time to dynamic-cast and find the right type ...
*/
void FITS_IO::write(std::string filename, std::shared_ptr<const IImage> img) {
#define CHECK_SAVE_TYPE(T, imgType, bitPixType, DIM)	\
	{	\
		std::shared_ptr<const ImageType<T>> img_ = std::dynamic_pointer_cast<const ImageType<T>>(img);	\
		if (img_) {	\
			writeType(filename, img, imgType, bitPixType, DIM);	\
			return;	\
		}	\
	}
#define COMMA ,
	CHECK_SAVE_TYPE(char, TBYTE, BYTE_IMG, 1)
	CHECK_SAVE_TYPE(Tensor::Vector<char COMMA 2>, TBYTE, BYTE_IMG, 2)
	CHECK_SAVE_TYPE(Tensor::Vector<char COMMA 3>, TBYTE, BYTE_IMG, 3)
	CHECK_SAVE_TYPE(short, TSHORT, SHORT_IMG, 1)
	CHECK_SAVE_TYPE(Tensor::Vector<short COMMA 2>, TSHORT, SHORT_IMG, 2)
	CHECK_SAVE_TYPE(Tensor::Vector<short COMMA 3>, TSHORT, SHORT_IMG, 3)
	CHECK_SAVE_TYPE(int, TLONG, LONG_IMG, 1)
	CHECK_SAVE_TYPE(Tensor::Vector<int COMMA 2>, TLONG, LONG_IMG, 2)
	CHECK_SAVE_TYPE(Tensor::Vector<int COMMA 3>, TLONG, LONG_IMG, 3)
	CHECK_SAVE_TYPE(float, TFLOAT, FLOAT_IMG, 1)
	CHECK_SAVE_TYPE(Tensor::Vector<float COMMA 2>, TFLOAT, FLOAT_IMG, 2)
	CHECK_SAVE_TYPE(Tensor::Vector<float COMMA 3>, TFLOAT, FLOAT_IMG, 3)
	CHECK_SAVE_TYPE(double, TDOUBLE, DOUBLE_IMG, 1)
	CHECK_SAVE_TYPE(Tensor::Vector<double COMMA 2>, TDOUBLE, DOUBLE_IMG, 2)
	CHECK_SAVE_TYPE(Tensor::Vector<double COMMA 3>, TDOUBLE, DOUBLE_IMG, 3)

	throw Common::Exception() << "failed to find RTTI for image";
}

Common::Singleton<FITS_IO> fitsIO;

};
#endif	//SUPPORT_FITS

