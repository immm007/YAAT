#pragma once
//�����ڴ��ͷ����ָ��ķ�װ
template<typename Header,typename T>
class MemoryIterator
{
public:
	//��ʵ��Ӧ�ù��еģ���������,��Ը��������д�����캯��
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

	//����������ʽ���飬���������и���
	T* operator&()
	{
		return reinterpret_cast<T*>(this->dataEntry());
	}
};