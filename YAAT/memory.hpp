#pragma once
#include <Windows.h>
#include <boost/noncopyable.hpp>
#include <assert.h>
#include "iterator.hpp"


template<typename Header,typename T>
class SharedMemory:public boost::noncopyable
{
public:
	static constexpr int length = sizeof(Header) + sizeof(T);

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
class SMForWrite : public SharedMemory<Header,T>
{
public:
	SMForWrite(const char* name, int size)
	{
		assert(size % 4096 == 0);
		this->_handle = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, size, name);
		assert(this->_handle != nullptr);
		this->_view = MapViewOfFile(this->_handle, FILE_MAP_WRITE, 0, 0, 0);
		assert(this->_view != nullptr);

		this->_size = size;
	}

	SMWriteIterator<Header, T> begin()
	{
		return SMWriteIterator<Header, T>{this->_view};
	}

	SMWriteIterator<Header, T> end()
	{
		return SMWriteIterator<Header, T>{ reinterpret_cast<char*>(this->_view) + this->length*(this->_size / this->length) };
	}
};

template<typename Header, typename T>
class SMForRead : public SharedMemory<Header,T>
{
public:
	SMForRead(const char* name)
	{
		this->_handle = OpenFileMappingA(PAGE_READWRITE, false, name);
		assert(this->_handle != nullptr);
		this->_view = MapViewOfFile(this->_handle, FILE_MAP_READ, 0, 0, 0);
		assert(this->_view != nullptr);

		MEMORY_BASIC_INFORMATION info;
		VirtualQuery(this->_view, &info, sizeof(MEMORY_BASIC_INFORMATION));

		this->_size = info.RegionSize;
	}

	const SMReadIterator<Header, T> cbegin() const
	{
		return SMReadIterator<Header, T>{this->_view};
	}

	const SMReadIterator<Header, T> cend() const
	{
		return SMReadIterator<Header, T>{ reinterpret_cast<char*>(this->_view) + this->length*(this->_size / this->length) };
	}
};