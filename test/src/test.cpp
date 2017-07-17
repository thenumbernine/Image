#include "Image/Image.h"
#include "Image/System.h"
#include "Common/Test.h"
#include <string>
#include <vector>

int main() {
	std::shared_ptr<Image::Image> image = std::make_shared<Image::Image>(Tensor::Vector<int,2>(256, 256));
	for (int y = 0; y < 256; ++y) {
		for (int x = 0; x < 256; ++x) {
			(*image)(x,y,0) = x;
			(*image)(x,y,1) = y;
			(*image)(x,y,2) = 128;
		}
	}

	TEST_EQ(image->getSize(), Tensor::Vector<int COMMA 2>(256, 256));
	TEST_EQ(image->getBitsPerPixel(), 24);

	//read test
	
	std::vector<std::string> readExts = {
		"bmp",
		"fits",
		"jpeg",
		"png",
		"ppm",
		"tga",
		"tiff"
	};

	for (std::string ext : readExts) {
		std::string filename = "test." + ext;
		std::cout << "testing read of " << filename << std::endl;
		std::shared_ptr<Image::Image> check = std::dynamic_pointer_cast<Image::Image>(Image::system->read(filename));
		TEST_EQ(check->getSize(), Tensor::Vector<int COMMA 2>(256, 256));
		TEST_EQ(check->getBitsPerPixel(), 24);
		int totalError = 0;
		for (int y = 0; y < 256; ++y) {
			for (int x = 0; x < 256; ++x) {
				for (int ch = 0; ch < 3; ++ch) {
					int err = (int)(*image)(x,y,ch) - (int)(*check)(x,y,ch);
					totalError += err;
				}
			}
		}
		std::cout << filename << " read total error " << totalError << std::endl;
	}
	
	//write test
	std::vector<std::string> writeExts = {
		"bmp",
		"fits",
		//"jpeg",	//I'm having link errors. dyld: Symbol not found: _jpeg_resync_to_restart 
		"png"
		//"ppm",	//not yet implemented
		//"tga",	//not yet implemented
		//"tiff"	//not yet implemented
	};

	for (std::string ext : writeExts) {
		Image::system->write("test." + ext, image);
	}



	return 0;
}

