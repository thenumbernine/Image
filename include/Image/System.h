#pragma once

#include "Image/IO.h"
#include "Image/Image.h"
#include "Common/Singleton.h"

namespace Image {

/**
 * holding info and access to all loaders
 * currently through the static members of the ImageLoader interface
 */
class System {
public:
	//the base of the image loader chain
	//TODO - only allow this to be accessible by new image loader classes
	IO *baseIO;

	System() : baseIO(NULL) {}

	//access to loading an image from our chain:
	IImage *read(std::string filename);
	
	//save an image
	void write(std::string filename, const IImage *image);
};
extern Common::Singleton<System> sys;

};

