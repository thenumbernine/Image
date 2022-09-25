#if IMAGE_SUPPORTS_BMP
#include "Image/BMP_IO.h"
#include <fstream>
#include <vector>

#if PLATFORM_MSVC
#define strcasecmp _stricmp
#endif

namespace Image {

//everything except the 2-byte sig ...
//because everything ELSE is 4byte aligned...
//and its either this or figure out the 'packed' attribute ...
struct bitmapHeader_t {
	int fileSize;		//02:	//size of total file
	int zero;			//06:	//reserved / zero
	int imgOffset;		//0A:	//offset from file start to beginning of image data
	int extraSize;		//0E:	//size of header after the initial 14 == 0x0E bytes
	int width;			//12:
	int height;			//16:
	union {
		struct {
			short planes;		//1A:
			short bitsPerPixel;	//1C:
		};
		int h6;
	};
	int compression;	//1E:
	int imgSize;		//22:
	int horzRes;		//26:
	int vertRes;		//2A:
	int palColorsUsed;	//30:
	int palImportant;	//34:
};

BMP_IO::~BMP_IO() {}
	
std::string BMP_IO::name() { return "BMP_IO"; }
	
bool BMP_IO::supportsExtension(const std::string& extension) {
	return !strcasecmp(extension.c_str(), "bmp");
}
	
std::shared_ptr<IImage> BMP_IO::read(const std::string& filename) {

	//list out resources we have to free here:
	try {
		std::ifstream file(filename, std::ios::binary);
		if (!file) throw Common::Exception() << "failed to open file for reading";
		
		short sig;
		if (!file.read((char *)&sig, sizeof(sig))) throw Common::Exception() << "failed to read signature";
		if (sig != 0x4D42) throw Common::Exception() << "got a bad signature " << sig;
		
		bitmapHeader_t hdr;
		if (!file.read((char *)&hdr, sizeof(hdr))) throw Common::Exception() << "failed to read header";

		if (hdr.planes != 1) throw Common::Exception() << "got bad # planes:" << hdr.planes;
		if (hdr.bitsPerPixel != 24) throw Common::Exception() << "got an unhandled bitsPerPixel:" << hdr.bitsPerPixel;
		if (hdr.compression) throw Common::Exception() << "got an unhandled compression: " << hdr.compression;

		//now get our row skip:
		int stride = hdr.width * hdr.bitsPerPixel;
		stride = (stride + 7) >> 3;	//div 8, round up
		int padding = (stride ^ (stride << 1)) & 3;
		
		int ystart, yend, ystep;
		int height = hdr.height;
		if (height >= 0) {
			ystart = height-1;
			yend = -1;
			ystep = -1;
		} else {
			height = -height;
			ystart = 0;
			yend = height;
			ystep = 1;
		}
		
		int imgsize = stride * height;
		std::vector<char> imgdata(imgsize);
		
		file.seekg(hdr.imgOffset, std::ios::beg);

		//now read it through - getting rid of the row spacing
		int dummy;
		for (int y = ystart; y != yend; y += ystep) {
			//swap ys as we go =) bitmaps are upside-down
			char *dst = &imgdata[y * hdr.width * 3];
			//swap red and blue as we go =)
			for (int x = 0; x < hdr.width; x++, dst += 3) {
				for (int ch = 2; ch >= 0; --ch) {
					if (!file.get(dst[2])) throw Common::Exception() << "failed to read data";
				}
			}
			if (!file.read((char *)&dummy, padding)) throw Common::Exception() << "failed to read data";
		}
		
		//do this last so img == null if anything goes wrong
		return std::make_shared<Image>(
			Tensor::int2(hdr.width, height),	//size
			&imgdata[0],							//data
			hdr.bitsPerPixel >> 3);				//channels
	} catch (const std::exception &t) {
		//finally code ...
		//only for errors
		throw Common::Exception() << "BMP_IO::read("<<filename<<") error: " << t.what();
	}
}
	
void BMP_IO::write(const std::string& filename, std::shared_ptr<const IImage> img) {
	try {
		if (img->getBitsPerPixel() < 24) throw Common::Exception() << "don't support writing for " << img->getBitsPerPixel() << " bits per pixel yet";

		std::ofstream file(filename, std::ios::binary);
		if (!file) throw Common::Exception() << "failed to open file for writing";
		if (!file.write("BM",2)) throw Common::Exception() << "failed to write signature";
		
		bitmapHeader_t hdr;
		memset(&hdr, 0, sizeof(hdr));
		hdr.width = img->getSize()(0);
		hdr.height = img->getSize()(1);
		hdr.horzRes = hdr.width;
		hdr.vertRes = hdr.height;
		int pixelsWide = 3 * hdr.width;
		int padding = (pixelsWide ^ (pixelsWide << 1)) & 3;
		int stride = pixelsWide + padding;
		hdr.imgOffset = 2 /*BM*/ + sizeof(hdr);
		hdr.extraSize = 40;
		hdr.imgSize = hdr.height * stride;
		hdr.fileSize = hdr.imgOffset + hdr.imgSize;	//no palette data atm
		hdr.planes = 1;
		hdr.bitsPerPixel = 24;
		if (!file.write((const char*)&hdr, sizeof(hdr))) throw Common::Exception() << "failed to write header";
		
		int bytesPerPixel = img->getChannels();
		int dummy = 0;
		for (int y = img->getSize()(1)-1; y >= 0; y--) {
			const char *src = img->getData() + img->getSize()(0) * bytesPerPixel * y;
			for (int x = 0; x < img->getSize()(0); x++, src += bytesPerPixel) {
				for (int ch = 2; ch >= 0; --ch) {
					if (!file.put(src[ch])) throw Common::Exception() << "failed to write data";
				}
			}
			if (!file.write((const char *)&dummy, padding)) throw Common::Exception() << "failed to write padding";
		}
	} catch (const std::exception &t) {
		throw Common::Exception() << "BMP_IO::write("<<filename<<") error:" << t.what();
	}
}

}
#endif //IMAGE_SUPPORTS_BMP
