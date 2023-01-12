#pragma once

#include "Image/IO.h"
#include "Image/IImage.h"

namespace Image {

/**
 * holding info and access to all loaders
 * currently through the static members of the ImageLoader interface
 */
class System {
public:
	std::vector<IO*> ios;

	System();

	//access to loading an image from our chain:
	std::shared_ptr<IImage> read(std::string const & filename);
	
	//save an image
	void write(std::string const & filename, std::shared_ptr<IImage const> image);
};

}
