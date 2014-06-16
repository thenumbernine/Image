#pragma once

#include "Common/Singleton.h"
#include "Common/Exception.h"
#include "Image/Image.h"
#include <string>

namespace Image {

/**
 * Abstract image loading interface.  create and register as you go.
 * Intertwined within it - through static methods - is access to the loader system
 */
class IO {
	friend class Common::Singleton<IO>;	//so only the singleton can instanciate this
	friend class System;
	
	//the image loader chain:
	IO *next;

	IO(const char &) { throw Common::Exception() << "no copy constructors for singletons!"; }

protected:
	//attach yerself to the chain!
	IO();
	
	IO *getNext();
	
public:

	//returns whether or not we can load the file, based on matching extension
	virtual bool supportsExtension(const std::string& extension) = 0;

	//return the name of the loader, for identification
	virtual std::string name() = 0;

	//returns the image in the file.
	//throws an exception if it fails
	//if it returns, you should be able to assert the image and its data exist
	virtual std::shared_ptr<IImage> read(const std::string& filename) = 0;
	
	//saves the image to the specified file
	//throws an exception if it fails
	virtual void write(const std::string& filename, std::shared_ptr<const IImage> img) = 0;
};

};

