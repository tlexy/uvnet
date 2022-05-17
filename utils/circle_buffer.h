#ifndef UVCORE_CIRCLE_BUFFER_H
#define UVCORE_CIRCLE_BUFFER_H

#include <stdint.h>
#include <stddef.h>

/// <summary>
/// 基础环形队列
/// </summary>
class CircleBuffer
{
public:
	CircleBuffer(size_t size);

	uint8_t* enable_size(size_t size);
	uint8_t* write_ptr();
	uint8_t* read_ptr();

	void has_written(size_t size);
	void has_read(size_t size);

	size_t write(const uint8_t* buf, size_t len);
	size_t read(uint8_t* buf, size_t len);

	size_t usable_size();
	size_t writable_size();
	size_t readable_size();

	void reset();

	~CircleBuffer();

private:
	void rearrange();

private:
	uint8_t* _buffer{NULL};
	size_t _read_pos{0};
	size_t _write_pos{0};
	size_t _size{0};
};

/// <summary>
/// 一个实用的连续存取空间的队列
/// </summary>
class SequenceBuffer
{
public:
	SequenceBuffer(size_t size);

	virtual char* get_buffer();
	virtual int get_buffer_length();

	CircleBuffer* get_inner_buffer();

	void has_written(size_t len);
	void has_read(size_t len);

	virtual ~SequenceBuffer();

protected:
	CircleBuffer _buffer;
};

#endif
