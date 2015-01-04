DIST_FILENAME=ImageLua
DIST_TYPE=lib
include ../../Common/Base.mk
include ../../Tensor/Include.mk
include ../Include.mk
include ../../LuaCxx/Include.mk		# only for the include dir paths

# get rid of the -install_name stuff
LDFLAGS_osx_lib=-dynamiclib -undefined suppress -flat_namespace -rpath . -install_name .
