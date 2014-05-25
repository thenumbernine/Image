#include "Image/System.h"

using namespace std;

namespace Image {

static const char *extForFilename(const char *filename) {
	const char *ext = filename;
	while (*ext) {ext++;}	//seek to end
	while (ext > filename) {
		ext--;
		if (*ext == '.') {
			ext++;
			break;
		}
	}
	return ext;
}

Image *System::load(const char *filename) {
	const char *ext = extForFilename(filename);

	try {
		for (IO *io = baseIO; io; io = io->getNext()) {
			if (!io->supportsExt(ext)) continue;
			return io->load(filename);	//throws if things go wrong
		}
		throw Exception() << "failed to find an appropriate reader for extension " << ext;
	} catch (const exception &t) {
		throw Exception() << "img::System::load(" << filename << ") error: " << t.what();
	}
}

void System::save(const Image *img, const char *filename) {
	const char *ext = extForFilename(filename);

	try {
		for (IO *io = baseIO; io; io = io->getNext()) {
			if (!io->supportsExt(ext)) continue;	//gonna use this to double for 'canSave'
			io->save(img, filename);	//throws if things go wrong
			return;
		}
		throw Exception() << "failed to find an appropriate reader for extension " << ext;
	} catch (const exception &t) {
		throw Exception() << "img::System::save(" << filename << ") error: " << t.what();
	}
}

Singleton<System> sys;

};
