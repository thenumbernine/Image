#include <stdio.h>
#include <stdlib.h>
#include "Common/Exception.h"
#include "Image/IO.h"

#ifdef WIN32
#define strcasecmp _stricmp
#endif

using namespace std;

namespace Image {

struct TGA_IO : public IO {
	virtual ~TGA_IO(){}
	virtual const char *name() { return "TGA_IO"; }
	virtual bool supportsExt(const char *fileExt) {
		return !strcasecmp(fileExt, "tga");
	}
	virtual IImage *load(const char *filename) {
		FILE *file = NULL;
		unsigned char *colorMap = NULL;
		unsigned char *data = NULL;
		unsigned char *imgdata = NULL;
		IImage *img = NULL;
		
		try {
			//open the file
			file = fopen(filename, "rb");
			if (!file) throw Exception() << "unable to open file";

			unsigned char header[18];
			if (fread(header,1,sizeof(header),file) != sizeof(header)) throw Exception() << "couldnt read header";
			if (fseek(file, header[0], SEEK_CUR)) throw Exception() << "error trying to seek past image ID info";

			//indexed color stuff:
			if (header[1]) {		//a color map is present - lets read it
				int colorMapCount = *(unsigned short*)(&header[5]);
				int colorMapBits = header[7];

				//my way of rounding up - base 8
				int colorMapBytes = (colorMapBits>>3) + !!(colorMapBits & 7);

				colorMap = new unsigned char[colorMapCount * colorMapBytes];
			}

	//		int imageType = header[2] & 7;
			int RLEcompressed = header[2] & 8;
			if (RLEcompressed) throw Exception() << "this TGA file just happens to be RLE, which I dont support yet";

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
				throw Exception() <<"This TGA is not of either 8, 24, or 32 bits per pixel";
			}

			unsigned int imageSize = width * height * bytes_per_pixel;

			data = new unsigned char[imageSize];

		//	for(temp=0; temp<(int)imageSize; temp++) data[temp]=0;

			unsigned int rowSize	= width * src_bytesPerPixel;

			unsigned char *rowPtr = data;
			int x;
			for (int y = 0; y < (int)height; y++) {
				if (bitsPerPixel == 8) {
					unsigned char t;
					for (x = 0; x < (int)width; x++) {
						if (fread(&t, 1, 1, file) != 1) throw Exception() << "error in reading this TGA";

						if (header[1] == 0 && header[2] == 3) {	//greyscale 8bit image
							rowPtr[2] = rowPtr[1] = rowPtr[0] = t;
						} else if (header[1] == 1 && header[2] == 1) {
							if (!colorMap) throw Exception() << "colormap hasnt been found";
							memcpy(rowPtr, colorMap+(t*3), src_bytesPerPixel);
						}

						rowPtr += src_bytesPerPixel;
					}
				} else { //24, 32
					if (fread(rowPtr, 1, rowSize, file) != rowSize) throw Exception() << "couldnt read texture row";
					rowPtr += rowSize;
				}
			}

			fclose (file);
			file = NULL;

			int imgsize = rowSize * height;
			imgdata = new unsigned char[imgsize];

			//now read it through - getting rid of the row spacing
			unsigned char *srcptr = data;	//reset the src ptr to the data start
			unsigned char *dstptr = imgdata;
			for (int row = 0; row < height; row++) {
				//swap rows as we go =) bitmaps are upside-down
				if (flip_y) {
					dstptr = imgdata + (height - row - 1) * rowSize;
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
		
			if (bitsPerPixel & 7) throw Exception() << "unsupported bits per pixel " << bitsPerPixel;
			img = new Image(Vector<int,2>(width, height), imgdata, bitsPerPixel >> 3);
		} catch (const exception &t) {
			//finally
			if (file) fclose(file);
			delete[] colorMap;
			delete[] data;
			//all else
			delete[] imgdata;
			throw Exception() << "TGA_IO::load(" << filename << ") error: " << t.what();
		}
		if (file) fclose(file);
		delete[] colorMap;
		delete[] data;
		assert(img);
		return img;
	}
	virtual void save(const IImage *img, const char *filename) {
		throw Exception() << "not implemented yet";
	}
};
static Singleton<TGA_IO> tgaIO;

};
