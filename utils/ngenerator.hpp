#ifndef N_GENERATOR_H
#define N_GENERATOR_H

#include <stdint.h>
#include <vector>

#define M 1000000

#define MAX_KEEP_SIZE 1*M
#define MAX_COUNTER (INT64_MAX - 1)

template <typename TYPE, int cap>
class FixedPool
{
public:
	FixedPool()
		:_write_pos(0),
		_read_pos(0),
		_fixed_size(cap)
	{
		_vec.resize(cap);
	}
	void push(TYPE ele)
	{
		if (_write_pos >= _fixed_size)
		{
			return;
		}
		_vec[_write_pos] = ele;
		++_write_pos;
	}

	bool has_element()
	{
		return _read_pos < _write_pos;
	}

	TYPE pop()
	{
		return _vec[_read_pos++];
	}

	void reset()
	{
		_write_pos = 0;
		_read_pos = 0;
	}
private:
	std::vector<TYPE> _vec;
	size_t _write_pos;
	size_t _read_pos;
	size_t _fixed_size;
};

template <typename TYPE, int64_t MAX_NUMBER = MAX_COUNTER>
class NGenerator
{
public:
	NGenerator(TYPE start = 10000)
		:_counter(start)
	{
		if (start < 1)
		{
			_counter = 1;
		}
	}

	TYPE get_next()
	{
		if (_counter >= MAX_NUMBER)
		{
			return get_next_from_pool();
		}
		return ++_counter;
	}

	void recyle(TYPE ele)
	{
		_recyle_list.push(ele);
	}

private:
	TYPE get_next_from_pool()
	{
		if (_alloc_list.has_element())
		{
			return _alloc_list.pop();
		}
		else
		{
			//分配完了，怎么处理？
			if (_recyle_list.has_element())
			{
				_alloc_list = _recyle_list;
				_recyle_list.reset();
			}
			if (_alloc_list.has_element())
			{
				return _alloc_list.pop();
			}
		}
		return 0;//无效的ID
	}

private:
	TYPE _counter;

	FixedPool<TYPE, MAX_KEEP_SIZE> _recyle_list;
	FixedPool<TYPE, MAX_KEEP_SIZE> _alloc_list;
};

#endif