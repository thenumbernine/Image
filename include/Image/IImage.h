#pragma once

#include "Tensor/Grid.h"
#include "Common/Meta.h"
#include <string.h>
#include <memory>
#include <type_traits>

namespace Image {

//image interface
struct IImage {
	virtual ~IImage() {};
	virtual ::Tensor::int2 getSize() const = 0;
	virtual int getChannels() const = 0;
	virtual int getPlanes() const = 0;
	virtual int getBitsPerPixel() const { return getChannels() << 3; }
	virtual char *getData() = 0;
	virtual const char *getData() const = 0;
	virtual size_t getDataSize() = 0;
};

template<typename Type_ = uint8_t>
struct ImageType : public IImage {
	using Type = Type_;
protected:
	::Tensor::int4 size;	//channels, width, height, planes

	using Grid = Tensor::Grid<Type, 4>;
	std::shared_ptr<Grid> grid;

public:
	ImageType(const ::Tensor::int2 size_, void *data = {}, int channels = 3, int planes = 1)
	: size(channels, size_.x, size_.y, planes)
	{
		grid = std::make_shared<Grid>(size);
		//ugly
		if (data) {
			memcpy(grid->v, data, getDataSize());
		}
	}

	virtual ~ImageType() {}

	virtual ::Tensor::int2 getSize() const { return size.subset<2,1>(); }
	virtual int getChannels() const { return size.x; }
	virtual int getPlanes() const { return size.w; }
	virtual int getBitsPerPixel() const { return getChannels() * sizeof(Type) << 3; }
	
	virtual char *getData() { return (char*)grid->v; }
	virtual const char *getData() const { return (char*)grid->v; }
	virtual Type *getDataType() { return grid->v; }
	virtual const Type *getDataType() const { return grid->v; }
	virtual size_t getDataSize() { return sizeof(Type) * size.product(); }

	virtual std::shared_ptr<Grid> getGrid() { return grid; }
	virtual std::shared_ptr<const Grid> getGrid() const { return grid; }

	//it'd be nice to return a vector with size the # of channels
	// but channels is not a compile time variable, so you have to pick out your elements individually.
	// should it become one?
	Type &operator()(int i, int j, int ch = 0, int pl = 0) { return (*grid)(::Tensor::int4(ch,i,j,pl)); }
	const Type &operator()(int i, int j, int ch = 0, int pl = 0) const { return (*grid)(::Tensor::int4(ch,i,j,pl)); }
};

using Image = ImageType<>;

};
