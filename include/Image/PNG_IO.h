#pragma once

#include "Image/IO.h"

namespace Image {

struct PNG_IO : public IO {
	virtual ~PNG_IO();
	virtual std::string name();
	virtual bool supportsExtension(std::string extension);
	virtual std::shared_ptr<IImage> read(std::string filename);
	virtual void write(std::string filename, std::shared_ptr<const IImage> img);
};

extern Common::Singleton<PNG_IO> pngIO;

};


