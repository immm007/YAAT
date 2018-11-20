#pragma once
#include <Windows.h>
#include <boost/noncopyable.hpp>
#include <assert.h>
#include <type_traits>
#include "Header.h"

template<typename T,typename header>
class SharedMemoryBase:public boost::noncopyable
{
public:
	~SharedMemoryBase()
	{
		UnmapViewOfFile(_view);
		CloseHandle(_handle);
	}
	inline bool eof() const
	{
		return _header == _endPtr;
	}
protected:
	SharedMemoryBase()
	{
		//头结构也必须是POD，然而volatile不被编译器认可
		//static_assert(std::is_pod<header>::value, "only POD header");
		static_assert(std::is_pod<T>::value, "only POD T");
	}
	static constexpr int headerLength = sizeof(header);
	static constexpr int contentLength = sizeof(T);
	static constexpr int length = headerLength + contentLength;

	void* _view;
	void* _handle;
	header* _header;
	void* _endPtr;

	inline T* dataEntry()
	{
		return (T*)(_header + 1);
	}
	inline void moveToNextHeader()
	{
		_header = (header*)((char*)_header + length);
	}
	inline int status() const
	{
		return _header->status;
	}
	inline void markWritten()
	{
		_header->status = 1;
	}
};

template<typename header, typename T, bool write>
class SharedMemory {};

template<typename T,typename header>
class SharedMemory<T,header,true> : public SharedMemoryBase<T,header>
{
public:
	SharedMemory(const char* name, int size)
	{
		assert(size % 4096 == 0);
		this->_handle = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, size, name);
		assert(this->_handle != nullptr);
		this->_view = MapViewOfFile(this->_handle, FILE_MAP_WRITE, 0, 0, 0);
		assert(this->_view != nullptr);

		this->_header = reinterpret_cast<header*>(this->_view);
		this->_endPtr = reinterpret_cast<char*>(this->_view) + size;
	}

	inline SharedMemory& operator<<(const T& data)
	{
		*(this->dataEntry()) = data;
		this->markWritten();
		this->moveToNextHeader();
		return *this;
	}
};

template<typename T, typename header>
class SharedMemory<T, header, false> : public SharedMemoryBase<T, header>
{
public:
	SharedMemory(const char* name)
	{
		this->_handle = OpenFileMappingA(PAGE_READWRITE, false, name);
		assert(this->_handle != nullptr);
		this->_view = MapViewOfFile(this->_handle, FILE_MAP_READ, 0, 0, 0);
		assert(this->_view != nullptr);

		MEMORY_BASIC_INFORMATION info;
		VirtualQuery(this->_view, &info, sizeof(MEMORY_BASIC_INFORMATION));

		this->_header = reinterpret_cast<header*>(this->_view);
		this->_endPtr = reinterpret_cast<char*>(this->_view) + info.RegionSize;
	}

	inline SharedMemory& operator>>(const T*& data)
	{
		if (this->status() == 0)
		{
			data = nullptr;
		}
		else
		{
			data = this->dataEntry();
			this->moveToNextHeader();
		}
		return *this;
	}
};

template<typename T>
using WriteOnlySharedMemory = SharedMemory<T, Header,true>;

template<typename T>
using ReadOnlySharedMemory = SharedMemory<T, Header, false>;