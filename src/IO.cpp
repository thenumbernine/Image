#include "Image/IO.h"
#include "Image/Image.h"

namespace Image {

IO::IO() {
	//LOCK ON!
	next = system->baseIO;
	system->baseIO = this;
}

IO *IO::getNext() { return next; }

}
