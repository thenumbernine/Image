#pragma once

//next time use std::filebuf?
class FileBuffer {

	char *data;
	int size;

	void reset();
public:

	FileBuffer() { reset(); }
	FileBuffer(const char *fn) {
		reset();
		load(fn);
	}

	~FileBuffer() {unload(); }

	char *getData() { return data; }
	int getSize() { return size; }
	void release() { reset(); } //without freeing.  otherwise the dtor will free it

	void unload();
	bool load(const char *fn);
};

