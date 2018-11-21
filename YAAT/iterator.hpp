#pragma once

template<typename Header, typename T>
class WriteSharedMemory;

template<typename Header, typename T>
class ReadSharedMemory;

//�����ڴ��ͷ����ָ��ķ�װ
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
	//����������ʽ���飬���������и���
	inline void* operator&()
	{
		return this->dataEntry();
	}
	//����������ʽ����
	inline void operator=(const T& data)
	{
		*reinterpret_cast<T*>(this->dataEntry()) = data;
	}
	//��������STL�㷨�ǰ��++������
	inline MemoryIterator& operator++()
	{
		//��Ҳ�������++���棬���ü�����������ͽӿ�
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
	//ֱ�Ӷ������ڴ棬��ʡ����
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