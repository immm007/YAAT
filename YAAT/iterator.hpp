#pragma once
#include <iterator>

template<typename Header, typename T>
class WriteSharedMemory;

template<typename Header, typename T>
class ReadSharedMemory;

//�����ڴ��ͷ����ָ��ķ�װ
template<typename Header,typename T>
class MemoryIterator
{
public:
	inline typename Header::Status status() const
	{
		return this->_header->status;
	}
	inline bool operator==(const MemoryIterator& other) const
	{
		return _header == other._header;
	}
	inline bool operator!=(const MemoryIterator& other) const
	{
		return _header != other._header;
	}
	inline MemoryIterator& operator++()
	{
		this->_header = (Header*)((char*)this->_header + this->length);
		return *this;
	}
	inline MemoryIterator& operator++(int)
	{
		auto ret = *this;
		this->operator++();
		return ret;
	}
protected:
	static constexpr int headerLength = sizeof(Header);
	static constexpr int contentLength = sizeof(T);
	static constexpr int length = headerLength + contentLength;

	MemoryIterator(void* p) :_header{ reinterpret_cast<Header*>(p) }
	{
	}
	MemoryIterator(char* p) :_header{ reinterpret_cast<Header*>(p) }
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
	//����������ʽ���飬���������и���
	inline T* operator&()
	{
		return reinterpret_cast<T*>(this->dataEntry());
	}
	//����������ʽ����
	inline void operator=(const T& data)
	{
		this->markWritten();
		*reinterpret_cast<T*>(this->dataEntry()) = data;
	}
	inline void markWritten()
	{
		this->_header->status = Header::Status::Written;
	}
};

template<typename Header, typename T>
class OnlineReadIterator :public MemoryIterator<Header, T>
{
public:
	friend class ReadSharedMemory<Header, T>;
	using MemoryIterator<Header, T>::MemoryIterator;
	//ֱ�Ӷ������ڴ棬��ʡ����
	inline const T* operator&() const	
	{
		if (this->status() == Header::Status::Raw)
		{
			return nullptr;
		}
		return reinterpret_cast<const T*>(this->dataEntry());
	}
};

//���̺����ʹ��
template<typename Header, typename T>
class OfflineReadIterator :public MemoryIterator<Header, T>,public std::iterator<std::random_access_iterator_tag,T>
{
public:
	friend class ReadSharedMemory<Header, T>;
	using MemoryIterator<Header, T>::MemoryIterator;

	inline const T& operator*() const 
	{
		return *reinterpret_cast<const T*>(this->dataEntry());
	}
};