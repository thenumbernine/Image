#pragma once

#include "Image/IO.h"

namespace Image {

struct PPM_IO : public IO {
	virtual ~PPM_IO();
	virtual std::string name();
	virtual bool supportsExtension(std::string extension);
	virtual std::shared_ptr<IImage> read(std::string filename);
	virtual void write(std::string filename, std::shared_ptr<const IImage> img);
};

extern Common::Singleton<PPM_IO> ppmIO;

};

