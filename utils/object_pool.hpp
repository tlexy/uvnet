#ifndef UVCORE_OBJECT_POOL_H
#define UVCORE_OBJECT_POOL_H

#include <stdint.h>
#include <stddef.h>
#include "ns_helper.h"
#include <array>
#include <memory>
#include <list>

NS_UVCORE_B

template <typename T, int size = 100>
class FixedObjectPool
{
public:
	using ptr_type = std::shared_ptr<T>;

	template<class ...Args>
	ptr_type alloc(Args... args)
	{
		if (_read_idx == _write_idx && _read_idx < _arr.size())
		{
			ptr_type ptr = std::make_shared<T>(args...);
			_arr[_write_idx++] = ptr;
			return _arr[_read_idx++];
		}
		else if (_write_idx > _read_idx)
		{
			return _arr[_read_idx++];
		}
		else
		{
			ptr_type ptr = ptr_type();
			return ptr;
		}
	}

	void recycle(ptr_type ptr)
	{
		if (_read_idx > 0 && _read_idx < _arr.size())
		{
			_arr[--_read_idx] = ptr;
		}
	}
private:
	std::array<ptr_type, size> _arr;
	int _read_idx{0};
	int _write_idx{0};
};

template <typename T>
class ObjectPool
{
public:
	using ptr_type = std::shared_ptr<T>;

	template<class ...Args>
	ptr_type alloc(Args... args)
	{
		ptr_type ptr;
		if (_list.begin() != _list.end())
		{
			ptr = (*_list.begin());
			_list.erase(_list.begin());
			return ptr;
		}
		else
		{
			ptr_type ptr = std::make_shared<T>(args...);
			return ptr;
		}
	}

	void recycle(ptr_type ptr)
	{
		_list.push_back(ptr);
	}
private:
	std::list<ptr_type> _list;
};

NS_UVCORE_E

#endif