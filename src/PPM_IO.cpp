#include <stdio.h>
#include <stdlib.h>
#include "Common/Exception.h"
#include "Image/IO.h"

#ifdef WIN32
#define strcasecmp _stricmp
#endif

using namespace std;

namespace Image {

struct PPM_IO : public IO {
	virtual ~PPM_IO(){}
	virtual const char *name() { return "PPM_IO"; }
	virtual bool supportsExt(const char *fileExt) {
		return !strcasecmp(fileExt, "ppm");
	}
	virtual IImage *load(const char *filename) {
	
		FILE *file;
		unsigned char *imgdata = NULL;
		IImage *img = NULL;
		
		try {
			
			int h,w, maxVal;
			char sbuf[128];
			char * tok;

			file = fopen(filename, "r");
			if (!file) throw Exception() << "failed to open file";

			fgets(sbuf, sizeof(sbuf), file);
			fgets(sbuf, sizeof(sbuf), file);
			fgets(sbuf, sizeof(sbuf), file);
			tok = strtok (sbuf, " "); w = atoi(tok);
			tok = strtok (NULL, " "); h = atoi(tok);
			fgets(sbuf, sizeof(sbuf), file);
			maxVal = atoi(sbuf);

			int size = w*h*3;
			
			imgdata = new unsigned char[size];

			int i = 0;

			while (!feof(file)) {

				fgets(sbuf, 128, file);
				tok = strtok (sbuf," ");
				while (tok != NULL) {
					if (atoi(tok) != 0 || tok[0] == '0') imgdata[i++] = (unsigned char)atoi(tok);
					tok = strtok (NULL, " ");

					if (i >= size) break;
				}

				if (i >= size) break;
			}

			img = new Image(Vector<int,2>(w,h),imgdata);
		} catch (const exception &t) {
			//finally
			if (file) fclose(file);
			//all else
			delete[] imgdata;
			throw Exception() << "PPM_IO::load(" << filename << ") error: " << t.what();
		}
		if (file) fclose(file);
		assert(img);
		return img;
	}
	virtual void save(const IImage *img, const char *filename) {
		throw Exception() << "not implemented yet";
	}
};
static Singleton<PPM_IO> ppmIO;

};
