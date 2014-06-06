#include "Image/Image.h"
#include "Image/System.h"
#include <string>
#include <vector>

int main() {
	Image::Image image(Tensor::Vector<int,2>(256, 256));
	for (int y = 0; y < 256; ++y) {
		for (int x = 0; x < 256; ++x) {
			image(x,y,0) = x;
			image(x,y,1) = y;
			image(x,y,2) = 128;
		}
	}

	//write test
	std::vector<std::string> exts = {
		"bmp",
		"fits",
		"jpeg",
		"png",
		"ppm",
		"tga",
		"tiff"
	};

	for (std::string ext : exts) {
		Image::sys->write("test." + ext, &image);
	}

	return 0;
}

