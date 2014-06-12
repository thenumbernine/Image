IMAGE_PATH:=$(dir $(lastword $(MAKEFILE_LIST)))
INCLUDE+=$(IMAGE_PATH)include
DYNAMIC_LIBS+=$(IMAGE_PATH)dist/$(PLATFORM)/$(BUILD)/libImage.dylib
DYNAMIC_LIBS+=/usr/local/lib/libcfitsio.2.3.36.dylib
DYNAMIC_LIBS+=/usr/local/lib/libpng16.16.dylib
DYNAMIC_LIBS+=/usr/local/lib/libtiff.5.dylib
