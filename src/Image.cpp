// I used to get by with these in their respective cpp files, but on vs10 this isn't working...
#include "Image/Image.h"

namespace Image {

Common::Singleton<System> system;

#if defined(Image_supports_bmp)
Common::Singleton<BMP_IO> bmpIO;
#endif	//Image_supports_bmp

#if defined(Image_supports_fits)
Common::Singleton<FITS_IO> fitsIO;
#endif	//Image_supports_fits

#if defined(Image_supports_jpeg)
Common::Singleton<JPEG_IO> jpegIO;
#endif	//Image_supports_jpeg

#if defined(Image_supports_png)
Common::Singleton<PNG_IO> pngIO;
#endif	//Image_supports_png

#if defined(Image_supports_ppm)
Common::Singleton<PPM_IO> ppmIO;
#endif	//Image_supports_ppm

#if defined(Image_supports_tga)
Common::Singleton<TGA_IO> tgaIO;
#endif	//Image_supports_tga

#if defined(Image_supports_tiff)
Common::Singleton<TIFF_IO> tiffIO;
#endif	//Image_supports_tiff

}
