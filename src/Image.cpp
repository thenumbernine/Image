// I used to get by with these in their respective cpp files, but on vs10 this isn't working...
#include "Image/Image.h"

namespace Image {

Common::Singleton<System> system;

#if IMAGE_SUPPORTS_BMP
Common::Singleton<BMP_IO> bmpIO;
#endif	//IMAGE_SUPPORTS_BMP

#if IMAGE_SUPPORTS_FITS
Common::Singleton<FITS_IO> fitsIO;
#endif	//IMAGE_SUPPORTS_FITS

#if IMAGE_SUPPORTS_JPEG
Common::Singleton<JPEG_IO> jpegIO;
#endif	//IMAGE_SUPPORTS_JPEG

#if IMAGE_SUPPORTS_PNG
Common::Singleton<PNG_IO> pngIO;
#endif	//IMAGE_SUPPORTS_PNG

#if IMAGE_SUPPORTS_PPM
Common::Singleton<PPM_IO> ppmIO;
#endif	//IMAGE_SUPPORTS_PPM

#if IMAGE_SUPPORTS_TGA
Common::Singleton<TGA_IO> tgaIO;
#endif	//IMAGE_SUPPORTS_TGA

#if IMAGE_SUPPORTS_TIFF
Common::Singleton<TIFF_IO> tiffIO;
#endif	//IMAGE_SUPPORTS_TIFF

}
