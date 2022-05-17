#ifndef CREATOR_H
#define CREATOR_H

#include <stdlib.h>
#ifndef _WIN32
#include <jemalloc/jemalloc.h>
#endif

template<typename TYPE>
class Creator
{
private:
	static void destory(TYPE* pointer)
	{
		if (std::is_class<TYPE>::value) pointer->~TYPE();
		free(pointer);
	}
public:
	typedef std::shared_ptr<TYPE> PTR;
	static PTR create()
	{
		void* m = malloc(sizeof(TYPE));
		return PTR(new(m)TYPE(), destory);
	}

	template<typename arg1>
	static PTR create(arg1 a1)
	{
		void* m = malloc(sizeof(TYPE));
		return PTR(new(m)TYPE(a1), destory);
	}

	template<typename arg1, typename arg2>
	static PTR create(arg1 a1, arg2 a2)
	{
		void* m = malloc(sizeof(TYPE));
		return PTR(new(m)TYPE(a1, a2), destory);
	}

	template<typename arg1, typename arg2, typename arg3>
	static PTR create(arg1 a1, arg2 a2, arg3 a3)
	{
		void* m = malloc(sizeof(TYPE));
		return PTR(new(m)TYPE(a1, a2, a3), destory);//std::make_shared<TYPE>(a1, a2, a3); //
	}

	template<typename arg1, typename arg2, typename arg3, typename arg4>
	static PTR create(arg1 a1, arg2 a2, arg3 a3, arg4 a4)
	{
		void* m = malloc(sizeof(TYPE));
		return PTR(new(m)TYPE(a1, a2, a3, a4), destory);
	}

	template<typename arg1, typename arg2, typename arg3, typename arg4, typename arg5>
	static PTR create(arg1 a1, arg2 a2, arg3 a3, arg4 a4, arg5 a5)
	{
		void* m = malloc(sizeof(TYPE));
		return PTR(new(m)TYPE(a1, a2, a3, a4, a5), destory);
	}
};

#endif