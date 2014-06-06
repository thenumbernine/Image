#pragma once

#include "Tensor/Grid.h"
#include "Tensor/Vector.h"
#include "Common/Meta.h"
#include <type_traits>

namespace Image {

//image interface
struct IImage {
	virtual ~IImage() {};
	virtual Tensor::Vector<int,2> getSize() const = 0;
	virtual int getChannels() const = 0;
	virtual int getBitsPerPixel() const { return getChannels() << 3; }
	virtual char *getData() = 0;
	virtual const char *getData() const = 0;
};

template<typename Type_ = char>
struct ImageType : public IImage {
	typedef Type_ Type;
protected:
	Tensor::Vector<int,3> size;	//channels, width, height

	typedef Tensor::Grid<Type, 3> Grid;
	std::shared_ptr<Grid> grid;

public:
	ImageType(const Tensor::Vector<int,2> size_, void *data = NULL, int channels = 3) 
	{
		size(0) = channels;	//channels first so it is inner-most nested, so our images are interleaved rather than planar
		size(1) = size_(0);
		size(2) = size_(1);
		grid = std::make_shared<Grid>(size);
		//ugly
		if (data) {
			memcpy(grid->v, data, sizeof(Type) * size.volume());
		}
	}

	virtual ~ImageType() {}

	virtual Tensor::Vector<int,2> getSize() const { return Tensor::Vector<int,2>(size(1), size(2)); }
	virtual int getChannels() const { return size(0); }
	virtual int getBitsPerPixel() const { return getChannels() * sizeof(Type) << 3; }
	
	virtual char *getData() { return (char*)grid->v; }
	virtual const char *getData() const { return (char*)grid->v; }
	virtual Type *getDataType() { return grid->v; }
	virtual const Type *getDataType() const { return grid->v; }

	virtual std::shared_ptr<Grid> getGrid() { return grid; }
	virtual std::shared_ptr<const Grid> getGrid() const { return grid; }

	//it'd be nice to return a vector with size the # of channels
	// but channels is not a compile time variable, so you have to pick out your elements individually.
	// should it become one?
	Type &operator()(int i, int j, int ch = 0) { return (*grid)(Tensor::Vector<int,3>(ch,i,j)); }
	const Type &operator()(int i, int j, int ch = 0) const { return (*grid)(Tensor::Vector<int,3>(ch,i,j)); }
};

typedef struct ImageType<> Image;

};

