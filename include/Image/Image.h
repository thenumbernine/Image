#pragma once

#include "Image/System.h"

#if defined(Image_supports_bmp)
#include "Image/BMP_IO.h"
#endif	//Image_supports_bmp

#if defined(Image_supports_fits)
#include "Image/FITS_IO.h"
#endif	//Image_supports_fits

#if defined(Image_supports_jpeg)
#include "Image/JPEG_IO.h"
#endif	//Image_supports_jpeg

#if defined(Image_supports_png)
#include "Image/PNG_IO.h"
#endif	//Image_supports_png

#if defined(Image_supports_ppm)
#include "Image/PPM_IO.h"
#endif	//Image_supports_ppm

#if defined(Image_supports_tga)
#include "Image/TGA_IO.h"
#endif	//Image_supports_tga

#if defined(Image_supports_tiff)
#include "Image/TIFF_IO.h"
#endif	//Image_supports_tiff

namespace Image {

extern Common::Singleton<System> system;

#if defined(Image_supports_bmp)
extern Common::Singleton<BMP_IO> bmpIO;
#endif	//Image_supports_bmp

#if defined(Image_supports_fits)
extern Common::Singleton<FITS_IO> fitsIO;
#endif	//Image_supports_fits

#if defined(Image_supports_jpeg)
extern Common::Singleton<JPEG_IO> jpegIO;
#endif	//Image_supports_jpeg

#if defined(Image_supports_png)
extern Common::Singleton<PNG_IO> pngIO;
#endif	//Image_supports_png

#if defined(Image_supports_ppm)
extern Common::Singleton<PPM_IO> ppmIO;
#endif	//Image_supports_ppm

#if defined(Image_supports_tga)
extern Common::Singleton<TGA_IO> tgaIO;
#endif	//Image_supports_tga

#if defined(Image_supports_tiff)
extern Common::Singleton<TIFF_IO> tiffIO;
#endif	//Image_supports_tiff

}
