#pragma once

#include "TensorMath/Meta.h"
#include "TensorMath/Grid.h"
#include "TensorMath/Vector.h"
#include <type_traits>

namespace Image {

struct Image {
	virtual Vector<int,2> getSize() const = 0;
	virtual int getChannels() const = 0;
	virtual int getBitsPerPixel() const { return getChannels() << 3; }
	virtual char *getData() = 0;
	virtual const char *getData() const = 0;
};

template<typename Type_ = char>
struct ImageType : public Image {
	typedef Type_ Type;
protected:
	Vector<int,3> size;	//width, height, channels

	typedef ::Grid<Type, 3> Grid;
	Grid *grid;

public:
	ImageType(const Vector<int,2> size_, void *data = NULL, int channels = 3) 
	: grid(NULL)
	{
		size(0) = size_(0);
		size(1) = size_(1);
		size(2) = channels;
		grid = new Grid(size);
		//ugly
		if (data) {
			memcpy(grid->v, data, sizeof(Type) * size.volume());
		}
	}

	~ImageType() {
		delete grid;
	}

	virtual Vector<int,2> getSize() const { return Vector<int,2>(size(0), size(1)); }
	virtual int getChannels() const { return size(2); }
	virtual int getBitsPerPixel() const { return getChannels() * sizeof(Type); }
	
	virtual char *getData() { return (char*)grid->v; }
	virtual const char *getData() const { return (char*)grid->v; }
	virtual Type *getDataType() { return grid->v; }
	virtual const Type *getDataType() const { return grid->v; }
};

};

