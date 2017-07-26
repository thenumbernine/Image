#if defined(Image_supports_ppm)
#include "Image/IO.h"
#include "Common/Exception.h"
#include "Common/Finally.h"
#include "Common/File.h"
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#ifdef PLATFORM_msvc
#define strcasecmp _stricmp
#endif

namespace Image {

struct PPM_IO : public IO {
	virtual ~PPM_IO(){}
	virtual std::string name() { return "PPM_IO"; }
	virtual bool supportsExtension(const std::string& extension) {
		return !strcasecmp(extension.c_str(), "ppm");
	}
	virtual std::shared_ptr<IImage> read(const std::string& filename) {
		try {
			
			char sbuf[128];

			FILE *file = fopen(filename.c_str(), "r");
			if (!file) throw Common::Exception() << "failed to open file";
			Common::Finally finally([&](){ fclose(file); });

			if (!fgets(sbuf, sizeof(sbuf), file)) throw Common::Exception() << "fgets failed";
			if (!fgets(sbuf, sizeof(sbuf), file)) throw Common::Exception() << "fgets failed";
			if (!fgets(sbuf, sizeof(sbuf), file)) throw Common::Exception() << "fgets failed";
			char* tok = strtok (sbuf, " ");
			int w = atoi(tok);
			tok = strtok (NULL, " ");
			int h = atoi(tok);
			if (!fgets(sbuf, sizeof(sbuf), file)) throw Common::Exception() << "fgets failed";
			//int maxVal = atoi(sbuf);

			int size = w*h*3;
		
			std::vector<unsigned char> imgdata(size);

			int i = 0;

			while (!feof(file)) {

				if (!fgets(sbuf, sizeof(sbuf), file)) throw Common::Exception() << "fgets failed";
				tok = strtok (sbuf," ");
				while (tok != NULL) {
					if (atoi(tok) != 0 || tok[0] == '0') imgdata[i++] = (unsigned char)atoi(tok);
					tok = strtok (NULL, " ");

					if (i >= size) break;
				}

				if (i >= size) break;
			}

			return std::make_shared<Image>(Tensor::Vector<int,2>(w,h), &imgdata[0]);
		} catch (const std::exception &t) {
			throw Common::Exception() << "PPM_IO::read(" << filename << ") error: " << t.what();
		}
	}
	virtual void write(const std::string& filename, std::shared_ptr<const IImage> img) {
		throw Common::Exception() << "not implemented yet";
	}
};

Common::Singleton<PPM_IO> ppmIO;

};
#endif	//Image_supports_ppm
