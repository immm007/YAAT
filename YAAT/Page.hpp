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
class FrameIterator
{
public:
	typedef T DataType;
	static constexpr int contentLength = sizeof(T);
	static constexpr int length = Frame::headerLength + contentLength;

	//必不可少默认构造函数
	FrameIterator() {}
	FrameIterator(void * start)
	{
		static_assert(std::is_pod<T>::value, "only POD");
		//不要使用成员初始化列表
		m_header = reinterpret_cast<char*>(start);
	}
	inline Frame::Status status() const
	{
		return reinterpret_cast<Frame::Header*>(m_header)->status;
	}
	inline void markWritten()
	{
		reinterpret_cast<Frame::Header*>(m_header)->status = Frame::Status::Written;
	}
	inline T& operator*()
	{
		return *reinterpret_cast<T*>(m_header + Frame::headerLength);
	}
	//前置++效率更高
	inline FrameIterator& operator++()
	{
		m_header += length;
		return *this;
	}
	inline T* operator&()
	{
		return reinterpret_cast<T*>(m_header + Frame::headerLength);
	}
	inline bool operator!=(const FrameIterator<T>& second) const
	{
		return m_header != second.m_header;
	}
	inline bool operator!=(void* target) const
	{
		return m_header != target;
	}
	inline bool operator==(const FrameIterator<T>& second) const
	{
		return m_header == second.m_header;
	}
	inline bool operator==(void* target) const
	{
		return m_header == target;
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
	void* m_endPtr;
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

		m_endPtr = reinterpret_cast<char*>(m_view) + size;

		m_iter = FrameIterator<T>{ m_view };
	}
	inline bool eof() const
	{
		return m_iter.operator==(m_endPtr);
	}
	//不检查是否满，在外部循环检查
	//这个接口是为请求式的API设计的，若是推送式的API可使用迭代器提供空间
	//不要混合使用这个两种结构，迭代器状态并不同步
	inline Page& operator<<(const typename FrameIterator<T>::DataType & data)
	{
		*m_iter = data;
		m_iter.markWritten();
		++m_iter;
		return *this;
	}
	FrameIterator<T> begin()
	{
		return FrameIterator<T>{m_view};
	}
	FrameIterator<T> end()
	{
		return FrameIterator<T>{m_endPtr};
	}
private:
	FrameIterator<T> m_iter;
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
		m_endPtr = reinterpret_cast<char*>(m_view) + m_size;

		m_iter = FrameIterator<T>{ m_view };
	}
	inline bool eof() const
	{
		return m_iter.operator==(m_endPtr);
	}
	inline bool hasData() const
	{
		return m_iter.status() == Frame::Status::Written;
	}

	//不检查是否有数据，在外部循环检查
	inline Page& operator>>(typename const FrameIterator<T>::DataType*& data)
	{
		data = &m_iter;
		++m_iter;
		return *this;
	}
private:
	FrameIterator<T> m_iter;
};

template<typename T>
using WritePage = Page<T, true>;

template<typename T>
using ReadPage = Page<T, false>;