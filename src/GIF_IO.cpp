#if IMAGE_SUPPORTS_GIF
#include "Image/GIF_IO.h"
#include "Common/Exception.h"
#include "Common/Finally.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>	//open()
#include <unistd.h>	//close()
#include <gif_lib.h>

#if PLATFORM_MSVC
#define strcasecmp _stricmp
#endif

namespace Image {

GIF_IO::~GIF_IO() {}

std::string GIF_IO::name() { return "GIF_IO"; }

bool GIF_IO::supportsExtension(const std::string& extension) {
	return !strcasecmp(extension.c_str(), "gif");
}

std::shared_ptr<IImage> GIF_IO::read(const std::string& filename) {		
	try {

		//using: https://gist.github.com/suzumura-ss/a5e922994513e44226d33c3a0c2c60d1

		int err = 0;
		GifFileType * gifFile = DGifOpenFileName(filename.c_str(), &err);
		//if (err != D_GIF_SUCCEEDED) {
		if (!gifFile) {
			throw Common::Exception() << "DGifOpenFileName failed with error " << err;
		}
		
		Common::Finally fileFinally([&](){ DGifCloseFile(gifFile, &err); });

		// what does this even do?
		if (DGifSlurp(gifFile) == GIF_ERROR) {
			throw Common::Exception() << "DGifSlurp failed with error " << gifFile->Error;
		}

		ColorMapObject const * const commonMap = gifFile->SColorMap;

//		for (int i = 0; i < gifFile->ImageCount; ++i) {
			int const i = 0;	// TODO how to change this image API to handle multiple images?
			SavedImage const & saved = gifFile->SavedImages[i];
			GifImageDesc const & desc = saved.ImageDesc;
			ColorMapObject const * const colorMap = desc.ColorMap ? desc.ColorMap : commonMap;
//			std::cout << "[" << i << "] "
//				<< desc.Width << "x" << desc.Height << "+" << desc.Left << "," << desc.Top
//				<< ", has local colorMap: " << (desc.ColorMap ? "Yes" : "No") << std::endl;

			int const imgbpp = 3;
			std::vector<unsigned char> imgdata(desc.Width * desc.Height * imgbpp);

			for (int v = 0; v < desc.Height; ++v) {
				for (int u = 0; u < desc.Width; ++u) {
					int c = saved.RasterBits[v * desc.Width + u];
//					printf(" %02X", c);
					if (colorMap) {
						GifColorType rgb = colorMap->Colors[c];
//						ss << " [" << (int)rgb.Red << "," << (int)rgb.Green << "," << (int)rgb.Blue << "]";
						int dstindex = imgbpp * (u + desc.Width * v);
						imgdata[0 + dstindex] = rgb.Red;
						imgdata[1 + dstindex] = rgb.Green;
						imgdata[2 + dstindex] = rgb.Blue;
					} else {
						throw Common::Exception() << "Can't decode this gif";	//truecolor gif? greyscale? no color map...
					}
				}
//				std::cout << ":" << ss.str() << std::endl;
			}
			
			return std::make_shared<Image>(Tensor::int2(desc.Width, desc.Height), &imgdata[0], imgbpp);
//		}
	} catch (const std::exception &t) {
		throw Common::Exception() << "GIF_IO::read(" << filename << ") error: " << t.what();
	}
}

void GIF_IO::write(const std::string& filename, std::shared_ptr<const IImage> img) {
	// TODO saving ... this requires discretizing palette, which there are a few algos for ...
}

}

#endif	//IMAGE_SUPPORTS_GIF
