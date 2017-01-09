DIST_FILENAME=Image
DIST_TYPE=lib

include ../Common/Base.mk
include ../Tensor/Include.mk
include Config.mk

ifdef IMAGE_SUPPORT_BMP
MACROS+=Image_supports_bmp
endif
ifdef IMAGE_SUPPORT_FITS
MACROS+=Image_supports_fits
endif
ifdef IMAGE_SUPPORT_JPEG
MACROS+=Image_supports_jpeg
endif
ifdef IMAGE_SUPPORT_PNG
MACROS+=Image_supports_png
endif
ifdef IMAGE_SUPPORT_PPM
MACROS+=Image_supports_ppm
endif
ifdef IMAGE_SUPPORT_TGA
MACROS+=Image_supports_tga
endif
ifdef IMAGE_SUPPORT_TIFF
MACROS+=Image_supports_tiff
endif
