/*
desired operation:

-- loading/saving
im = img.load('input.png')
im:save('output.png')

im = img.new{width=width, height=height[, channels=channels][, color=0xargb][, format])

-- resizing
width, height, channels = im:size()
im:size{width=newWidth, height=newHeight[, channels=newChannels]}

-- cloning
im2 = im:clone()

-- changing channel format
t = im:format()
im:format('byte')	-- 'byte', 'short', 'int', 'float', 'double' ... bits? 16-bit has sub-byte formats ...

-- pixel access
im(x,y, r,g,b)
r,g,b,... = im(x,y)

-- iterator
for x,y,r,g,b,... in im:iter() do
	...
end

I'm going to do float format across the board for starters
I'll switch to something else later
*/

#include <lua.hpp>
#include "Common/Macros.h"
#include "Image/System.h"
#include "Tensor/Vector.h"

static int imgref = LUA_REFNIL;

unsigned char* getImageDataOffset(Image::IImage* image, Tensor::Vector<int,2> offset) {
	return (unsigned char *)image->getData() + image->getChannels() * (offset(0) + image->getSize()(0) * offset(1));
}


template<typename T> T getParam(lua_State *L, int table, const char *fieldName);

template<> int getParam<int>(lua_State *L, int table, const char *fieldName) {
	lua_getfield(L, table, fieldName);
	if (!lua_isnumber(L, -1)) luaL_error(L, "getParam %s expected a number but found %s", fieldName, lua_typename(L, lua_type(L, -1)));
	return lua_tointeger(L, -1);
}

template<> void *getParam<void*>(lua_State *L, int table, const char *fieldName) {
	lua_getfield(L, table, fieldName);
	if (!lua_isuserdata(L, -1)) luaL_error(L, "getParam %s expected userdata but found %s", fieldName, lua_typename(L, lua_type(L, -1)));
	return lua_touserdata(L, -1);
}

template<typename T> T getDefParam(lua_State *L, int table, const char *fieldName, const T& def);

template<> int getDefParam(lua_State *L, int table, const char *fieldName, const int &def) {
	lua_getfield(L, table, fieldName);
	if (lua_isnil(L, -1)) return def;
	if (!lua_isnumber(L, -1)) luaL_error(L, "getDefParam %s expected a number but found %s", fieldName, lua_typename(L, lua_type(L, -1)));
	return lua_tointeger(L, -1);
}

//im = img.new{width=w, height=h, [channels=c]}
static int binding_new(lua_State *L) {
	int argc = lua_gettop(L);
	if (argc < 1) luaL_error(L, "expected {width=w, height=h, [channels=3, format='byte']");
	int width = getParam<int>(L, 1, "width");
	int height = getParam<int>(L, 1, "height");
	int channels = getDefParam<int>(L, 1, "channels", 3);

	lua_newtable(L);
	lua_rawgeti(L, LUA_REGISTRYINDEX, imgref);
	lua_setmetatable(L, -2);
	
	std::shared_ptr<Image::IImage>** ptr = (std::shared_ptr<Image::IImage>**)lua_newuserdata(L, sizeof(std::shared_ptr<Image::IImage>*));	//1) allocate the Lua gc system
	*ptr = new std::shared_ptr<Image::IImage>();	//2) allocate in the STL gc system
	lua_setfield(L, -2, "ptr");
	
	**ptr = std::make_shared<Image::Image>(Tensor::Vector<int,2>(width, height), nullptr, channels << 3);

	return 1;
}

//im = img.load(filename)
static int binding_load(lua_State *L) {
	int argc = lua_gettop(L);
	if (argc < 1) luaL_error(L, "expected filename");
	const char *filename = lua_tostring(L, 1);
	if (!filename) luaL_error(L, "expected filename to be a string");
	
	lua_newtable(L);	//{}
	lua_rawgeti(L, LUA_REGISTRYINDEX, imgref);
	lua_setmetatable(L, -2);
	
	std::shared_ptr<Image::IImage>** ptr = (std::shared_ptr<Image::IImage>**)lua_newuserdata(L, sizeof(std::shared_ptr<Image::IImage>*));	//{} Image::IImage*
	*ptr = new std::shared_ptr<Image::IImage>();
	lua_setfield(L, -2, "ptr");	//{} ({}.ptr = Image::IImage*)

	char errbuf[2048] = {0};
	try {
		**ptr = Image::system->read(filename);
	} catch (const std::exception &e) {
		strncpy(errbuf, e.what(), sizeof(errbuf));
		errbuf[sizeof(errbuf)-1] = 0;
	}
	if (*errbuf) luaL_error(L, "%s", errbuf);

	return 1;
}

//im:save(filename)
static int binding_save(lua_State *L) {
	int argc = lua_gettop(L);
	if (argc < 2) luaL_error(L, "expected image, filename");
	
	if (!lua_istable(L, 1)) luaL_error(L, "expected param 1 to be an image");
	std::shared_ptr<Image::IImage>** ptr = (std::shared_ptr<Image::IImage>**)getParam<void*>(L, 1, "ptr");
	if (!*ptr) luaL_error(L, "expected ptr to be non-null");

	const char *filename = lua_tostring(L, 2);
	if (!filename) luaL_error(L, "expected filename to be a string");

	char errbuf[2048] = {0};
	try {
		Image::system->write(filename, **ptr);
	} catch (const std::exception &e) {
		strncpy(errbuf, e.what(), sizeof(errbuf));
		errbuf[sizeof(errbuf)-1] = 0;
	}
	if (*errbuf) luaL_error(L, "%s", errbuf);

	return 0;
}

//width, height, channels = im:size(newwidth, newheight)
static int binding_size(lua_State *L) {
	int argc = lua_gettop(L);
	if (argc < 1) luaL_error(L, "expected img.size(imgobj[, width, height)");
	
	if (!lua_istable(L, 1)) luaL_error(L, "expected a table for param 1");
	std::shared_ptr<Image::IImage>** ptr = (std::shared_ptr<Image::IImage>**)getParam<void*>(L, 1, "ptr");
	if (!*ptr) luaL_error(L, "expected ptr to be non-null");
	
	Tensor::Vector<int,2> oldSize = (**ptr)->getSize();
	int oldChannels = (**ptr)->getBitsPerPixel() >> 3;	//TODO - we're assuming 8 bits per channel!

#if 0		//not yet
	if (argc >= 3) {
		Tensor::Vector<int,2> newSize;
		newSize(0) = lua_tonumber(L, 2);
		newSize(1) = lua_tonumber(L, 3);
		(**ptr)->resize(newSize);
	}
#endif
	
	lua_pushnumber(L, oldSize(0));
	lua_pushnumber(L, oldSize(1));
	lua_pushnumber(L, oldChannels);
	return 3;
}

//x,y,resample
static int binding_resample(lua_State *L) {
	int argc = lua_gettop(L);
	if (argc < 3) luaL_error(L, "expected img.resample(imgobj, width, height[, sampling])");
	
	if (!lua_istable(L, 1)) luaL_error(L, "expected a table for param 1");
	std::shared_ptr<Image::IImage>** srcImg = (std::shared_ptr<Image::IImage>**)getParam<void*>(L, 1, "ptr");
	if (!*srcImg) luaL_error(L, "expected ptr to be non-null");
	
	if (!lua_isnumber(L, 2)) luaL_error(L, "width should be a number");
	if (!lua_isnumber(L, 3)) luaL_error(L, "height should be a number");
	if (argc >= 4 && !lua_isstring(L, 4)) luaL_error(L, "sampling should be a string");

	if ((**srcImg)->getBitsPerPixel() & 7) luaL_error(L, "can only resample images whose bits-per-pixel is a factor of 8");
	int channels = (**srcImg)->getBitsPerPixel() >> 3;
	
	Tensor::Vector<int,2> newSize;
	newSize(0) = lua_tonumber(L, 2);
	newSize(1) = lua_tonumber(L, 3);

	bool resampleLinear = false;
	bool resampleCubic = false;
	if (argc >= 4) {
		const char *samplingStr = lua_tostring(L, 4);
		if (!strcasecmp(samplingStr, "linear")) {
			resampleLinear = true;
		} else if (!strcasecmp(samplingStr, "cubic")) {
			resampleCubic = true;
		}
	}
	
	lua_newtable(L);
	lua_rawgeti(L, LUA_REGISTRYINDEX, imgref);
	lua_setmetatable(L, -2);
	std::shared_ptr<Image::IImage>** dstImg = (std::shared_ptr<Image::IImage>**)lua_newuserdata(L, sizeof(std::shared_ptr<Image::IImage>*));
	*dstImg = new std::shared_ptr<Image::IImage>();
	lua_setfield(L, -2, "ptr");
	**dstImg = std::make_shared<Image::Image>(newSize, nullptr, channels << 3);
	
	if (resampleLinear) {
		for (int dstj = 0; dstj < newSize(1); dstj++) {
			float srcjfloat = (float)dstj / (float)newSize(1) * (float)(**srcImg)->getSize()(1);
			int srcj = (int)floor(srcjfloat);
			float srcjfrac = srcjfloat - (float)srcj;
			for (int dsti = 0; dsti < newSize(0); dsti++) {
				float srcifloat = (float)dsti / (float)newSize(0) * (float)(**srcImg)->getSize()(0);
				int srci = (int)floor(srcifloat);
				float srcifrac = srcifloat - (float)srci;
				
				unsigned char *srcp[2][2];
				for (int di = 0; di < 2; di++) {
					for (int dj = 0; dj < 2; dj++) {
						srcp[di][dj] = getImageDataOffset((**srcImg).get(), Tensor::Vector<int,2>( (srci+di)%(**srcImg)->getSize()(0) , (srcj+dj)%(**srcImg)->getSize()(1) ));
					}
				}
				unsigned char *dstp = getImageDataOffset((**dstImg).get(), Tensor::Vector<int,2>(dsti, dstj));
				for (int k = 0; k < channels; k++) {
					dstp[k] = (unsigned char)(srcp[0][0][k] * (1.f - srcifrac) * (1.f - srcjfrac)
						+ srcp[1][0][k] * srcifrac * (1.f - srcjfrac)
						+ srcp[0][1][k] * (1.f - srcifrac) * srcjfrac
						+ srcp[1][1][k] * srcifrac * srcjfrac);
				}
			}
		}
	} else {
		for (int dstj = 0; dstj < newSize(1); dstj++) {
			float srcjfloat = (float)dstj / (float)newSize(1) * (float)(**srcImg)->getSize()(1);
			int srcj = (int)floor(srcjfloat);
			for (int dsti = 0; dsti < newSize(0); dsti++) {
				float srcifloat = (float)dsti / (float)newSize(0) * (float)(**srcImg)->getSize()(0);
				int srci = (int)floor(srcifloat);
				
				unsigned char *dstp = getImageDataOffset((**dstImg).get(), Tensor::Vector<int,2>(dsti,dstj));
				unsigned char *srcp = getImageDataOffset((**srcImg).get(), Tensor::Vector<int,2>(srci,srcj));
				for (int k = 0; k < channels; k++) {
					dstp[k] = srcp[k];
				}
			}
		}
	}
	
	return 1;
}

static int imgIterFunc(lua_State *L) {
	//state obj, var table
	lua_getfield(L, 1, "x");
	int x = lua_tointeger(L, -1);
	lua_pop(L,1);
	lua_getfield(L, 1, "y");
	int y = lua_tointeger(L, -1);
	lua_pop(L,1);

	lua_getfield(L, 1, "img");
	if (!lua_istable(L, -1)) luaL_error(L, "expected a table for param 1");
	std::shared_ptr<Image::IImage>** ptr = (std::shared_ptr<Image::IImage>**)getParam<void*>(L, -1, "ptr");
	if (!*ptr) luaL_error(L, "expected ptr to be non-null");

	x++;
	if (x >= (**ptr)->getSize()(0)) {
		x = 0;
		y++;
		if (y >= (**ptr)->getSize()(1)) {
			y = 0;
			lua_pushinteger(L, 0);
			lua_setfield(L, 1, "y");
			lua_pushinteger(L, 0);
			lua_setfield(L, 1, "x");
			lua_pushnil(L);
			return 1;
		}
		lua_pushinteger(L, y);
		lua_setfield(L, 1, "y");
	}
	lua_pushinteger(L, x);
	lua_setfield(L, 1, "x");

	lua_pushinteger(L, x);
	lua_pushinteger(L, y);
	return 2;
}

//this runs slower than the lua impl
//maybe because of all the lua->c call overhead?
static int binding_iter(lua_State *L) {
	int argc = lua_gettop(L);
	if (argc < 1) luaL_error(L, "expected img.iter(imgobj)");
	
	if (!lua_istable(L, 1)) luaL_error(L, "expected a table for param 1");
	
	lua_pushcfunction(L, imgIterFunc);
	lua_newtable(L);
	lua_pushinteger(L, -1);
	lua_setfield(L, -2, "x");
	lua_pushinteger(L, 0);
	lua_setfield(L, -2, "y");
	lua_pushvalue(L, 1);
	lua_setfield(L, -2, "img");
	lua_pushnil(L);
	return 3;
}

static int binding_data(lua_State *L) {
	int argc = lua_gettop(L);
	if (argc < 1) luaL_error(L, "expected img");

	if (!lua_istable(L, 1)) luaL_error(L, "expected a table for param 1");
	std::shared_ptr<Image::IImage>** ptr = (std::shared_ptr<Image::IImage>**)getParam<void*>(L, 1, "ptr");
	if (!*ptr) luaL_error(L, "expected ptr to be non-null");
	
	lua_pushlstring(L, (char*)(**ptr)->getData(), (**ptr)->getDataSize());
	return 1;
}

static int binding_dataptr(lua_State *L) {
	int argc = lua_gettop(L);
	if (argc < 1) luaL_error(L, "expected img");

	if (!lua_istable(L, 1)) luaL_error(L, "expected a table for param 1");
	std::shared_ptr<Image::IImage>** ptr = (std::shared_ptr<Image::IImage>**)getParam<void*>(L, 1, "ptr");
	if (!*ptr) luaL_error(L, "expected ptr to be non-null");
	
	lua_pushlightuserdata(L, (**ptr)->getData());
	return 1;
}

static int binding___call(lua_State *L) {
	int argc = lua_gettop(L);
	if (argc < 1) luaL_error(L, "expected img");

	if (!lua_istable(L, 1)) luaL_error(L, "expected a table for param 1");
	std::shared_ptr<Image::IImage>** ptr = (std::shared_ptr<Image::IImage>**)getParam<void*>(L, 1, "ptr");
	if (!*ptr) luaL_error(L, "expected ptr to be non-null");

	if (argc < 3) luaL_error(L, "expected img(x,y, ...)");
	
	int channels = (**ptr)->getBitsPerPixel() >> 3;	//TODO - remove assumption of 8 bits per channel

	//TODO ugly
	unsigned char oldPixel[10];
	assert(sizeof(oldPixel) >= channels);
	
	Tensor::Vector<int,2> tc;
	tc(0) = lua_tonumber(L, 2);
	tc(1) = lua_tonumber(L, 3);
	tc(0) %= (**ptr)->getSize()(0);
	tc(1) %= (**ptr)->getSize()(1);
	tc += (**ptr)->getSize(); 
	tc(0) %= (**ptr)->getSize()(0);
	tc(1) %= (**ptr)->getSize()(1);

	memcpy(oldPixel, getImageDataOffset((**ptr).get(), tc), channels);
	
	if (argc > 3) {
		unsigned char newPixel[sizeof(oldPixel)];
		int ch = argc - 3;
		if (ch > sizeof(newPixel)) ch = sizeof(newPixel);
		for (int i = 0; i < ch; i++) {
			float v = lua_tonumber(L, i+4);
			if (v < 0) v = 0;
			if (v > 1) v = 1;
			newPixel[i] = v * 255.f;
		}
		memcpy(getImageDataOffset((**ptr).get(), tc), newPixel, channels);
	}

	for (int i = 0; i < channels; i++) {
		lua_pushnumber(L, (float)oldPixel[i] / 255.f);
	}
	return channels;
}

static int binding___gc(lua_State *L) {

	int argc = lua_gettop(L);
	if (argc < 1) luaL_error(L, "expected img");
	
	if (!lua_istable(L, 1)) luaL_error(L, "expected a table for param 1");
	Image::IImage **ptr = (Image::IImage**)getParam<void*>(L, 1, "ptr");
	if (!*ptr) luaL_error(L, "expected ptr to be non-null");
	
	delete *ptr;
	*ptr = nullptr;

	return 0;
}

#define BINDING(x)	{#x, binding_##x}
static luaL_Reg bindings[] = {
	BINDING(new),
	BINDING(load),
	BINDING(save),
	BINDING(size),
	BINDING(resample),
	BINDING(iter),
	BINDING(data),
	BINDING(dataptr),
	BINDING(__gc),
	BINDING(__call),
};

extern "C" {
#ifdef WIN32
__declspec(dllexport)
#endif
int luaopen_libImageLua(lua_State *L) {
	lua_newtable(L);	//img
	for (int i = 0; i < numberof(bindings); ++i) {
		lua_pushcfunction(L, bindings[i].func);	//img func
		lua_setfield(L, lua_gettop(L)-1, bindings[i].name);	//img, img[name] = func
	}
	//img
	
	lua_pushvalue(L, -1);	//stack: img img
	lua_setfield(L, -2, "__index");	//img.__index = img, stack : img
	
	lua_pushvalue(L, -1);	//stack: img img
	imgref = luaL_ref(L, LUA_REGISTRYINDEX);	//registery[imgref] = img, stack : img
	
	return 1;
}
};

