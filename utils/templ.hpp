#ifndef GENERNAL_TEMPLATE_H
#define GENERNAL_TEMPLATE_H

#include <vector>

template <typename T, typename ITER, typename CompFunc>
bool binary_rearch(ITER begin, ITER end, const T& key, CompFunc func, T& out)
{
	int flag = 0;
	int index = 0;
	while (end > begin)
	{
		if (end - begin == 1)
		{
			bool flag = func(key, *begin);
			if (flag != 0)
			{
				return false;
			}
			out = *begin;
			return true;
		}
		//
		index = (end - begin) / 2;
		flag = func(key, *(begin + index));
		//
		if (flag == 0)
		{
			out = *(begin + index);
			return true;
		}
		else if (flag < 0)
		{
			end = begin + index;
		}
		else
		{
			begin = begin + index + 1;
		}
	}
	return false;
}

template <typename T, typename CompFunc>
void insert_binary_rearch(std::vector<T>& vecs, const T& key, CompFunc func)
{
	auto begin = vecs.begin();
	auto end = vecs.end();
	/*if (begin >= end)
	{
		return;
	}*/
	if (vecs.size() < 1)
	{
		vecs.push_back(key);
	}

	int flag = 0;
	int index = 0;
	while (end > begin)
	{
		if (end - begin == 1)
		{
			int flag = func(key, *begin);
			if (flag > 0)
			{
				vecs.insert(end, key);
			}
			else if(flag < 0)
			{
				vecs.insert(begin, key);
			}
			else
			{
				break;
			}
			return;
		}
		//
		index = (end - begin) / 2;
		flag = func(key, *(begin + index));
		if (flag == 0)
		{
			break;
		}
		else if (flag < 0)
		{
			end = begin + index;
		}
		else
		{
			begin = begin + index;
		}
	}
	
}

template<typename TYPE>
static void null_destory(TYPE* pointer)
{
}

#endif