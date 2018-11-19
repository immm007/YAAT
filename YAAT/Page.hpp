#pragma once
#include <type_traits>
#include <boost/noncopyable.hpp>
#include <Windows.h>
#include <assert.h>


struct Frame
{
	enum Status
	{
		Raw, Written
	};
	struct Header
	{
		volatile Status status;
	};
	static constexpr int headerLength = sizeof(Header);
};

//为存放POD设计的迭代器
template<typename T>
class Iterator
{
public:
	typedef T DataType;
	static constexpr int contentLength = sizeof(T);
	static constexpr int length = Frame::headerLength + contentLength;

	Iterator() {}
	Iterator(void * start)
	{
		static_assert(std::is_pod<T>::value, "only POD");
		m_header = reinterpret_cast<char*>(start);
	}
	inline Frame::Status status() const
	{
		return reinterpret_cast<Frame::Header*>(m_header)->status;
	}
	inline void markWritten() const
	{
		reinterpret_cast<Frame::Header*>(m_header)->status = Frame::Status::Written;
	}
	inline T& operator*() const
	{
		return *reinterpret_cast<T*>(m_header + Frame::headerLength);
	}
	inline Iterator& operator++()
	{
		m_header += length;
		return *this;
	}
	inline const T* operator&() const
	{
		return reinterpret_cast<const T*>(m_header + Frame::headerLength);
	}
private:
	char* m_header;
};

class PageBase :boost::noncopyable
{
public:
	virtual ~PageBase()
	{
		UnmapViewOfFile(m_view);
		CloseHandle(m_handle);
	}
	bool eof() const
	{
		return m_pos == m_endPos;
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
	int m_pos{ 0 };
	int m_endPos;
};

template<typename T, bool write>
class Page {};

template<typename T>
class Page<T, true> :public PageBase
{
public:
	Page(const char* name, size_t size) :PageBase(name, size)
	{
		m_handle = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, size, name);
		assert(m_handle != nullptr);
		m_view = MapViewOfFile(m_handle, FILE_MAP_WRITE, 0, 0, 0);
		assert(m_view != nullptr);
		m_endPos = m_size / Iterator<T>::length;
		m_iter = Iterator<T>{ m_view };
	}

	//不检查是否满，在外部循环检查
	inline Page& operator<<(const typename Iterator<T>::DataType & data)
	{
		*m_iter = data;
		m_iter.markWritten();
		++m_iter;
		++m_pos;
		return *this;
	}
private:
	Iterator<T> m_iter;
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
		m_endPos = m_size / Iterator<T>::length;
		m_iter = Iterator<T>{ m_view };
	}

	inline bool hasData() const
	{
		return m_iter.status() == Frame::Status::Written;
	}

	//不检查是否有数据，在外部循环检查
	inline Page& operator>>(typename const Iterator<T>::DataType*& data)
	{
		data = &m_iter;
		++m_iter;
		++m_pos;
		return *this;
	}
private:
	Iterator<T> m_iter;
};

template<typename T>
using WritePage = Page<T, true>;

template<typename T>
using ReadPage = Page<T, false>;