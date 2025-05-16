#pragma once

#include "Image/Config.h"
#include "Image/System.h"

#ifdef IMAGE_SUPPORTS_BMP
#include "Image/BMP_IO.h"
#endif	//IMAGE_SUPPORTS_BMP

#ifdef IMAGE_SUPPORTS_FITS
#include "Image/FITS_IO.h"
#endif	//IMAGE_SUPPORTS_FITS

#ifdef IMAGE_SUPPORTS_JPEG
#include "Image/JPEG_IO.h"
#endif	//IMAGE_SUPPORTS_JPEG

#ifdef IMAGE_SUPPORTS_PNG
#include "Image/PNG_IO.h"
#endif	//IMAGE_SUPPORTS_PNG

#ifdef IMAGE_SUPPORTS_PPM
#include "Image/PPM_IO.h"
#endif	//IMAGE_SUPPORTS_PPM

#ifdef IMAGE_SUPPORTS_TGA
#include "Image/TGA_IO.h"
#endif	//IMAGE_SUPPORTS_TGA

#ifdef IMAGE_SUPPORTS_TIFF
#include "Image/TIFF_IO.h"
#endif	//IMAGE_SUPPORTS_TIFF

namespace Image {

extern Common::Singleton<System> system;

#ifdef IMAGE_SUPPORTS_BMP
extern Common::Singleton<BMP_IO> bmpIO;
#endif	//IMAGE_SUPPORTS_BMP

#ifdef IMAGE_SUPPORTS_FITS
extern Common::Singleton<FITS_IO> fitsIO;
#endif	//IMAGE_SUPPORTS_FITS

#ifdef IMAGE_SUPPORTS_JPEG
extern Common::Singleton<JPEG_IO> jpegIO;
#endif	//IMAGE_SUPPORTS_JPEG

#ifdef IMAGE_SUPPORTS_PNG
extern Common::Singleton<PNG_IO> pngIO;
#endif	//IMAGE_SUPPORTS_PNG

#ifdef IMAGE_SUPPORTS_PPM
extern Common::Singleton<PPM_IO> ppmIO;
#endif	//IMAGE_SUPPORTS_PPM

#ifdef IMAGE_SUPPORTS_TGA
extern Common::Singleton<TGA_IO> tgaIO;
#endif	//IMAGE_SUPPORTS_TGA

#ifdef IMAGE_SUPPORTS_TIFF
extern Common::Singleton<TIFF_IO> tiffIO;
#endif	//IMAGE_SUPPORTS_TIFF

}
