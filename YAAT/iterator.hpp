#pragma once

template<typename Header, typename T>
class WriteSharedMemory;

template<typename Header, typename T>
class ReadSharedMemory;

//共享内存带头数据指针的封装
template<typename Header,typename T>
class MemoryIterator
{
protected:
	static constexpr int headerLength = sizeof(Header);
	static constexpr int contentLength = sizeof(T);
	static constexpr int length = headerLength + contentLength;

	MemoryIterator(void* p) :_header{ reinterpret_cast<Header*>(p) }
	{
	}
	inline void* dataEntry()
	{
		return reinterpret_cast<void*>(_header + 1);
	}
	Header* _header;
};

template<typename Header,typename T>
class WriteIterator :public MemoryIterator<Header, T>
{
public:
	friend class WriteSharedMemory<Header,T>;
	using MemoryIterator<Header,T>::MemoryIterator;
	//适用于请求式行情，解析过程中复制
	inline void* operator&()
	{
		return this->dataEntry();
	}
	//适用于推送式行情
	inline void operator=(const T& data)
	{
		*reinterpret_cast<T*>(this->dataEntry()) = data;
	}
	//不会用在STL算法里，前置++更好用
	inline MemoryIterator& operator++()
	{
		//我也不想放在++里面，但得兼容请求和推送接口
		this->markWritten();
		_header = (Header*)((char*)_header + length);
		return *this;
	}
private:
	inline void markWritten()
	{
		this->_header->status = Header::MemoryStatus::Written;
	}
};

template<typename Header, typename T>
class ReadIterator :public MemoryIterator<Header, T>
{
public:
	friend class ReadSharedMemory<Header, T>;
	using MemoryIterator<Header, T>::MemoryIterator;

	inline ReadIterator& operator++()
	{
		_header = (Header*)((char*)_header + length);
		return *this;
	}
	//直接读共享内存，节省拷贝
	inline const T* operator*()
	{
		if (this->memoryStatus() == Header::MemoryStatus::Raw)
		{
			return nullptr;
		}
		return reinterpret_cast<const T*>(this->dataEntry());
	}
private:
	inline typename Header::MemoryStatus memoryStatus() const
	{
		return _header->status;
	}
};