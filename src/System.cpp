#include "Image/System.h"
#include "Common/File.h"
#include "Common/Exception.h"

namespace Image {

System::System()
 : baseIO(NULL)
{
}

std::shared_ptr<IImage> System::read(const std::string& filename) {
	std::string ext = Common::File::getExtension(filename);

	try {
		for (IO *io = baseIO; io; io = io->getNext()) {
			if (!io->supportsExtension(ext)) continue;
			return io->read(filename);	//throws if things go wrong
		}
		throw Common::Exception() << "failed to find an appropriate reader for extension " << ext;
	} catch (const std::exception &t) {
		throw Common::Exception() << "Image::System::load(" << filename << ") error: " << t.what();
	}
}

void System::write(const std::string& filename, std::shared_ptr<const IImage> image) {
	std::string ext = Common::File::getExtension(filename);

	try {
		for (IO *io = baseIO; io; io = io->getNext()) {
			if (!io->supportsExtension(ext)) continue;	//gonna use this to double for 'canSave'
			io->write(filename, image);	//throws if things go wrong
			return;
		}
		throw Common::Exception() << "failed to find an appropriate writer for extension " << ext;
	} catch (const std::exception &t) {
		throw Common::Exception() << "Image::System::save(" << filename << ") error: " << t.what();
	}
}

}
