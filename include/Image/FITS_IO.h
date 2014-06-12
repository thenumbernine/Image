#pragma once

#include "Image/IO.h"

namespace Image {

struct FITS_IO : public IO {
	virtual ~FITS_IO();
	virtual std::string name();
	virtual bool supportsExtension(std::string extension);
	virtual std::shared_ptr<IImage> read(std::string filename);
	virtual void write(std::string filename, std::shared_ptr<const IImage> img);
	virtual void writeType(std::string filename, std::shared_ptr<const IImage> img, int imgType, int bitPixType, int DIM);
};

extern Common::Singleton<FITS_IO> fitsIO;

};


