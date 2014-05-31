#include <stdio.h>
#include <stdlib.h>
#include <png.h>
#include "Common/Exception.h"
#include "Image/IO.h"

#ifdef WIN32
#define strcasecmp _stricmp
#endif

//http://zarb.org/~gc/html/libpng.html

using namespace Common;

namespace Image {

struct PNG_IO : public IO {
	virtual ~PNG_IO(){}
	virtual const char *name() { return "PNG_IO"; }
	virtual bool supportsExt(const char *fileExt);
	virtual IImage *load(const char *filename);
	virtual void save(const IImage *img, const char *filename);
};

using namespace std;

#if 0
static int read_chunk_callback(png_infop ptr,
	 png_unknown_chunkp chunk)
{
   /* The unknown chunk structure contains your
	  chunk data: */
	   png_byte name[5];
	   png_byte *data;
	   png_size_t size;
   /* Note that libpng has already taken care of
	  the CRC handling */

   /* put your code here.  Return one of the
	  following: */
	int n = 1;

   return (-n); /* chunk had an error */
   return (0); /* did not recognize */
   return (n); /* success */
}
#endif

bool PNG_IO::supportsExt(const char *fileExt) {
	return !strcasecmp(fileExt, "png");
}

IImage *PNG_IO::load(const char *filename) {	
	FILE *fp = NULL;
	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	png_infop end_info = NULL;
	png_bytep row_buf = NULL;
	unsigned char *imgdata = NULL;
	IImage *img = NULL;
	
	try {
		if (!(fp = fopen(filename, "rb"))) throw Exception() << "couldn't open file " << filename;

		if (!(png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, (png_voidp)NULL, NULL, NULL))) throw Exception() << "failed to alloc png_ptr";
		if (!(info_ptr = png_create_info_struct(png_ptr))) throw Exception() << "failed to create info struct";
		if (!(end_info = png_create_info_struct(png_ptr))) throw Exception() << "failed to create end info struct";

		//i'm new to long jumps, but does this make this location the destination of the png_ptr longjump?
		if (setjmp(png_jmpbuf(png_ptr))) throw Exception() << "png_jmpbuf was hit";

		//from here on out we're using libpng's longjump rather than C++'s try/catch

		png_init_io(png_ptr, fp);
		png_set_read_status_fn(png_ptr, NULL);		//for reporting stuff as the rows complete reading

		//getting a crash here with my windows library
		png_read_info(png_ptr, info_ptr);

		png_uint_32 width = 0;
		png_uint_32 height = 0;
		int bit_depth = 0;
		int color_type = 0;
		png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, NULL, NULL, NULL);

		//rather than complain up front, lets hope that the conversion code below will handle everything
		//int channels = png_get_channels(png_ptr, info_ptr);
		//if (channels != 3 && channels != 4) throw &(s << "got invalid channel count " << channels);

		int rowbytes = png_get_rowbytes(png_ptr, info_ptr);

		//do any conversions that we can
		if (color_type == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(png_ptr);
		//if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) png_set_gray_1_2_4_to_8(png_ptr);
		if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) png_set_tRNS_to_alpha(png_ptr);
		if (bit_depth == 16) png_set_strip_16(png_ptr);
		if (bit_depth < 8) png_set_packing(png_ptr);
		
		row_buf = (png_bytep)png_malloc(png_ptr, rowbytes);
		int num_pass = png_set_interlace_handling(png_ptr);

		bool hasAlpha = color_type & PNG_COLOR_MASK_ALPHA;
		int bytespp = hasAlpha ? 4 : 3;
		imgdata = new unsigned char[height * rowbytes];
		
		//what is the purpose of multiple passes?
		//interlaced mode, but i'm memcpying... 
		for (int pass = 0; pass < num_pass; pass++) {
			unsigned char *dst = imgdata;
			for (int y = 0; y < height; y++) {
				png_read_rows(png_ptr, (png_bytepp)&row_buf, NULL, 1);
				memcpy(dst, row_buf, rowbytes);
				dst += rowbytes;
			}
		}

		png_free_data(png_ptr, info_ptr, PNG_FREE_UNKN, -1);
		png_read_end(png_ptr, end_info);
		
		//img's existence signifies that we've made it
		img = new Image(Vector<int,2>(width, height), imgdata, bytespp);
	} catch (const exception &t) {
		//finally
		if (row_buf) png_free(png_ptr, row_buf);
		if (png_ptr) png_destroy_read_struct(&png_ptr, info_ptr ? &info_ptr : (png_infopp)NULL, end_info ? &end_info : (png_infopp)NULL);
		if (fp) fclose(fp);
		//and all else
		delete[] imgdata;
		throw Exception() << "PNG_IO::load(" << filename << ") error: " << t.what();
	}

	if (row_buf) png_free(png_ptr, row_buf);
	if (png_ptr) png_destroy_read_struct(&png_ptr, info_ptr ? &info_ptr : (png_infopp)NULL, end_info ? &end_info : (png_infopp)NULL);
	if (fp) fclose(fp);
	
	assert(img);
	return img;
}

void PNG_IO::save(const IImage *img, const char *filename) {

	FILE *fp = NULL;
	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	png_bytep * row_pointers = NULL;

	try {
		/* create file */
		if (!(fp = fopen(filename, "wb"))) throw Exception() << "could not be opened for writing";

		/* initialize stuff */
		png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		if (!png_ptr) throw Exception() << "png_create_write_struct failed";

		info_ptr = png_create_info_struct(png_ptr);
		if (!info_ptr) throw Exception() << "png_create_info_struct failed";

		if (setjmp(png_jmpbuf(png_ptr))) throw Exception() << "Error during init_io";

		png_init_io(png_ptr, fp);


		/* write header */
		if (setjmp(png_jmpbuf(png_ptr))) throw Exception() << "Error during writing header";

		if (img->getChannels() != 3 && img->getChannels() != 4) throw Exception() << "only supports 24 or 32 bpp";

		png_uint_32 width = img->getSize()(0);
		png_uint_32 height = img->getSize()(1);
		int bit_depth = 8;	//bits per channel, not bits per pixel
		int color_type = img->getChannels() == 3 ? PNG_COLOR_TYPE_RGB : PNG_COLOR_TYPE_RGBA;
		png_set_IHDR(png_ptr, info_ptr, width, height, bit_depth, color_type, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

		png_write_info(png_ptr, info_ptr);

		row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
		int bytesPerPixel = img->getChannels();
		for (int y=0; y<height; y++) {
			row_pointers[y] = (png_byte*)(img->getData() + y * img->getSize()(0) * bytesPerPixel);
		}
		
		
		/* write bytes */
		if (setjmp(png_jmpbuf(png_ptr))) throw Exception() << "Error during writing bytes";

		png_write_image(png_ptr, row_pointers);

		/* end write */
		if (setjmp(png_jmpbuf(png_ptr))) throw Exception() << "Error during end of write";

		png_write_end(png_ptr, NULL);
	} catch (const exception &t) {
		//finally
		free(row_pointers);
		fclose(fp);
		//all else
		throw Exception() << "PNG_IO::save("<<filename<<") failed: " << t.what();
	}
	
	//TODO - destroy_write_struct, to parallel the destroy_read_struct above?

	free(row_pointers);
	fclose(fp);
}

Singleton<PNG_IO> pngIO;

};
