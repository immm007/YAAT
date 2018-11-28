#pragma once
#include <iterator>

template<typename Header, typename T>
class SMForWrite;

template<typename Header, typename T>
class SMForRead;

//共享内存带头数据指针的封装
template<typename Header,typename T>
class SMIterator
{
public:
	inline bool operator==(const SMIterator& other) const
	{
		return _header == other._header;
	}

	inline bool operator!=(const SMIterator& other) const
	{
		return _header != other._header;
	}

	inline SMIterator& operator++()
	{
		this->_header = (Header*)((char*)this->_header + this->length);
		return *this;
	}

	inline SMIterator& operator++(int)
	{
		auto ret = *this;
		this->operator++();
		return ret;
	}
protected:
	static constexpr int length = sizeof(Header) + sizeof(T);

	SMIterator(void* p) :_header{ reinterpret_cast<Header*>(p) }
	{
	}

	SMIterator(char* p) :_header{ reinterpret_cast<Header*>(p) }
	{
	}
	inline T* dataEntry()
	{
		return (T*)(_header + 1);
	}
	inline const T* dataEntry() const
	{
		return (const T*)(_header + 1);
	}
	Header* _header;
};

template<typename Header,typename T>
class SMWriteIterator :public SMIterator<Header, T>
{
public:
	friend class SMForWrite<Header,T>;
	//适用于请求式行情，解析过程中复制
	inline T* operator&()
	{
		return this->dataEntry();
	}
	//适用于推送式行情
	inline void operator=(const T& data)
	{
		this->markWritten();
		*(this->dataEntry()) = data;
	}

	inline void markWritten()
	{
		this->_header->status = Header::Status::Written;
	}
private:
	using SMIterator<Header, T>::SMIterator;
};

template<typename Header, typename T>
class SMReadIterator :public SMIterator<Header, T>,public std::iterator<std::forward_iterator_tag,T>
{
public:
	friend class SMForRead<Header, T>;
	//直接读共享内存，节省拷贝
	inline const T* operator&() const	
	{
		if (this->_header->status == Header::Status::Raw)
		{
			return nullptr;
		}
		return this->dataEntry();
	}
	//兼容STL算法
	inline const T& operator*() const
	{
		return *(this->dataEntry());
	}
private:
	using SMIterator<Header, T>::SMIterator;
};