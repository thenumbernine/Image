#if defined(SUPPORT_JPEG)
#include "Image/JPEG_IO.h"
#include "Common/Exception.h"
#include "Common/Finally.h"

#include <stdio.h>
#include <stdlib.h>
extern "C" {
#include <jpeglib.h>
#ifndef WIN32
#include <jerror.h>
#endif
}
#include <setjmp.h>

#include <vector>

#ifdef WIN32
#define strcasecmp _stricmp
#endif

namespace Image {

JPEG_IO::~JPEG_IO() {}

std::string JPEG_IO::name() { return "JPEG_IO"; }

//////// begin jpeglib example

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

//////// end jpeglib example

bool JPEG_IO::supportsExtension(std::string extension) {
	return !strcasecmp(extension.c_str(), "jpeg")
		|| !strcasecmp(extension.c_str(), "jpg");
}

std::shared_ptr<IImage> JPEG_IO::read(std::string filename) {
	try {
		FILE *file = fopen(filename.c_str(), "rb");
		if (!file) throw Common::Exception() << "couldn't open file " << filename;
		Common::Finally fileFinally([&](){ fclose(file); });

		struct my_error_mgr jerr;
		
		struct jpeg_decompress_struct cinfo;
		cinfo.err = jpeg_std_error(&jerr.pub);
		
		jerr.pub.error_exit = my_error_exit;
		if (setjmp(jerr.setjmp_buffer)) throw Common::Exception() << "jpeg error was hit";

		jpeg_create_decompress(&cinfo);
		Common::Finally jpegFinally([&](){ jpeg_destroy_decompress(&cinfo); });
		
		jpeg_stdio_src(&cinfo, file);
		jpeg_read_header(&cinfo, TRUE);
		jpeg_start_decompress(&cinfo);
		int row_stride = cinfo.output_width * cinfo.output_components;
		std::vector<unsigned char> imgdata(cinfo.output_height * row_stride);

		JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);
		unsigned char *dst = &imgdata[0];
		while (cinfo.output_scanline < cinfo.output_height) {
			jpeg_read_scanlines(&cinfo, buffer, 1);
			memcpy(dst, buffer[0], row_stride);
			dst += row_stride;
		}

		jpeg_finish_decompress(&cinfo);
		
		return std::make_shared<Image>(Tensor::Vector<int,2>(cinfo.output_width, cinfo.output_height), &imgdata[0]);
	} catch (const std::exception &t) {
		throw Common::Exception() << "JPEG_IO::read(" << filename << ") error: " << t.what();
	}
}

void JPEG_IO::write(std::string filename, std::shared_ptr<const IImage> img) {
	throw Common::Exception() << "not implemented yet";
}

#ifndef WIN32

//if i really wanted i could abstract this to combine with the above code
//but the above seems to use libjpeg stuff made just for file loading
std::shared_ptr<IImage> JPEG_IO::readFromMemory(const char *buffer, size_t size) {
	try {
		struct my_error_mgr jerr;
		
		struct jpeg_decompress_struct cinfo;
		cinfo.err = jpeg_std_error(&jerr.pub);
		
		jerr.pub.error_exit = my_error_exit;
		if (setjmp(jerr.setjmp_buffer)) throw Common::Exception() << "jpeg error was hit";

		jpeg_create_decompress(&cinfo);
		Common::Finally jpegFinally([&](){ jpeg_destroy_decompress(&cinfo); });
		
		//here is where my code differs from above
		//that and i got irif od the file pointer
		//	jpeg_stdio_src(&cinfo, fp);
		//replacement codepiece

		//taken from jpeg_stdio_src
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

		std::vector<unsigned char> imgdata(cinfo.output_height * row_stride);

		JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);
		unsigned char *dst = &imgdata[0];
		while (cinfo.output_scanline < cinfo.output_height) {
			jpeg_read_scanlines(&cinfo, buffer, 1);
			memcpy(dst, buffer[0], row_stride);
			dst += row_stride;
		}

		jpeg_finish_decompress(&cinfo);
		
		return std::make_shared<Image>(Tensor::Vector<int,2>(cinfo.output_width, cinfo.output_height), &imgdata[0]);
	} catch (const std::exception &t) {
		throw Common::Exception() << "JPEG_IO::readFromMemory() error: " << t.what();
	}
}

#endif

Common::Singleton<JPEG_IO> jpegIO;

};
#endif	//SUPPORT_JPEG

