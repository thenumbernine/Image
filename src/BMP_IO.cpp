#include <fstream>
#include "Image/IO.h"
#include "Image/Image.h"

#ifdef WIN32
#define strcasecmp _stricmp
#endif

using namespace Common;
using namespace std;

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

struct BMP_IO : public IO {
	virtual ~BMP_IO(){}
	virtual const char *name() { return "BMP_IO"; }
	virtual bool supportsExt(const char *fileExt) {
		return !strcasecmp(fileExt, "bmp");
	}
	virtual IImage *load(const char *filename) {

		//list out resources we have to free here:
		unsigned char *imgdata = NULL;
		IImage *img = NULL;
		try {
			ifstream f(filename, ios::binary);
			if (!f.is_open()) throw Exception() << "failed to open file for reading";
			
			short sig;
			f.read((char *)&sig, sizeof(sig));
			if (sig != 0x4D42) throw Exception() << "got a bad sig:" << sig;
			
			bitmapHeader_t hdr;
			f.read((char *)&hdr, sizeof(hdr));

			if (hdr.planes != 1) throw Exception() << "got bad # planes:" << hdr.planes;
			if (hdr.bitsPerPixel != 24) throw Exception() << "got an unhandled bitsPerPixel:" << hdr.bitsPerPixel;
			if (hdr.compression) throw Exception() << "got an unhandled compression: " << hdr.compression;

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
			imgdata = new unsigned char[imgsize];
			
			f.seekg(hdr.imgOffset, ios_base::beg);

			//now read it through - getting rid of the row spacing
			int dummy;
			for (int y = ystart; y != yend; y += ystep) {
				//swap ys as we go =) bitmaps are upside-down
				unsigned char *dst = imgdata + y * hdr.width * 3;
				//swap red and blue as we go =)
				for (int x = 0; x < hdr.width; x++, dst += 3) {
					dst[2] = f.get();
					dst[1] = f.get();
					dst[0] = f.get();
				}
				f.read((char *)&dummy, padding);
			}
			
			//do this last so img == null if anything goes wrong
			img = new Image(
				Vector<int,2>(hdr.width, height),	//size
				imgdata,							//data
				hdr.bitsPerPixel >> 3);				//channels
		} catch (const exception &t) {
			//finally code ...
			//only for errors
			delete[] imgdata;
			throw Exception() << "BMP_IO::load("<<filename<<") error: " << t.what();
		}
		assert(img);
		return img;		
	}
	virtual void save(const IImage *img, const char *filename) {
		try {
			if (img->getBitsPerPixel() < 24) throw Exception() << "don't support writing for " << img->getBitsPerPixel() << " bits per pixel yet";

			ofstream f(filename, ios::binary);
			if (!f.is_open()) throw Exception() << "failed to open file for writing";
			f.write("BM",2);
			
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
			f.write((const char*)&hdr, sizeof(hdr));
			
			int bytesPerPixel = img->getChannels();
			int dummy = 0;
			for (int y = img->getSize()(1)-1; y >= 0; y--) {
				const char *src = img->getData() + img->getSize()(0) * bytesPerPixel * y;
				for (int x = 0; x < img->getSize()(0); x++, src += bytesPerPixel) {
					f.put(src[2]);
					f.put(src[1]);
					f.put(src[0]);
				}
				f.write((const char *)&dummy, padding);
			}
		} catch (const exception &t) {
			throw Exception() << "BMP_IO::save("<<filename<<") error:" << t.what();
		}
	}
};
static Singleton<BMP_IO> bmpIO;

};

