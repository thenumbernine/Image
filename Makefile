DIST_FILENAME=libImage.dylib
DIST_TYPE=dylib

include ../GLApp/Makefile.mk

CFLAGS_BASE+= -I../GLApp/include -I../TensorMath/include
