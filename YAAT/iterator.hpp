#pragma once

template<typename Header, typename T>
class WriteSharedMemory;

template<typename Header, typename T>
class ReadSharedMemory;

//�����ڴ��ͷ����ָ��ķ�װ
template<typename Header,typename T>
class MemoryIterator
{
public:
	inline bool operator==(const MemoryIterator& other)
	{
		return _header == other._header;
	}
	inline bool operator!=(const MemoryIterator& other)
	{
		return _header != other._header;
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
		*reinterpret_cast<T*>(this->dataEntry()) = data;
	}
	//��������STL�㷨�ǰ��++������
	inline WriteIterator& operator++()
	{
		//��Ҳ�������++���棬���ü�����������ͽӿ�
		this->markWritten();
		this->_header = (Header*)((char*)this->_header + this->length);
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
		this->_header = (Header*)((char*)this->_header + this->length);
		return *this;
	}
	//ֱ�Ӷ������ڴ棬��ʡ����
	inline const T* operator&() const	{
		if (this->memoryStatus() == Header::MemoryStatus::Raw)
		{
			return nullptr;
		}
		return reinterpret_cast<const T*>(this->dataEntry());
	}
private:
	inline typename Header::MemoryStatus memoryStatus() const
	{
		return this->_header->status;
	}
};