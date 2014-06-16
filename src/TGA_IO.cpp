#if defined(SUPPORT_TGA)
#include "Image/TGA_IO.h"
#include "Common/Exception.h"
#include <fstream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>

#ifdef WIN32
#define strcasecmp _stricmp
#endif

namespace Image {

TGA_IO::~TGA_IO() {}

std::string TGA_IO::name() { return "TGA_IO"; }
	
bool TGA_IO::supportsExtension(const std::string& extension) {
	return !strcasecmp(extension.c_str(), "tga");
}
	
std::shared_ptr<IImage> TGA_IO::read(const std::string& filename) {
	try {
		std::ifstream file(filename, std::ios::binary);
		if (!file) throw Common::Exception() << "unable to open file";

		char header[18];
		
		if (!file.read(header, sizeof(header))) throw Common::Exception() << "couldnt read header";
		if (!file.seekg(header[0], std::ios::cur)) throw Common::Exception() << "error trying to seek past image ID info";

		//indexed color stuff:
		std::vector<char> colorMap;
		if (header[1]) {		//a color map is present - lets read it
			int colorMapCount = *(unsigned short*)(&header[5]);
			int colorMapBits = header[7];

			//my way of rounding up - base 8
			int colorMapBytes = (colorMapBits>>3) + !!(colorMapBits & 7);

			colorMap.resize(colorMapCount * colorMapBytes);
		}

//		int imageType = header[2] & 7;
		int RLEcompressed = header[2] & 8;
		if (RLEcompressed) throw Common::Exception() << "this TGA file just happens to be RLE, which I dont support yet";

		unsigned short width = *(unsigned short*)(&header[12]);
		unsigned short height = *(unsigned short *)(&header[14]);
		unsigned int bitsPerPixel = header[16];
		int src_bytesPerPixel = bitsPerPixel>>3;

		int bytes_per_pixel;
		if (src_bytesPerPixel == 4) bytes_per_pixel = 4;
		else bytes_per_pixel = 3;

		bool flip_y = true;
		if (header[17] & 0x20) flip_y = false;		//vertical flip
		//if (header[17] & 0x10) flags |= TF_FLIP_X;		//horizontal flip

		if(bitsPerPixel != 8 && bitsPerPixel !=24 && bitsPerPixel!=32) {
			throw Common::Exception() <<"This TGA is not of either 8, 24, or 32 bits per pixel";
		}

		unsigned int imageSize = width * height * bytes_per_pixel;

		std::vector<char> data(imageSize);

		unsigned int rowSize	= width * src_bytesPerPixel;

		char *rowPtr = &data[0];
		int x;
		for (int y = 0; y < (int)height; y++) {
			if (bitsPerPixel == 8) {
				char t;
				for (x = 0; x < (int)width; x++) {
					if (!file.read(&t, 1) != 1) throw Common::Exception() << "error in reading this TGA";

					if (header[1] == 0 && header[2] == 3) {	//greyscale 8bit image
						rowPtr[2] = rowPtr[1] = rowPtr[0] = t;
					} else if (header[1] == 1 && header[2] == 1) {
						if (!colorMap.size()) throw Common::Exception() << "colormap hasnt been found";
						memcpy(rowPtr, &colorMap[t*3], src_bytesPerPixel);
					}

					rowPtr += src_bytesPerPixel;
				}
			} else { //24, 32
				if (!file.read(rowPtr, rowSize)) throw Common::Exception() << "couldnt read texture row";
				rowPtr += rowSize;
			}
		}

		int imgsize = rowSize * height;
		std::vector<unsigned char> imgdata(imgsize);

		//now read it through - getting rid of the row spacing
		char *srcptr = &data[0];	//reset the src ptr to the data start
		unsigned char *dstptr = &imgdata[0];
		for (int row = 0; row < height; row++) {
			//swap rows as we go =) bitmaps are upside-down
			if (flip_y) {
				dstptr = &imgdata[(height - row - 1) * rowSize];
			}
			//swap red and blue as we go =)
			for (int col = 0; col < width; col++, srcptr += src_bytesPerPixel, dstptr += src_bytesPerPixel) {
				//copy over as much as we want
				memcpy(dstptr, srcptr, src_bytesPerPixel);
				//swap red & blue
				dstptr[0] = srcptr[2];
				dstptr[2] = srcptr[0];
			}
		}
	
		if (bitsPerPixel & 7) throw Common::Exception() << "unsupported bits per pixel " << bitsPerPixel;
		return std::make_shared<Image>(Tensor::Vector<int,2>(width, height), &imgdata[0], bitsPerPixel >> 3);
	} catch (const std::exception &t) {
		//finally
		//all else
		throw Common::Exception() << "TGA_IO::read(" << filename << ") error: " << t.what();
	}
}
	
void TGA_IO::write(const std::string& filename, std::shared_ptr<const IImage> img) {
	throw Common::Exception() << "not implemented yet";
}

Common::Singleton<TGA_IO> tgaIO;

};
#endif	//SUPPORT_TGA

