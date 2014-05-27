#include <stdio.h>
#include <stdlib.h>
extern "C" {
#include <jpeglib.h>
#ifndef WIN32
#include <jerror.h>
#endif
}
#include <setjmp.h>

#include "Common/Exception.h"
#include "Image/Image.h"
#include "Image/IO.h"

#ifdef WIN32
#define strcasecmp _stricmp
#endif

namespace Image {

struct JPEG_IO : public IO {
	virtual ~JPEG_IO(){}
	virtual const char *name() { return "JPEG_IO"; }
	virtual bool supportsExt(const char *fileExt);
	virtual IImage *load(const char *filename);
	virtual void save(const IImage *img, const char *filename);

#ifndef WIN32
	//special-case for this loader
	//useful when you download a jpeg and dont want to save it to disk to load it again
	IImage *loadFromMemory(const char *buffer, size_t size);
#endif
};

using namespace std;

//////// begin jpeglib example rip

struct my_error_mgr {
	struct jpeg_error_mgr pub;	/* "public" fields */
	jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

/*
 * Here's the routine that will replace the standard error_exit method:
 */
//METHODDEF()
static void my_error_exit (j_common_ptr cinfo) {
	/* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
	my_error_ptr myerr = (my_error_ptr) cinfo->err;

	/* Always display the message. */
	/* We could postpone this until after returning, if we chose. */
	cinfo->err->output_message (cinfo);

	//cout << "jpeg error: " << cinfo->err->msg_code << endl;

	/* Return control to the setjmp point */
	longjmp(myerr->setjmp_buffer, 1);
}

//////// end jpeglib example rip

bool JPEG_IO::supportsExt(const char *fileExt) {
	return !strcasecmp(fileExt, "jpeg")
		|| !strcasecmp(fileExt, "jpg");
}

IImage *JPEG_IO::load(const char *filename) {
	
	FILE *fp = NULL;
	struct jpeg_decompress_struct cinfo;
	bool cinfo_init = false;
	struct my_error_mgr jerr;
	unsigned char *imgdata = NULL;
	IImage *img = NULL;

	try {
		if (!(fp = fopen(filename, "rb"))) throw Exception() << "couldn't open file " << filename;

		cinfo.err = jpeg_std_error(&jerr.pub);
		
		jerr.pub.error_exit = my_error_exit;
		if (setjmp(jerr.setjmp_buffer)) throw Exception() << "jpeg error was hit";

		cinfo_init = true;	//from here on out, if our longjmp gets hit, then jpeg_destroy_decompress is required.  thats how the example goes.
		jpeg_create_decompress(&cinfo);
		jpeg_stdio_src(&cinfo, fp);
		jpeg_read_header(&cinfo, TRUE);
		jpeg_start_decompress(&cinfo);
		int row_stride = cinfo.output_width * cinfo.output_components;
		imgdata = new unsigned char[cinfo.output_height * row_stride];

		JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);
		unsigned char *dst = imgdata;
		while (cinfo.output_scanline < cinfo.output_height) {
			jpeg_read_scanlines(&cinfo, buffer, 1);
			memcpy(dst, buffer[0], row_stride);
			dst += row_stride;
		}

		jpeg_finish_decompress(&cinfo);
		
		//img's existence signifies that we've made it
		img = new Image(Vector<int,2>(cinfo.output_width, cinfo.output_height), imgdata);

	} catch (const exception &t) {
		//finally code
		if (cinfo_init) jpeg_destroy_decompress(&cinfo);
		if (fp) fclose(fp);
		//just for excpetions
		delete[] imgdata;
		throw Exception() << "JPEG_IO::load(" << filename << ") error: " << t.what();
	}
		
	if (cinfo_init) jpeg_destroy_decompress(&cinfo);
	if (fp) fclose(fp);
	
	assert(img);
	
	return img;
}

void JPEG_IO::save(const IImage *img, const char *filename) {
	throw Exception() << "not implemented yet";
}

#ifndef WIN32

//if i really wanted i could abstract this to combine with the above code
//but the above seems to use libjpeg stuff made just for file loading
//COMMIT THIS TO MEMORY
IImage *JPEG_IO::loadFromMemory(const char *buffer, size_t size) {
	
	struct jpeg_decompress_struct cinfo;
	bool cinfo_init = false;
	struct my_error_mgr jerr;
	unsigned char *imgdata = NULL;
	IImage *img = NULL;

	try {
		cinfo.err = jpeg_std_error(&jerr.pub);
		
		jerr.pub.error_exit = my_error_exit;
		if (setjmp(jerr.setjmp_buffer)) throw Exception() << "jpeg error was hit";

		cinfo_init = true;	//from here on out, if our longjmp gets hit, then jpeg_destroy_decompress is required.  thats how the example goes.
		jpeg_create_decompress(&cinfo);
		
		//here is where my code differs from above
		//that and i got irif od the file pointer
		//	jpeg_stdio_src(&cinfo, fp);
		//replacement codepiece

		//ripped from jpeg_stdio_src
		{
			//	STRUCTURE
			
			typedef struct {
				struct jpeg_source_mgr pub;	/* public fields */
				
				const char *ptr;
				size_t size;
				size_t pos;

//				FILE * infile;		/* source stream */
				JOCTET * buffer;		/* start of buffer */
				boolean start_of_file;	/* have we gotten any data yet? */
			} my_source_mgr;
			typedef my_source_mgr * my_src_ptr;
			
#define INPUT_BUF_SIZE 4096

			//	METHODS
			class JpegMemMethods {
			public:
				//can i do this like i can if its in a class in a method?
				//can i do it with static class methods?
				//eff eyah i can!
				static void init_source(j_decompress_ptr cinfo) {
					my_src_ptr src = (my_src_ptr)cinfo->src;
					src->pos = 0;
					src->start_of_file = TRUE;
				}

				static boolean fill_input_buffer (j_decompress_ptr cinfo) {
					my_src_ptr src = (my_src_ptr) cinfo->src;
					int nbytes = src->size - src->pos;
					assert(nbytes >= 0);
					if (nbytes == 0) {
						if (src->start_of_file)	/* Treat empty input file as fatal error */
							ERREXIT(cinfo, JERR_INPUT_EMPTY);
						WARNMS(cinfo, JWRN_JPEG_EOF);
						/* Insert a fake EOI marker */
						src->buffer[0] = (JOCTET) 0xFF;
						src->buffer[1] = (JOCTET) JPEG_EOI;
						nbytes = 2;
					} else {
						if (nbytes > INPUT_BUF_SIZE) {
							memcpy(src->buffer, src->ptr + src->pos, nbytes);
							src->pos += nbytes;
						}
					}

					src->pub.next_input_byte = src->buffer;
					src->pub.bytes_in_buffer = nbytes;
					src->start_of_file = FALSE;
					return TRUE;
				}
				
				static void skip_input_data (j_decompress_ptr cinfo, long num_bytes) {
					my_src_ptr src = (my_src_ptr) cinfo->src;
					if (num_bytes > 0) {
#if 0
						while (num_bytes > (long) src->pub.bytes_in_buffer) {
							num_bytes -= (long) src->pub.bytes_in_buffer;
							fill_input_buffer(cinfo);
						}
						src->pub.next_input_byte += (size_t) num_bytes;
						src->pub.bytes_in_buffer -= (size_t) num_bytes;
#else
						src->pos += num_bytes;
#endif
					}
				}

				static void term_source(j_decompress_ptr cinfo) { }
			};
			

			
			//	INIT

			my_src_ptr src;
			if (cinfo.src == NULL) {	/* first time for this JPEG object? */
				cinfo.src = (struct jpeg_source_mgr *)cinfo.mem->alloc_small((j_common_ptr)&cinfo, JPOOL_PERMANENT, sizeof(my_source_mgr));
				src = (my_src_ptr) cinfo.src;
				//why can't i just stuff all my pointer in here to begin with?
				src->buffer = (JOCTET *) cinfo.mem->alloc_small((j_common_ptr)&cinfo, JPOOL_PERMANENT, INPUT_BUF_SIZE * sizeof(JOCTET));
			}

			src = (my_src_ptr) cinfo.src;
			src->pub.init_source = JpegMemMethods::init_source;
			src->pub.fill_input_buffer = JpegMemMethods::fill_input_buffer;
			src->pub.skip_input_data = JpegMemMethods::skip_input_data;
			src->pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
			src->pub.term_source = JpegMemMethods::term_source;
			src->pub.bytes_in_buffer = 0; /* forces fill_input_buffer on first read */
			src->pub.next_input_byte = NULL; /* until buffer loaded */
			src->ptr = buffer;
			src->size = size;
			src->pos = 0;
		}
		
		
		jpeg_read_header(&cinfo, TRUE);
		jpeg_start_decompress(&cinfo);
		int row_stride = cinfo.output_width * cinfo.output_components;

		imgdata = new unsigned char[cinfo.output_height * row_stride];

		JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);
		unsigned char *dst = imgdata;
		while (cinfo.output_scanline < cinfo.output_height) {
			jpeg_read_scanlines(&cinfo, buffer, 1);
			memcpy(dst, buffer[0], row_stride);
			dst += row_stride;
		}

		jpeg_finish_decompress(&cinfo);
		
		//img's existence signifies that we've made it
		img = new Image(Vector<int,2>(cinfo.output_width, cinfo.output_height), imgdata);

	} catch (const exception &t) {
		//finally code
		if (cinfo_init) jpeg_destroy_decompress(&cinfo);
		//just for excpetions
		delete[] imgdata;
		throw Exception() << "JPEG_IO::loadFromMemory() error: " << t.what();
	}
		
	if (cinfo_init) jpeg_destroy_decompress(&cinfo);
	assert(img);
	return img;
}

#endif

Singleton<JPEG_IO> jpegIO;

};
