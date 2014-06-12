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
	virtual bool supportsExtension(std::string extension);
	virtual std::shared_ptr<IImage> read(std::string filename);
	
	/*
	Default write operation that is hooked up to the system.
	This is what is called when a write operation is invoked with a filename extension ".fits".
	Only checks a fixed number of types: char, short, int, float, double for 1, 2, or 3 channels.
	*/
	virtual void write(std::string filename, std::shared_ptr<const IImage> img);

	/*
	Templated write function
	*/
	template<typename Type, int rank>
	void writeImage(std::string filename, std::shared_ptr<const ImageType<Tensor::Vector<Type, rank>>> img) {
		writeType(filename, img, fitsType<Type>::value, fitsImage<Type>::value, rank);
	}

	/*
	Underlying write function that the other functions call.
	*/
	virtual void writeType(std::string filename, std::shared_ptr<const IImage> img, int imgType, int bitPixType, int DIM);
};

extern Common::Singleton<FITS_IO> fitsIO;

};


