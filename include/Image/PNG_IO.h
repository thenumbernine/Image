#pragma once

#include "Image/IO.h"

namespace Image {

struct PNG_IO : public IO {
	virtual ~PNG_IO();
	virtual std::string name();
	virtual bool supportsExtension(const std::string& extension);
	virtual std::shared_ptr<IImage> read(const std::string& filename);
	virtual void write(const std::string& filename, std::shared_ptr<const IImage> img);
};

}
