#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "Image/FileBuffer.h"

void FileBuffer::reset() {
	data = NULL;
	size = 0;
}

void FileBuffer::unload() {
	if (data) delete data;
	reset();
}

bool FileBuffer::load(const char *fn) {
	unload();
	assert(fn);
	assert(!data);
	assert(!size);
	
	FILE *fp = fopen(fn,"rb");
	if (!fp) return false;
	
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	rewind(fp);
	data = new char[size+1];
	if (data) {
		data[size] = '\0';

		int readsize = (int)fread(data,1,size,fp);
		if (readsize != size) {
			printf("got a size %d readsize %d from file %s\n", size, readsize, fn);
		}
		data[readsize] = '\0';
	}

	fclose(fp);

	return !!data;
}
