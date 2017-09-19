#pragma once

#include "Image/IO.h"

#define __CINT__
extern "C" {
#include <fitsio.h>
}

namespace Image {

template<typename Type> struct fitsType;
template<> struct fitsType<char> { enum { value = TBYTE }; };
template<> struct fitsType<short> { enum { value = TSHORT}; };
template<> struct fitsType<int> { enum { value = TLONG}; };
template<> struct fitsType<float> { enum { value = TFLOAT}; };
template<> struct fitsType<double> { enum { value = TDOUBLE}; };

template<typename Type> struct fitsImage;
template<> struct fitsImage<char> { enum { value = BYTE_IMG }; };
template<> struct fitsImage<short> { enum { value = SHORT_IMG }; };
template<> struct fitsImage<int> { enum { value = LONG_IMG }; };
template<> struct fitsImage<float> { enum { value = FLOAT_IMG }; };
template<> struct fitsImage<double> { enum { value = DOUBLE_IMG }; };

struct FITS_IO : public IO {
	virtual ~FITS_IO();
	virtual std::string name();
	virtual bool supportsExtension(const std::string& extension);
	virtual std::shared_ptr<IImage> read(const std::string& filename);
	
	/*
	Default write operation that is hooked up to the system.
	This is what is called when a write operation is invoked with a filename extension ".fits".
	Only checks a fixed number of types: char, short, int, float, double for 1, 2, or 3 channels.
	*/
	virtual void write(const std::string& filename, std::shared_ptr<const IImage> img);

protected:
	/*
	Used by write 
	*/
	template<typename T>
	bool checkSaveType(const std::string& filename, std::shared_ptr<const IImage> img) {
		std::shared_ptr<const ImageType<T>> img_ = std::dynamic_pointer_cast<const ImageType<T>>(img);
		if (img_) {
			writeType(filename, img, fitsType<T>::value, fitsImage<T>::value, img->getPlanes());
			return true;
		}
		return false;
	}

public:

	/*
	Underlying write function that the other functions call.
	*/
	virtual void writeType(const std::string& filename, std::shared_ptr<const IImage> img, int imgType, int bitPixType, int dim);
};

}
