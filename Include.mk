IMAGE_PATH:=$(dir $(lastword $(MAKEFILE_LIST)))
include $(IMAGE_PATH)Config.mk
INCLUDE+=$(IMAGE_PATH)include
DEPEND_LIBS+=$(IMAGE_PATH)dist/$(PLATFORM)/$(BUILD)/$(LIB_PREFIX)Image$(LIB_SUFFIX)

ifdef IMAGE_SUPPORTS_FITS
DYNAMIC_LIBS_osx+=$(HOME)/lib/libcfitsio.5.3.41$(LIB_SUFFIX)
#DYNAMIC_LIBS_linux+=/usr/lib/x86_64-linux-gnu/libcfitsio$(LIB_SUFFIX)
LIBS_linux+=cfitsio
endif

ifdef IMAGE_SUPPORTS_PNG
DYNAMIC_LIBS_osx+=$(HOME)/lib/libpng16.16$(LIB_SUFFIX)
#DYNAMIC_LIBS_linux+=/usr/lib/x86_64-linux-gnu/libpng$(LIB_SUFFIX)
LIBS_linux+=png
endif

ifdef IMAGE_SUPPORTS_TIFF
DYNAMIC_LIBS_osx+=$(HOME)/lib/libtiff.5$(LIB_SUFFIX)
#DYNAMIC_LIBS_linux+=/usr/lib/x86_64-linux-gnu/libtiff$(LIB_SUFFIX)
LIBS_linux+=tiff
endif
