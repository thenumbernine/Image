#pragma once

#include "Image/IO.h"

namespace Image {

struct JPEG_IO : public IO {
	virtual ~JPEG_IO();
	virtual std::string name();
	virtual bool supportsExtension(const std::string& extension);
	virtual std::shared_ptr<IImage> read(const std::string& filename);
	virtual void write(const std::string& filename, std::shared_ptr<const IImage> img);

#ifndef WIN32
	//special-case for this loader
	//useful when you download a jpeg and dont want to write it to disk to read it again
	std::shared_ptr<IImage> readFromMemory(const char *buffer, size_t size);
#endif
};

}
