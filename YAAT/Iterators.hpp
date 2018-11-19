#pragma once
#include <iterator>
#include "Frame.hpp"


template<typename T>
class WriteIterator:public std::iterator<std::output_iterator_tag,T>
{
public:
	WriteIterator(void * start) :_pos{ reinterpret_cast<char*>(start) }
	{

	}
	WriteIterator(char * start) :_pos{ start }
	{

	}
	//复制一个Frame开销很小的，等于复制指针
	inline Frame<T> operator*() const
	{
		return Frame<T>{_pos};
	}
	inline WriteIterator& operator++()
	{
		_pos += Frame<T>::length;
		return *this;
	}
	inline bool operator==(const WriteIterator<T> second)
	{
		return _pos == second._pos;
	}
	inline bool operator!=(const WriteIterator<T> second)
	{
		return _pos != second._pos;
	}
private:
	char* _pos;
};

template<typename T>
class ReadIterator :public std::iterator<std::bidirectional_iterator_tag, T>
{
public:
	ReadIterator(void * start) :_pos{ reinterpret_cast<char*>(start) }
	{

	}
	ReadIterator(char * start) :_pos{ start }
	{

	}
	inline const T* operator*() const
	{
		return Frame<T>{_pos}.data();
	}
	inline ReadIterator& operator++()
	{
		_pos += Frame<T>::length;
		return *this;
	}
	inline bool operator==(const ReadIterator<T> second)
	{
		return _pos == second._pos;
	}
	inline bool operator!=(const ReadIterator<T> second)
	{
		return _pos != second._pos;
	}
private:
	char* _pos;
};