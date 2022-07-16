#include "circle_buffer.h"
#include <stdlib.h>
#include "global.h"
#include<algorithm>

CircleBuffer::CircleBuffer(size_t size)
	:_size(size),
	_read_pos(0),
	_write_pos(0)
{
	_buffer = (uint8_t*)malloc(size);
}

uint8_t* CircleBuffer::enable_size(size_t size)
{
	if (size > _size)
	{
		return NULL;
	}
	if (usable_size() < size)
	{
		return NULL;
	}
	if (writable_size() < size)
	{
		rearrange();
	}
	return write_ptr();
}

uint8_t* CircleBuffer::write_ptr()
{
	return _buffer + _write_pos;
}

uint8_t* CircleBuffer::read_ptr()
{
	return _buffer + _read_pos;
}

size_t CircleBuffer::readable_size()
{
	return _write_pos - _read_pos;
}

size_t CircleBuffer::usable_size()
{
	size_t use = _write_pos - _read_pos;
	if (_size >= use)
	{
		return _size - use;
	}
	return 0;
}

size_t CircleBuffer::writable_size()
{
	return _size - _write_pos;
}

void CircleBuffer::rearrange()
{
	std::copy(_buffer + _read_pos, _buffer + _write_pos, _buffer);
	_write_pos = _write_pos - _read_pos;
	_read_pos = 0;
}

void CircleBuffer::has_written(size_t size)
{
	if (_write_pos + size <= _size)
	{
		_write_pos += size;
	}
}

void CircleBuffer::has_read(size_t size)
{
	if (_read_pos + size <= _write_pos)
	{
		_read_pos += size;
	}
}

void CircleBuffer::reset()
{
	_read_pos = 0;
	_write_pos = 0;
}

size_t CircleBuffer::write(const uint8_t* buf, size_t len)
{
	if (writable_size() >= len)
	{
		std::copy(buf, buf + len, write_ptr());
		has_written(len);
		return len;
	}
	else
	{
		enable_size(len);
		if (writable_size() >= len)
		{
			std::copy(buf, buf + len, write_ptr());
			has_written(len);
			return len;
		}
	}
	return 0;
}

size_t CircleBuffer::read(uint8_t* buf, size_t len)
{
	if (len <= readable_size())
	{
		std::copy(read_ptr(), read_ptr() + len, buf);
		return len;
	}
	return 0;
}

CircleBuffer::~CircleBuffer()
{
	if (_buffer)
	{
		free(_buffer);
		_buffer = NULL;
	}
}

////////////////////////////////////////-----SequenceBuffer

SequenceBuffer::SequenceBuffer(size_t size)
	:_buffer(size)
{

}

char* SequenceBuffer::get_buffer()
{
	return (char*)_buffer.write_ptr();
}

int SequenceBuffer::get_buffer_length()
{
	return _buffer.writable_size();
}

CircleBuffer* SequenceBuffer::get_inner_buffer()
{
	return &_buffer;
}

void SequenceBuffer::has_written(size_t len)
{
	_buffer.has_written(len);
}

void SequenceBuffer::has_read(size_t len)
{
	_buffer.has_read(len);
}

SequenceBuffer::~SequenceBuffer()
{}