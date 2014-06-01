IMAGE_PATH:=$(dir $(lastword $(MAKEFILE_LIST)))

INCLUDE+=$(IMAGE_PATH)include
DYNAMIC_LIBS+=$(IMAGE_PATH)dist/$(PLATFORM)/$(BUILD)/libImage.dylib

