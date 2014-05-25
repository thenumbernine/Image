#include "Image/IO.h"
#include "Image/System.h"

namespace Image {

IO::IO() {
	//LOCK ON!
	next = sys->baseIO;
	sys->baseIO = this;
}

IO *IO::getNext() { return next; }

};
