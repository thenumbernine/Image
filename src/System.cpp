#include "Image/BMP_IO.h"
#include "Image/FITS_IO.h"
#include "Image/GIF_IO.h"
#include "Image/JPEG_IO.h"
#include "Image/PNG_IO.h"
#include "Image/PPM_IO.h"
#include "Image/TGA_IO.h"
#include "Image/TIFF_IO.h"

#include "Image/System.h"

#include "Common/File.h"
#include "Common/Exception.h"

#include <filesystem>

namespace Image {

#if IMAGE_SUPPORTS_BMP
Common::Singleton<BMP_IO> bmpIO;
#endif	//IMAGE_SUPPORTS_BMP

#if IMAGE_SUPPORTS_FITS
Common::Singleton<FITS_IO> fitsIO;
#endif	//IMAGE_SUPPORTS_FITS

#if IMAGE_SUPPORTS_GIF
Common::Singleton<GIF_IO> gifIO;
#endif	//IMAGE_SUPPORTS_GIF

#if IMAGE_SUPPORTS_JPEG
Common::Singleton<JPEG_IO> jpegIO;
#endif	//IMAGE_SUPPORTS_JPEG

#if IMAGE_SUPPORTS_PNG
Common::Singleton<PNG_IO> pngIO;
#endif	//IMAGE_SUPPORTS_PNG

#if IMAGE_SUPPORTS_PPM
Common::Singleton<PPM_IO> ppmIO;
#endif	//IMAGE_SUPPORTS_PPM

#if IMAGE_SUPPORTS_TGA
Common::Singleton<TGA_IO> tgaIO;
#endif	//IMAGE_SUPPORTS_TGA

#if IMAGE_SUPPORTS_TIFF
Common::Singleton<TIFF_IO> tiffIO;
#endif	//IMAGE_SUPPORTS_TIFF

System::System() {
#if IMAGE_SUPPORTS_BMP
	ios.push_back(&*bmpIO);
#endif
#if IMAGE_SUPPORTS_FITS
	ios.push_back(&*fitsIO);
#endif
#if IMAGE_SUPPORTS_GIF
	ios.push_back(&*gifIO);
#endif
#if IMAGE_SUPPORTS_JPEG
	ios.push_back(&*jpegIO);
#endif
#if IMAGE_SUPPORTS_PNG
	ios.push_back(&*pngIO);
#endif
#if IMAGE_SUPPORTS_PPM
	ios.push_back(&*ppmIO);
#endif
#if IMAGE_SUPPORTS_TGA
	ios.push_back(&*tgaIO);
#endif
#if IMAGE_SUPPORTS_TIFF
	ios.push_back(&*tiffIO);
#endif
}

std::shared_ptr<IImage> System::read(std::string const & filename) {
	std::string ext = std::filesystem::path(filename).extension();

	try {
		for (auto const & io : ios) {
			if (!io->supportsExtension(ext)) continue;
			return io->read(filename);	//throws if things go wrong
		}
		throw Common::Exception() << "failed to find an appropriate reader for extension " << ext;
	} catch (std::exception const & t) {
		throw Common::Exception() << "Image::System::load(" << filename << ") error: " << t.what();
	}
}

void System::write(std::string const & filename, std::shared_ptr<IImage const> image) {
	std::string ext = std::filesystem::path(filename).extension();

	try {
		for (auto const & io : ios) {
			if (!io->supportsExtension(ext)) continue;	//gonna use this to double for 'canSave'
			io->write(filename, image);	//throws if things go wrong
			return;
		}
		throw Common::Exception() << "failed to find an appropriate writer for extension " << ext;
	} catch (std::exception const & t) {
		throw Common::Exception() << "Image::System::save(" << filename << ") error: " << t.what();
	}
}

}
