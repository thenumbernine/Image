#if IMAGE_SUPPORTS_PPM
#include "Image/PPM_IO.h"
#include "Common/Exception.h"
#include "Common/Finally.h"
#include "Common/File.h"
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#if PLATFORM_MSVC
#define strcasecmp _stricmp
#endif

namespace Image {

PPM_IO::~PPM_IO() {}

std::string PPM_IO::name() { return "PPM_IO"; }

bool PPM_IO::supportsExtension(const std::string& extension) {
	return !strcasecmp(extension.c_str(), "ppm");
}

std::shared_ptr<IImage> PPM_IO::read(const std::string& filename) {
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

void PPM_IO::write(const std::string& filename, std::shared_ptr<const IImage> img) {
	throw Common::Exception() << "not implemented yet";
}

}
#endif	//IMAGE_SUPPORTS_PPM
