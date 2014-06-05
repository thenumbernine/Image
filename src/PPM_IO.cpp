#include "Image/IO.h"
#include "Common/Exception.h"
#include "Common/Finally.h"
#include "Common/File.h"
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#ifdef WIN32
#define strcasecmp _stricmp
#endif

namespace Image {

struct PPM_IO : public IO {
	virtual ~PPM_IO(){}
	virtual std::string name() { return "PPM_IO"; }
	virtual bool supportsExtension(std::string extension) {
		return !strcasecmp(extension.c_str(), "ppm");
	}
	virtual IImage *read(std::string filename) {
		try {
			
			int h,w, maxVal;
			char sbuf[128];
			char * tok;

			FILE *file = fopen(filename.c_str(), "r");
			if (!file) throw Common::Exception() << "failed to open file";
			Common::Finally finally([&](){ fclose(file); });

			fgets(sbuf, sizeof(sbuf), file);
			fgets(sbuf, sizeof(sbuf), file);
			fgets(sbuf, sizeof(sbuf), file);
			tok = strtok (sbuf, " "); w = atoi(tok);
			tok = strtok (NULL, " "); h = atoi(tok);
			fgets(sbuf, sizeof(sbuf), file);
			maxVal = atoi(sbuf);

			int size = w*h*3;
		
			std::vector<unsigned char> imgdata(size);

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

			return new Image(Tensor::Vector<int,2>(w,h), &imgdata[0]);
		} catch (const std::exception &t) {
			throw Common::Exception() << "PPM_IO::read(" << filename << ") error: " << t.what();
		}
	}
	virtual void write(std::string filename, const IImage *img) {
		throw Common::Exception() << "not implemented yet";
	}
};
static Common::Singleton<PPM_IO> ppmIO;

};
