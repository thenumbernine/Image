DIST_FILENAME=Image
DIST_TYPE=lib
include ../Common/Base.mk
include ../Tensor/Include.mk
MACROS+=SUPPORT_BMP
MACROS+=SUPPORT_FITS
MACROS+=SUPPORT_JPEG
MACROS+=SUPPORT_PNG
MACROS+=SUPPORT_PPM
MACROS+=SUPPORT_TGA
MACROS+=SUPPORT_TIFF
