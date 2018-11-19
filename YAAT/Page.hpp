#pragma once
#include <boost/noncopyable.hpp>
#include <Windows.h>
#include <assert.h>
#include <string>
#include "iterators.hpp"

class PageBase :boost::noncopyable
{
public:
	virtual ~PageBase()
	{
		UnmapViewOfFile(m_view);
		CloseHandle(m_handle);
	}
protected:
	PageBase(const char* name, size_t size) :
		m_name{ name },
		m_size{ size }
	{

	}

	PageBase(const char* name) :
		m_name{ name }
	{

	}

	std::string m_name;
	size_t m_size;
	void* m_view;
	void* m_handle;
};

template<typename T, bool write>
class Page {};

template<typename T>
class Page<T, true> :public PageBase
{
public:
	Page(const char* name, size_t size) :PageBase(name, size)
	{
		//4K对齐
		assert(size % 4096 == 0);
		m_handle = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, size, name);
		assert(m_handle != nullptr);
		m_view = MapViewOfFile(m_handle, FILE_MAP_WRITE, 0, 0, 0);
		assert(m_view != nullptr);
	}
	WriteIterator<T> begin()
	{
		return WriteIterator<T>{m_view};
	}
	WriteIterator<T> end()
	{
		return WriteIterator<T>{(reinterpret_cast<char*>(m_view)+m_size)};
	}
};

template<typename T>
class Page<T, false> :public PageBase
{
public:
	Page(const char* name) :PageBase(name)
	{
		m_handle = OpenFileMappingA(PAGE_READWRITE, false, name);
		assert(m_handle != nullptr);
		m_view = MapViewOfFile(m_handle, FILE_MAP_READ, 0, 0, 0);
		assert(m_view != nullptr);

		MEMORY_BASIC_INFORMATION info;
		VirtualQuery(m_view, &info, sizeof(MEMORY_BASIC_INFORMATION));
		m_size = info.RegionSize;
	}
	ReadIterator<T> begin()
	{
		return ReadIterator<T>{m_view};
	}
	ReadIterator<T> end()
	{
		return ReadIterator<T>{(reinterpret_cast<char*>(m_view) + m_size)};
	}
};

template<typename T>
using WritePage = Page<T, true>;

template<typename T>
using ReadPage = Page<T, false>;