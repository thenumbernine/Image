IMAGE_PATH:=$(dir $(lastword $(MAKEFILE_LIST)))

include $(IMAGE_PATH)Config.mk

INCLUDE+=$(IMAGE_PATH)include

DYNAMIC_LIBS+=$(IMAGE_PATH)dist/$(PLATFORM)/$(BUILD)/libImage$(LIB_SUFFIX)

ifdef IMAGE_SUPPORT_FITS
DYNAMIC_LIBS+=$(HOME)/lib/libcfitsio.2.3.37$(LIB_SUFFIX)
endif
ifdef IMAGE_SUPPORT_PNG
DYNAMIC_LIBS+=$(HOME)/lib/libpng17.17$(LIB_SUFFIX)
endif
ifdef IMAGE_SUPPORT_TIFF
DYNAMIC_LIBS+=/usr/local/lib/libtiff.5$(LIB_SUFFIX)
endif
