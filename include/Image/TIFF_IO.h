#pragma once

#include "Image/IO.h"

namespace Image {

struct TIFF_IO : public IO {
	virtual ~TIFF_IO();
	virtual std::string name();
	virtual bool supportsExtension(std::string extension);
	virtual std::shared_ptr<IImage> read(std::string filename);
	virtual void write(std::string filename, std::shared_ptr<const IImage> img);
};

extern Common::Singleton<TIFF_IO> tiffIO;

};


