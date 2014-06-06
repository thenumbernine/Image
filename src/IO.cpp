#include "Image/IO.h"
#include "Image/System.h"

namespace Image {

IO::IO() {
	//LOCK ON!
	next = system->baseIO;
	system->baseIO = this;
}

IO *IO::getNext() { return next; }

};
