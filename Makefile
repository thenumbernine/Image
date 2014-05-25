# PLATFORM: osx
# BUILD: debug, release

DIST_FILENAME=libimage.dylib
DIST_TYPE=dylib

include ../GLApp/Makefile.mk

CFLAGS_BASE+= -I../GLApp/include -I../TensorMath/include
