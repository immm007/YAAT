#pragma once
//共享内存带头数据指针的封装
template<typename Header,typename T>
class MemoryIterator
{
public:
	//其实不应该共有的，但是我懒,不愿意子类再写个构造函数
	MemoryIterator(void* p = nullptr) :_header{ reinterpret_cast<Header*>(p) }
	{
	}
	inline MemoryIterator& operator++()
	{
		_header =  (Header*)((char*)_header + length);
		return *this;
	}
protected:
	static constexpr int headerLength = sizeof(Header);
	static constexpr int contentLength = sizeof(T);
	static constexpr int length = headerLength + contentLength;

	inline typename Header::MemoryStatus memoryStatus() const
	{
		return _header->status;
	}
	inline void markWritten()
	{
		_header->status == Header::Status::Written;
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
	using MemoryIterator<Header,T>::MemoryIterator;

	//适用于请求式行情，解析过程中复制
	T* operator&()
	{
		return reinterpret_cast<T*>(this->dataEntry());
	}
};