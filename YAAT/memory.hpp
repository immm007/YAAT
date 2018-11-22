#pragma once
#include <Windows.h>
#include <boost/noncopyable.hpp>
#include <assert.h>
#include "iterator.hpp"


template<typename Header,typename T>
class SharedMemory:public boost::noncopyable
{
public:
	~SharedMemory()
	{
		UnmapViewOfFile(_view);
		CloseHandle(_handle);
	}
protected:
	void* _view;
	void* _handle;
	int _size;
};

template<typename Header,typename T>
class WriteSharedMemory : public SharedMemory<Header,T>
{
public:
	WriteSharedMemory(const char* name, int size)
	{
		assert(size % 4096 == 0);
		this->_handle = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, size, name);
		assert(this->_handle != nullptr);
		this->_view = MapViewOfFile(this->_handle, FILE_MAP_WRITE, 0, 0, 0);
		assert(this->_view != nullptr);

		this->_size = size;
	}
	WriteIterator<Header, T> begin()
	{
		return WriteIterator<Header, T>{this->_view};
	}
	WriteIterator<Header, T> end()
	{
		return WriteIterator<Header, T>{ reinterpret_cast<char*>(this->_view) + WriteIterator<Header, T>::length*(this->_size / WriteIterator<Header, T>::length) };
	}
};

template<typename Header, typename T>
class ReadSharedMemory : public SharedMemory<Header,T>
{
public:
	ReadSharedMemory(const char* name)
	{
		this->_handle = OpenFileMappingA(PAGE_READWRITE, false, name);
		assert(this->_handle != nullptr);
		this->_view = MapViewOfFile(this->_handle, FILE_MAP_READ, 0, 0, 0);
		assert(this->_view != nullptr);

		MEMORY_BASIC_INFORMATION info;
		VirtualQuery(this->_view, &info, sizeof(MEMORY_BASIC_INFORMATION));

		this->_size = info.RegionSize;
	}
	const ReadIterator<Header, T> cbegin() const
	{
		return ReadIterator<Header, T>{this->_view};
	}
};