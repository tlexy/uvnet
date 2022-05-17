#ifndef UVCORE_MEMORY_ALLOC_HELPER_H
#define UVCORE_MEMORY_ALLOC_HELPER_H

#include <stdlib.h>
#ifndef _WIN32
#include <jemalloc/jemalloc.h>
#endif

class MemoryAllocHelper
{
public:
	MemoryAllocHelper(int alloc_bytes)
	{
		_mem = (char*)malloc(alloc_bytes);
		_begin = 0;
		_end = alloc_bytes;
	}
	char* get_memory(int size)
	{
		if (_begin + size <= _end)
		{
			_begin += size;
			return _mem + _begin - size;
		}
		return NULL;
	}

	~MemoryAllocHelper()
	{
		_begin = 0;
		_end = 0;
		if (_mem)
		{
			free(_mem);
		}
		_mem = NULL;
	}

private:
	char* _mem;
	int _begin;
	int _end;
};

#endif