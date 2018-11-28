#pragma once
#include <iterator>

template<typename Header, typename T>
class SMForWrite;

template<typename Header, typename T>
class SMForRead;

//�����ڴ��ͷ����ָ��ķ�װ
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
	//����������ʽ���飬���������и���
	inline T* operator&()
	{
		return this->dataEntry();
	}
	//����������ʽ����
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
	//ֱ�Ӷ������ڴ棬��ʡ����
	inline const T* operator&() const	
	{
		if (this->_header->status == Header::Status::Raw)
		{
			return nullptr;
		}
		return this->dataEntry();
	}
	//����STL�㷨
	inline const T& operator*() const
	{
		return *(this->dataEntry());
	}
private:
	using SMIterator<Header, T>::SMIterator;
};